#ifndef VERTEX_COLORING_H
#define VERTEX_COLORING_H

#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include "Graph.h"
#include <VectorSet.h>
#include <omp.h>
#include <unordered_set>

template<class VectorT>
class VertexColoring {
public:
    Graph<VectorT>& graph;
    std::vector<int> bestColoring;
    VectorT maxClique;
    std::vector<int> diffNeighbors;

    VertexColoring(Graph<VectorT>& g);
    int findChromaticNumber();
    bool isProperlyColored(const std::vector<int>& coloring);

    struct Node {
        Graph<VectorT> graph;
        std::vector<std::vector<int>> mergedSets;
        VectorT activeVertices;
        int lowerBound;
        int upperBound;
        
        // Constructor
        explicit Node(const Graph<VectorT>& g) : 
            graph(g), 
            mergedSets(g.getNumVertices()),
            activeVertices() 
        {
        // Initialize mergedSets
        for (int i = 0; i < g.getNumVertices(); i++) {
            mergedSets[i] = {i};
        }
        
        // Initialize activeVertices
        activeVertices.clear();  // Ensure it's empty
        activeVertices.reserve(g.getNumVertices());
        for (int i = 0; i < g.getNumVertices(); i++) {
            activeVertices.add(i);
        }
        lowerBound = 0;
        upperBound = g.getNumVertices();
    }

        // Copy constructor
        Node(const Node& other) : 
            graph(other.graph),
            isActive(other.isActive),
            numActiveVertices(other.numActiveVertices),
            lowerBound(other.lowerBound),
            upperBound(other.upperBound)
        {}

        // Assignment operator
        Node& operator=(const Node& other) {
            if(this != &other) {
                graph = other.graph;
                isActive = other.isActive;
                numActiveVertices = other.numActiveVertices;
                lowerBound = other.lowerBound;
                upperBound = other.upperBound;
            }
            return *this;
        }

        // Helper methods
        void deactivateVertex(int v) {
            if(v >= 0 && v < isActive.size() && isActive[v]) {
                isActive[v] = false;
                numActiveVertices--;
            }
        }

        std::vector<int> getActiveVertices() const {
            std::vector<int> active;
            active.reserve(numActiveVertices);
            for(int i = 0; i < isActive.size(); i++) {
                if(isActive[i]) {
                    active.push_back(i);
                }
            }
            return active;
        }
    };

    std::pair<int, int> chooseBranchingVertices(const Node& node) const {
        int v1 = -1, v2 = -1;
        int maxDegree = -1;
        
        auto activeVerts = node.getActiveVertices();
        for(size_t i = 0; i < activeVerts.size(); i++) {
            int vi = activeVerts[i];
            for(size_t j = i + 1; j < activeVerts.size(); j++) {
                int vj = activeVerts[j];
                if(!node.graph.areNeighbours(vi, vj)) {
                    int combinedDegree = node.graph.getDegree(vi) + node.graph.getDegree(vj);
                    if(combinedDegree > maxDegree) {
                        maxDegree = combinedDegree;
                        v1 = vi;
                        v2 = vj;
                    }
                }
            }
        }
        return {v1, v2};
    }

    Node mergeVertices(const Node& parent, int v1, int v2) const {
        try {
            if(v1 < 0 || v2 < 0 || 
               v1 >= parent.graph.getNumVertices() || 
               v2 >= parent.graph.getNumVertices()) {
                return parent;
            }

            Node newNode(parent);
            
            // Update edges
            auto activeVerts = newNode.getActiveVertices();
            for(int vi : activeVerts) {
                if(vi != v1 && vi != v2) {
                    bool shouldConnect = newNode.graph.areNeighbours(vi, v1) || 
                                      newNode.graph.areNeighbours(vi, v2);
                    newNode.graph.setNeighbours(vi, v1, shouldConnect);
                    newNode.graph.setNeighbours(v1, vi, shouldConnect);
                }
            }
            
            // Deactivate v2
            newNode.deactivateVertex(v2);
            
            return newNode;
        } catch(const std::exception& e) {
            std::cout << "Error in mergeVertices: " << e.what() << std::endl;
            return parent;
        }
    }

    Node addEdge(const Node& parent, int v1, int v2) const {
        try {
            if(v1 < 0 || v2 < 0 || 
               v1 >= parent.graph.getNumVertices() || 
               v2 >= parent.graph.getNumVertices()) {
                return parent;
            }

            Node newNode(parent);
            newNode.graph.setNeighbours(v1, v2, true);
            return newNode;
        } catch(const std::exception& e) {
            std::cout << "Error in addEdge: " << e.what() << std::endl;
            return parent;
        }
    }

    void branchAndBoundSequential(Node& node) {
        try {
            if(debugOut) {
                std::cout << "\nCurrent node stats:" << std::endl;
                std::cout << "Active vertices: " << node.numActiveVertices << std::endl;
                std::cout << "Current lower bound: " << globalLowerBound << std::endl;
                std::cout << "Current upper bound: " << globalUpperBound << std::endl;
            }

            // Base cases
            if(node.numActiveVertices <= 1 || globalLowerBound == globalUpperBound) {
                return;
            }

            // Calculate bounds for current node
            VectorT clique = node.graph.findMaxCliqueApprox();
            node.lowerBound = clique.size();
            
            std::vector<int> initialColoring(node.graph.getNumVertices(), -1);
            for(size_t i = 0; i < clique.size(); i++) {
                initialColoring[clique[i]] = i;
            }
            node.upperBound = greedyColoring(initialColoring);
            
            // Update global bounds
            globalLowerBound = std::max(globalLowerBound, node.lowerBound);
            globalUpperBound = std::min(globalUpperBound, node.upperBound);
            
            if(debugOut) {
                std::cout << "Node bounds - Lower: " << node.lowerBound 
                         << ", Upper: " << node.upperBound << std::endl;
            }

            // Choose vertices for branching
            auto vertices = chooseBranchingVertices(node);
            if(vertices.first == -1 || vertices.second == -1) {
                return;
            }

            if(debugOut) {
                std::cout << "Branching on vertices " << vertices.first << " and " 
                         << vertices.second << std::endl;
            }

            // Branch 1: Merge vertices
            Node mergedNode = mergeVertices(node, vertices.first, vertices.second);
            if(mergedNode.numActiveVertices < node.numActiveVertices) {
                branchAndBoundSequential(mergedNode);
            }

            // Branch 2: Add edge (only if we haven't found optimal solution)
            if(globalLowerBound < globalUpperBound) {
                Node edgeNode = addEdge(node, vertices.first, vertices.second);
                branchAndBoundSequential(edgeNode);
            }
        } catch(const std::exception& e) {
            std::cout << "Error in branchAndBoundSequential: " << e.what() << std::endl;
        }
    }

private:
    int globalLowerBound;
    int globalUpperBound;
    int debugOut = 1;

    int greedyColoring(std::vector<int>);
    std::vector<int> findMaxClique();
    void branchAndBound(std::vector<int> &currentColoring, int maxColor);
    int chooseVertex(std::vector<int> &currentColoring);
    bool isSafe(const std::vector<int> &coloring, int vertex, int color);
    int countDistinctNeighborColors(int vertex, const std::vector<int> &currentColoring);
    int calculateUpperBound(const std::vector<int> &partialColoring);
};

template <class VectorT>
VertexColoring<VectorT>::VertexColoring(Graph<VectorT>& g) : graph(g) {}

template <class VectorT>
int VertexColoring<VectorT>::findChromaticNumber() {
    // Initialize root node
    Node rootNode(graph);
    
    // Initialize bounds
    maxClique = graph.findMaxCliqueApprox();
    globalLowerBound = maxClique.size();
    std::vector<int> initialColoring(graph.getNumVertices(), -1);
    globalUpperBound = greedyColoring(initialColoring);
    
    if(globalLowerBound == globalUpperBound) {
        return globalLowerBound;
    }
    
    // Start branch and bound
    branchAndBoundSequential(rootNode);
    
    return globalUpperBound;
}

template <class VectorT>
bool VertexColoring<VectorT>::isSafe(const std::vector<int>& coloring, int vertex, int color) {
    for (int i = 0; i < maxClique.size(); ++i) { //loop over assigned vertices
        if (coloring[maxClique[i]] == color && graph.areNeighbours(vertex, maxClique[i])) {
            return false;
        }
    }
    for (int i = 0; i < vertex; ++i) { //loop over assigned vertices
        if (coloring[i] == color && graph.areNeighbours(vertex, i)) {
            return false;
        }
    }
    return true;
}

template <class VectorT>
int VertexColoring<VectorT>::countDistinctNeighborColors(int vertex, const std::vector<int>& currentColoring) {
    std::unordered_set<int> uniqueColors;
    
    for (int i = 0; i < currentColoring.size(); i++) {
        int color = currentColoring[i];
        if (color != -1) {
            uniqueColors.insert(color);
        }
    }
    
    return uniqueColors.size();
}

template <class VectorT>
int VertexColoring<VectorT>::greedyColoring(std::vector<int> inputColors) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<int> colors = inputColors;
    int numVertices = graph.getNumVertices();
    std::vector<bool> availableColors(numVertices, true);
    int maxUsedColor = 0;

    for (int v = 0; v < numVertices; ++v) {
        if (colors[v] == -1){
            for (int i = 0; i < numVertices; ++i) {
                if (graph.areNeighbours(v, i) && colors[i] != -1) {
                    availableColors[colors[i]] = false;
                }
            }

            int color;
            for (color = 0; color < numVertices; ++color) {
                if (availableColors[color]) break;
            }
            colors[v] = color;
        }
        maxUsedColor = std::max(maxUsedColor, colors[v]);
        std::fill(availableColors.begin(), availableColors.end(), true);
    }
    isProperlyColored(colors);
    bestColoring = colors;
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    //std::cout << "Upper bound duration: " << duration.count() << "ms\n";
    return maxUsedColor+1;
}

template <class VectorT>
bool VertexColoring<VectorT>::isProperlyColored(const std::vector<int>& coloring) {
    
    if (std::find(coloring.begin()+1, coloring.end(), -1) != coloring.end()) {
        std::cout << "ERR: Found uncolored vertex \n";
        return false;
    }
    
    // Check if adjacent vertices have different colors
    for (int v = 0; v < coloring.size(); ++v) {
        for (int u = v + 1; u < coloring.size(); ++u) {
            if (graph.areNeighbours(v, u) && coloring[v] == coloring[u]) {
                std::cout << "ERR: Vertices " << v << " and " << u << " are colored the same\n";
                return false;
            }
        }
    }
    
    return true;
}

#endif // VERTEX_COLORING_H
