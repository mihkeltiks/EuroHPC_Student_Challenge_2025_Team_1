#include <iostream>
#include <cassert>
#include <set>
#include <sstream>
#include "../src/Graph.h"
#include "../src/Dimacs.h"
#include "../src/VectorSet.h"

// Helper function to print a clique
template<typename VectorT>
void printClique(const VectorT& clique) {
    std::cout << "Clique vertices: ";
    for (size_t i = 0; i < clique.size(); ++i) {
        std::cout << clique[i] << " ";
    }
    std::cout << "\nClique size: " << clique.size() << std::endl;
}

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

        graph.wasRemapedTo0based = !loader.verticesAreMappedFrom1based();
    } else {
        error << "\n   Dimacs loader error: " << loader.getError();
    }
    return graph;
};

typedef int VertexId;
typedef VectorSet<VertexId> NodeSet;

void testMaxCliqueApprox(const char* instanceFile) {
    std::cout << "Testing maximum clique approximation..." << std::endl;
    
    // Load test graph from file
    Graph<NodeSet> g = loadGraph<NodeSet>(instanceFile);
    std::cout << "number of vertices: " << g.getNumVertices() << std::endl;
    
    std::cout << "Graph loaded successfully!" << std::endl;
    
    // Find maximum clique
    NodeSet maxClique = g.findMaxCliqueApprox();
    
    // Print results
    std::cout << "\nFound maximum clique:" << std::endl;
    printClique(maxClique);
    
    // Verify that the found vertices form a clique
    for (auto i : maxClique) {
        for (auto j : maxClique) {
            if (i != j && !g.areNeighbours(i, j)) {
                std::cerr << "Error: Vertices " << i << " and " << j 
                        << " are in the clique but are not connected!" << std::endl;
                throw std::runtime_error("Vertices in result do not form a clique!");
            }
        }
    }
    
    std::cout << std::endl <<"Maximum clique approximation test passed!" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <instance_file>" << std::endl;
        std::cerr << "Example: " << argv[0] << " maxclique_instances/brock200_4.clq" << std::endl;
        return 1;
    }

    try {
        testMaxCliqueApprox(argv[1]);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        return 1;
    }
}