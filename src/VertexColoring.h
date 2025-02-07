#ifndef VERTEX_COLORING_H
#define VERTEX_COLORING_H

#include <vector>
#include "Graph.h" // Include your Graph class header
#include <VectorSet.h>

template<class VectorT>
class VertexColoring {
public:
    Graph<VectorT>& graph;  // Reference to the graph

    // Constructor
    VertexColoring(Graph<VectorT>& g);

    // Function to color the graph
    int colorGraph();

private:
    std::vector<int> vertexColors;  // Stores the color assigned to each vertex
    int maxColors;  // To track the maximum number of colors used so far

    // Function to check if it's safe to assign a color to a vertex
    bool isSafe(int vertex, int color);

    // Recursive function to solve the coloring problem
    bool solve(int vertex);
};

// ========================= Implementation =========================

template <class VectorT>
VertexColoring<VectorT>::VertexColoring(Graph<VectorT>& g) : graph(g), maxColors(0) {
    vertexColors.resize(graph.adjacencyMatrix.size(), -1); // -1 means uncolored
}

template <class VectorT>
bool VertexColoring<VectorT>::isSafe(int vertex, int color) {
    // Check if assigning color to the vertex is safe
    std::cout << "Want to check vertex " << vertex << " for color " << color << "\n";
    for (int i = 0; i < graph.getNumVertices(); i++) {
        if (graph.areNeighbours(i,vertex)){
            std::cout << i << " with color " << vertexColors[i] << ", safe " << (vertexColors[i] == color) << "\n";
            if (vertexColors[i] == color) {
                return false;
            }
        }
    }
    return true;
}

template <class VectorT>
bool VertexColoring<VectorT>::solve(int vertex) {
    std::cout << "Graph size " << graph.adjacencyMatrix.size() << ", vertexcolors size" << vertexColors.size() << "\n";
    if (vertex == graph.adjacencyMatrix.size()) {
        return true; // All vertices are colored
    }

    for (int color = 0; color < maxColors + 1; ++color) {
        if (isSafe(vertex, color)) {
            vertexColors[vertex] = color;

            if (solve(vertex + 1)) {
                return true; // If we find a valid coloring, return true
            }

            vertexColors[vertex] = -1; // Backtrack
        }
    }

    return false; // If no color works, return false
}

template <class VectorT>
int VertexColoring<VectorT>::colorGraph() {
    // Try to color the graph with a number of colors
    bool foundSolution = false;
    bool debugOutput = true;
    std::cout << "COLORINGDEBUG \n";
    graph.debugOut();
    for (maxColors = 0; maxColors < graph.adjacencyMatrix.size(); ++maxColors) {
        vertexColors.assign(graph.adjacencyMatrix.size(), -1); // Reset colors
        foundSolution = solve(1);
        if (foundSolution) {
            if (debugOutput){
                std::cout << "Found valid coloring: ";
                for (int i = 0; i < vertexColors.size(); i++){
                    std::cout << vertexColors[i] << ", ";
                }
                std::cout << "\n";
            }
            break;
        }
    }

    return maxColors + 1; // The minimum number of colors used
}

#endif // VERTEX_COLORING_H
