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
    // std::vector<int> maxClique = findMaxClique();
    graph.sortVerticesByDegree();
    maxClique = graph.findMaxCliqueApprox();
    std::cout << "approx Max clique set: " << maxClique << std::endl;
    std::cout << "approx Max clique size : " << maxClique.size() << std::endl;


    graph.removeVerticesWithLowDegree(maxClique.size() - 1,  maxClique);
    int numVertices = graph.getNumVertices();
    bestColoring.assign(numVertices, -1);
    std::vector<int> currentColoring(numVertices, -1);
    for (int i = 0; i < maxClique.size(); ++i) {
        currentColoring[maxClique[i]] = i;
    }

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

    branchAndBound(currentColoring, *std::max_element(currentColoring.begin(), currentColoring.end()) + 1);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << "BB duration: " << duration.count() << "ms\n";

    return globalUpperBound;
}


    template <class VectorT>
    void VertexColoring<VectorT>::branchAndBound(std::vector<int>& currentColoring, int maxColor) {
        // std::cout << " Here \n";
        int localLowerBound = std::max(globalLowerBound, maxColor);
        if (localLowerBound >= globalUpperBound) {
            return;
        }
        // std::cout << " Here2 \n";
        int vertex = chooseVertex(currentColoring);
        if (vertex == -1) {
            int colorsUsed = maxColor;
            std::cout << "New coloring found using " << colorsUsed << "\n";
            globalUpperBound = colorsUsed;
            bestColoring = currentColoring;
            return;
        }
        // std::cout << " Here3 \n";
        #pragma omp parallel
        {
            #pragma omp single nowait
            {
                for (int color = 0; color < globalUpperBound - 1; ++color) {
                    // std::cout << " Here 4  " << vertex <<  " " << color << "\n";
                    if (isSafe(currentColoring, vertex, color)) {
                        // std::cout << " Here 5\n";
                        #pragma omp task firstprivate(currentColoring, color, maxColor)
                        {
                            currentColoring[vertex] = color;
                            branchAndBound(currentColoring, std::max(maxColor, color+1));
                            currentColoring[vertex] = -1;
                        }
                    }
                }
            }
        }
    }

    template <class VectorT>
    int VertexColoring<VectorT>::chooseVertex(std::vector<int>& currentColoring){
        int bestindex = -1;
        int maxNeighbors = 0;
        int maxDegree = 0;

        for (int i = 0; i < currentColoring.size(); i++){
            if (currentColoring[i] == -1){
                int neighbors = countDistinctNeighborColors(i, currentColoring);
                int degree = graph.getDegree(i);
                if (neighbors < maxNeighbors) {
                    continue;
                } else if (degree > maxDegree || maxNeighbors > neighbors){
                    maxNeighbors = neighbors;
                    maxDegree = degree;
                    bestindex = i;
                } 
            }
        }
        return bestindex;
    }


template <class VectorT>
bool VertexColoring<VectorT>::isSafe(const std::vector<int>& coloring, int vertex, int color) {
    // std::cout << " Here 11\n";
    for (int i = 0; i < maxClique.size(); ++i) { //loop over assigned vertices
        // std::cout << " Here 11\n";
        if (coloring[maxClique[i]] == color && graph.areNeighbours(vertex, maxClique[i])) {
            return false;
        }
    }
    // std::cout << " Here 22\n";
    for (int i = 0; i < vertex; ++i) { //loop over assigned vertices
        if (coloring[i] == color && graph.areNeighbours(vertex, i)) {
            return false;
        }
    }
    // std::cout << " Here 33\n";
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
    // int numVertices = graph.getNumVertices();
    
    // std::cout << "Coloring: ";
    // for (int i = 0; i < coloring.size(); i++){
    //     std::cout << coloring[i] << ", ";
    // }
    // std::cout << "\n";
        
    // if (coloring.size() != numVertices) {
    //     std::cout << "ERR: Coloring size is not the same as the number of vertices of the graph \n";
    //     return false;
    // }
    
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
