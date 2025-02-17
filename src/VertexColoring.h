#ifndef VERTEX_COLORING_H
#define VERTEX_COLORING_H

#include <vector>
#include <algorithm>
#include <limits>
#include <chrono>
#include "Graph.h"
#include <VectorSet.h>
#include <omp.h>

template<class VectorT>
class VertexColoring {
public:
    Graph<VectorT>& graph;
    std::vector<int> bestColoring;

    VertexColoring(Graph<VectorT>& g);
    int findChromaticNumber();
    bool isProperlyColored(const std::vector<int>& coloring);

private:
    int globalLowerBound;
    int globalUpperBound;
    int debugOut = 1;

    int greedyColoring(std::vector<int>);
    std::vector<int> findMaxClique();
    void branchAndBound(std::vector<int>& currentColoring, int vertex);
    bool isSafe(const std::vector<int>& coloring, int vertex, int color);
    int calculateUpperBound(const std::vector<int>& partialColoring);
};

template <class VectorT>
VertexColoring<VectorT>::VertexColoring(Graph<VectorT>& g) : graph(g) {}

template <class VectorT>
int VertexColoring<VectorT>::findChromaticNumber() {
    int numVertices = graph.getNumVertices();
    bestColoring.assign(numVertices, -1);
    

    // std::vector<int> maxClique = findMaxClique();
    graph.sortVerticesByDegree();
    graph.removeVerticesWithLowDegree(1); // Can this be max-clique - 2 or something I wonder? rn just removes isolated vertices
    VectorT maxClique = graph.findMaxCliqueApprox();
    std::cout << "approx Max clique set: " << maxClique << std::endl;
    std::cout << "approx Max clique size : " << maxClique.size() << std::endl;

    std::vector<int> currentColoring(numVertices, -1);
    for (int i = 0; i < maxClique.size(); ++i) {
        currentColoring[maxClique[i]] = i;
    }

    globalLowerBound = maxClique.size();
    globalUpperBound = greedyColoring(currentColoring);
    
    if (globalUpperBound == globalLowerBound){
        std::cout << "Upper and lower bound are the same: " << globalLowerBound << "\n";
        return globalLowerBound;
    }
    if (debugOut){
        std::cout << "Lower bound: "<< globalLowerBound << "\n";
        std::cout << "Upper bound: "<< globalUpperBound << "\n";
    }

    auto start = std::chrono::high_resolution_clock::now();
    branchAndBound(currentColoring, 0);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "BB duration: " << duration.count() << "ms\n";

    return globalUpperBound;
}


template <class VectorT>
void VertexColoring<VectorT>::branchAndBound(std::vector<int>& currentColoring, int vertex) {
    if (vertex == graph.getNumVertices()) {
        int colorsUsed = *std::max_element(currentColoring.begin(), currentColoring.end()) + 1;        
        if (colorsUsed < globalUpperBound) {
            std::cout << "Best coloring found using " << colorsUsed << "\n";
            {
            globalUpperBound = colorsUsed;
            bestColoring = currentColoring;
            }
            // std::cout << "Coloring: ";
            // for (int i = 0; i < bestColoring.size(); i++){
            //     std::cout << bestColoring[i] << ", ";
            // }
            // std::cout << "\n";
        }
        return;
    }

    int localLowerBound = std::max(globalLowerBound, *std::max_element(currentColoring.begin(), currentColoring.end())+1);
    if (localLowerBound >= globalUpperBound) {
        return;
    }

    if (currentColoring[vertex] != -1) {
        branchAndBound(currentColoring, vertex + 1);
        return;
    }

    for (int color = 0; color < globalUpperBound; ++color) {
        if (isSafe(currentColoring, vertex, color)) {
            currentColoring[vertex] = color;

            if (vertex < 4) {
                #pragma omp task firstprivate(currentColoring)
                branchAndBound(currentColoring, vertex + 1);
            } else {
                branchAndBound(currentColoring, vertex + 1);
            }

            currentColoring[vertex] = -1;  // Backtrack
        }
    }
}

template <class VectorT>
bool VertexColoring<VectorT>::isSafe(const std::vector<int>& coloring, int vertex, int color) {
    for (int i = 0; i < graph.getNumVertices(); ++i) {
        if (graph.areNeighbours(vertex, i) && coloring[i] == color) {
            return false;
        }
    }
    return true;
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
            // std::cout << v << " gets color " << color << "\n";
            colors[v] = color;
        }
        maxUsedColor = std::max(maxUsedColor, colors[v]);
        std::fill(availableColors.begin(), availableColors.end(), true);
    }
    isProperlyColored(colors);
    bestColoring = colors;
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "Upper bound duration: " << duration.count() << "ms\n";
    return maxUsedColor+1;
}



template <class VectorT>
bool VertexColoring<VectorT>::isProperlyColored(const std::vector<int>& coloring) {
    int numVertices = graph.getNumVertices();
    
    // std::cout << "Coloring: ";
    // for (int i = 0; i < coloring.size(); i++){
    //     std::cout << coloring[i] << ", ";
    // }
    // std::cout << "\n";
        
    if (coloring.size() != numVertices) {
        std::cout << "ERR: Coloring size is not the same as the number of vertices of the graph \n";
        return false;
    }
    
    if (std::find(coloring.begin()+1, coloring.end(), -1) != coloring.end()) {
        std::cout << "ERR: Found uncolored vertex \n";
        return false;
    }
    
    // Check if adjacent vertices have different colors
    for (int v = 0; v < numVertices; ++v) {
        for (int u = v + 1; u < numVertices; ++u) {
            if (graph.areNeighbours(v, u) && coloring[v] == coloring[u]) {
                std::cout << "ERR: Vertices " << v << " and " << u << " are colored the same\n";
                return false;
            }
        }
    }
    
    return true;
}




#endif // VERTEX_COLORING_H
