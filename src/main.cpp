#include <iostream>
#include <bitset>
#include <cassert>
#include <algorithm>
#include "CommandLineParameters.h"
#include "Dimacs.h"
#include "Graph.h"
#include "VectorSet.h"
#include <VertexColoring.h>

/**
 * @brief Calculate graph density from the number of vertices @link #v and number of edges @link e
 * @param v number of vertices
 * @param e number of edges
 * @return density of the graph (ratio of connectedness, 0 - not connected at all, 1 - fully connected)
 */
float getDensity(int v, int e) {
    return e*2.0/(v*(v-1.0));
}

/**
 * @brief Load a graph from the file provided by a file name 
 * @param fname a file name string
 * @return a Graph on success; throws on error
 */
template<typename NodeSet>
Graph<NodeSet> loadGraph(const char* fname) {
    std::cout << "Loading graph " << fname;
    std::cout << std::endl;
    Graph<NodeSet> graph;
    std::stringstream error;
    
    DimacsLoader loader{};
    if (loader.load(fname) && (loader.getNumVertices() > 0)) {
        graph.init(loader.getAdjacencyMatrix(), loader.getDegrees());
        std::cout << "  this is a Dimacs file with a graph of " << loader.getNumVertices() << " vertices, " << loader.getNumEdges() << " edges, " 
            << getDensity(loader.getNumVertices(), loader.getNumEdges()) << " density" << std::endl;
        graph.wasRemapedTo0based = loader.verticesAreMappedFrom1based();
    } else {
        error << "\n   Dimacs loader error: " << loader.getError();
    }
    return graph;

};

        typedef int VertexId;
        typedef VectorSet<VertexId> NodeSet;


int main(int argc, char** argv) {
    try {
        // load commandline arguments (also generate graphs or load them from file, as required)
        using namespace CommandlineParameters;
        ParameterSet parameterSet;
        bool debugMC;
        bool printHelp = false;
        long timeout = 1000;
        std::string inputString;    // this is a dummy variable, reused by parsing function and will not hold any useful value after parsing commandline
        std::vector<std::string> inputGraphParameters;
        std::vector<int> bindProcessors;
        int numThreads = 0, numJobs = 1;
        bool invertInputGraph = false;
        Graph<NodeSet> inputGraph;

        std::cout << "warning: this algorithm does not use bitboards\n";
        
        parameterSet.addDefinition("-timeout", "Timeout in seconds; bound the maximum execution time of the algorithm.")
            .setNumberOfValues(1)
            .bindToVariable(timeout)
            .setDefaultValue(1000);
        parameterSet.addDefinition("-input", "Provides a graph input to the algorithm, in a form of a file (path is provided); supported file formatas are dimacs text, SIP, Arg, and .dat.")
            .setNumberOfValues(1)
            .bindToVariable(inputString)
            .addOnChangeHandler([&inputGraphParameters](const std::string& val)->std::string {inputGraphParameters.push_back(val); return "";});
        parameterSet.addDefinition("-threads", "Defines the number of threads to execute on.")
            .setDefaultValue("0")     // 0 = sequential algorithm
            .setNumberOfValues(1)
            .bindToVariable(numThreads);
        parameterSet.addDefinition("-jobs", "Defines the number of threads to execute on.")
            .setDefaultValue("1")     // 0 = sequential algorithm
            .setNumberOfValues(1)
            .bindToVariable(numJobs);
        parameterSet.addDefinition("-bind", "Defines how the threads should be bound to cores: takes a list of core indices, separated by commas.")
            .setNumberOfValues(1)
            .bindToVariable(inputString)
            .addOnChangeHandler([&bindProcessors](const std::string& val)->std::string {
                std::string singleIndex;
                std::stringstream valStream(val);
                while(getline(valStream, singleIndex, ',')) 
                    bindProcessors.push_back(std::stoi(singleIndex));
                return "";}
            );
        parameterSet.addDefinition("-invertInput", "Inverts the graph before starting the search; effectively this changes max cliuqe to max independent set algorithm.")
            .setNumberOfValues(0)
            .bindToVariable(invertInputGraph);
        parameterSet.addDefinition("-help", "Show the help screen.")
            .setDefaultValue("false")
            .bindToVariable(printHelp);
        parameterSet.addDefinition("--help", "Show the help screen.")
            .makeHelpInvisible()
            .setDefaultValue("false")
            .bindToVariable(printHelp);
        
        // TODO add all missing definitions
                
        // parse the parameters
        try {
            auto r = parameterSet.parse(argv+1, argc-1); 
            
            // check if parameters were added at all
            if (printHelp || (argc == 0)) {
                std::cerr << "This program should be provided the following arguments:\n";
                std::cerr << parameterSet.generateHelpScreen();
                return true;
            }
            
            if (r.errors) {
                std::cout << "Error while parsing arguments: " << parameterSet.getLog() << std::endl; 
                return false;
            }
        } catch (...) {
            std::cout << "Exception while parsing parameters.\n";
            throw;
        }
        
        if (inputGraphParameters.size() == 1){ 
            try {        
                const char* testCliqueFile = (inputGraphParameters[0].c_str() == nullptr ? "12345.clq" : inputGraphParameters[0].c_str());
                
                inputGraph = loadGraph<NodeSet>(testCliqueFile);
                inputGraph.debugOut();
                bool graphLoaded = inputGraph.getNumEdges() > 0;
                if (!graphLoaded) {
                    throw std::runtime_error("Unable to load graph");
                }
                if (invertInputGraph) {
                    inputGraph.invertEdges();
                    std::cout << "Inverted graph: " << inputGraph.getNumVertices() << " vertices " << inputGraph.getNumEdges() << " edges " 
                    << getDensity(inputGraph.getNumVertices(), inputGraph.getNumEdges()) << " density " << std::endl;
                }

                inputGraph.debugOut();

                VertexColoring<NodeSet> coloring(inputGraph);
                int numColors = coloring.colorGraph();
                std::cout << "Minimum number of colors: " << numColors << std::endl;
                std::cout << std::endl;

                // maximum clique approximation
                NodeSet maxClique = inputGraph.findMaxCliqueApprox();
                std::cout << "approx Max clique set: " << maxClique << std::endl;
                std::cout << "approx Max clique size : " << maxClique.size() << std::endl;

            } catch (std::exception& e) {
                std::cout << "Terminated due to exception: ";
                std::cerr << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Terminated due to ";
                std::cerr << "unknown";
                std::cerr << " exception: " << std::endl;
            }
        }
        else {
            if (inputGraphParameters.size() > 1)
                std::cout << "Error: number of input graphs must be 1." << std::endl;
            else
                std::cout << "Error: no input graphs were provided." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Terminated due to exception: " << e.what() << std::endl;
    } catch (const std::string& e) {
        std::cout << "Terminated due to exception: " << e << std::endl;
    } catch (...) {
        std::cout << "Terminated due to unknown exception: " << std::endl;
    }
    return 0;
}
