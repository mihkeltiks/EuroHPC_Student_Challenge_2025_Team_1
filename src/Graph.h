#ifndef GRAPH_H
#define GRAPH_H


#include <vector>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <utility>
#include <bitset>
#include <map>
#include <numeric>
#include "GraphLabels.h"

template<class VectorT>
class Graph {
public:
    typedef typename VectorT::VertexId VertexId;
    typedef VectorT VertexSet;

    const long int adjacencyMatrixMaxNodes = 10000;      // 1000 nodes translates into 1M node total matrix size, and 10000 -> 100M
    bool wasRemapedTo0based = false;                 // this will be set to true if graph is loaded from a file type that is 1-based
    std::vector<VectorT> adjacencyMatrix, invAdjacencyMatrix;
    std::vector<int> degrees;
    std::vector<VertexId> mapping;
    // by default, labels are empty, and for all uses, the graph should be considered unlabelled
    GraphLabels<VertexId> labels;
    
    void debugOut() const {
        std::cout << "DEBUG adjacency size = " << adjacencyMatrix.size() << ", " << invAdjacencyMatrix.size() << "\n";

        // Print degrees
        std::cout << "DEBUG degrees = " << degrees.size() << ": ";
        for (const auto& d : degrees) {
            std::cout << d << " ";
        }
        std::cout << "\n";

        // Print mapping
        std::cout << "DEBUG mapping = " << mapping.size() << ": ";
        for (const auto& m : mapping) {
            std::cout << m << " ";
        }
        std::cout << "\n\n";

        bool validLabels = (labels.getNumVertexLabels() > 1) || (labels.getNumEdgeLabels() > 1);
        if (degrees.size() <= 10) {
            size_t n = degrees.size();
            for (size_t i = 0; i < n; ++i) {
                std::cout << (wasRemapedTo0based ? i+1 : i);
                if (validLabels)
                    std::cout << "(" << labels.getVertexLabels()[i] << ")";
                std::cout << ": ";
                bool comma = false;
                for (size_t j = i+1; j < n; ++j) {
                    if (adjacencyMatrix[i][j]) {
                        std::cout << (comma ? "," : "") << (wasRemapedTo0based ? j+1 : j);
                        comma = true;
                    }
                }
                std::cout << "\n";
            }
        }
    }

    void debugAdjacencyOut() const {
        size_t n = getNumVertices();
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                std::cout << (adjacencyMatrix[i][j] ? "█" : "░");
            }
            std::cout << "\n";
        }        
    }
    
    Graph() {}
    
    Graph(Graph&& other) {
        std::swap(adjacencyMatrix, other.adjacencyMatrix);
        std::swap(invAdjacencyMatrix, other.invAdjacencyMatrix);
        std::swap(degrees, other.degrees);
        std::swap(mapping, other.mapping);
        std::swap(labels, other.labels);
    }
    
    const Graph& operator= (Graph&& other) {
        std::swap(adjacencyMatrix, other.adjacencyMatrix);
        std::swap(invAdjacencyMatrix, other.invAdjacencyMatrix);
        std::swap(degrees, other.degrees);
        std::swap(mapping, other.mapping);
        std::swap(labels, other.labels);
        return *this;
    }
    
    const Graph& operator= (const Graph& other) {
        adjacencyMatrix = other.adjacencyMatrix;
        invAdjacencyMatrix = other.invAdjacencyMatrix;
        degrees = other.degrees;
        mapping = other.mapping;
        labels = other.labels;
        return *this;
    }
    
    template<class T>
    struct isTypeASet {
        enum {value = false};
    };

    /**
     * @brief Invert the edges (not connected vertices become connected and vice-versa)
     * This operation will invalidate edge labels
     */
    void invertEdges() {
        size_t n = adjacencyMatrix.size();
        for (size_t i = 0; i < n; ++i) {
            if (isTypeASet<VectorT>::value) {
                for (size_t j = 0; j < n; ++j) {
                    adjacencyMatrix[i][j] = !adjacencyMatrix[i][j];
                    invAdjacencyMatrix[i][j] = !invAdjacencyMatrix[i][j];
                }
                invAdjacencyMatrix[i][i] = false;
                adjacencyMatrix[i][i] = false;
            } else {
                // MD: this branch of code has not been verified yet
                auto temp = adjacencyMatrix[i];
                adjacencyMatrix[i] = invAdjacencyMatrix[i];
                invAdjacencyMatrix[i] = temp;
            }
            degrees[i] = n-degrees[i]-1;
        }
        labels.clearEdgeLabels();
    }
    
    unsigned int getNumVertices() const {return adjacencyMatrix.size();}
    
    unsigned int getNumEdges() const {
        unsigned int numEdges = 0;
        for (size_t i = 0; i < degrees.size(); ++i) {
            numEdges += degrees[i];
        }
        return numEdges / 2;
    }
    
    bool createAdjacencyMatrix(unsigned int n) {
        if (n > adjacencyMatrixMaxNodes)
            return false;
        
        adjacencyMatrix.resize(n); 
        invAdjacencyMatrix.resize(n); 
        for (size_t i = 0; i < n; ++i) {
            adjacencyMatrix[i].resize(n, false);
            invAdjacencyMatrix[i].resize(n, false);
        }
        return true;
    }
    
    /**
     * @brief Initialize from adjacency matrix and degrees
     * 
     * @param adjacency the adjacency matrix for verices 
     * @param d         the vector of vertex degrees
     */
    void init(const std::vector<std::vector<char> >& adjacency, const std::vector<int>& d) {
        size_t n = adjacency.size();
        adjacencyMatrix.resize(n); 
        invAdjacencyMatrix.resize(n); 
        for (size_t i = 0; i < n; ++i) {
            adjacencyMatrix[i].resize(n, false);
            invAdjacencyMatrix[i].resize(n, false);
            invAdjacencyMatrix[i][i] = false;
            for (size_t j = i+1; j < adjacency[i].size(); ++j) {
                adjacencyMatrix[i][j] = adjacency[i][j];
                invAdjacencyMatrix[i][j] = (adjacency[i][j] == false);
            }
        }
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < i; ++j) {
                adjacencyMatrix[i][j] = adjacency[j][i];
                invAdjacencyMatrix[i][j] = (adjacency[j][i] == false);
            }
        }
        degrees = d;
        mapping.clear();
    }

    /**
     * @brief Initialize from adjacency matrix, degrees, and labels (for vertices and edges)
     * 
     * @param adjacency the adjacency matrix of vertices
     * @param d         vector of vertex degrees
     * @param labels    labels for vertices and/or edges
     */
    void init(const std::vector<std::vector<char> >& adjacency, 
        const std::vector<int>& d, const GraphLabels<uint16_t>& labels) {
        init(adjacency, d);
        this->labels = labels;
    }
    
    /**
     * @brief When constructing graph manually, use this function, after the adjacency matrix has been set, to calculate and store the node degrees
     */
    void calculateNodeDegrees() {
        size_t n = adjacencyMatrix.size();
        degrees.resize(n, 0);
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i+1; j < n; ++j) {
                if (adjacencyMatrix[i][j]) {
                    degrees[i]++;
                    degrees[j]++;
                }
            }
        }
    }

    /**
     * @brief Find an approximation of the maximum clique using a greedy approach
     * @return VectorT containing the vertices that form an approximate maximum clique
     */
    VectorT findMaxCliqueApprox() {
        // compute degrees
        calculateNodeDegrees();
        
        size_t n = getNumVertices();
        
        // Create vertex-degree pairs for sorting (may be parallelizable)
        std::vector<std::pair<VertexId, int>> vertexDegrees;
        vertexDegrees.reserve(n);
        for (VertexId i = 0; i < n; ++i) {
            vertexDegrees.push_back({i, degrees[i]});
        }
        
        // Sort vertices by degree in descending order
        std::sort(vertexDegrees.begin(), vertexDegrees.end(),
            [](const std::pair<VertexId, int>& a, const std::pair<VertexId, int>& b) { 
                return a.second > b.second; 
            });

        // Initialize result vector with highest degree vertex
        VectorT clique;
        clique.add(vertexDegrees[0].first);
        
        // Batch processing of candidates (parallelizable)
        const size_t batchSize = 128;
        for (size_t i = 1; i < n; i += batchSize) {
            size_t endIdx = std::min(i + batchSize, n);
            
            // Process a batch of candidates
            for (size_t j = i; j < endIdx; ++j) {
                VertexId candidate = vertexDegrees[j].first;
                bool canAdd = true;
                
                // Check if candidate forms clique with all current members
                size_t cliqueSize = clique.size();
                for (size_t k = 0; k < cliqueSize; ++k) {
                    if (!areNeighbours(candidate, clique[k])) {
                        canAdd = false;
                        break;
                    }
                }
                
                if (canAdd) {
                    clique.add(candidate);
                }
            }
        }

        return clique;
    }
    
    /**
     * @brief Perform intersection between a node #p's neighbours and a set of vertices #vertices, store the result in set of vertices #result
     * @param p         a vertex, whose neighbours will be taken as 1st set of input vertices
     * @param vertices  the 2nd set of input vertices
     * @param result    the output set of vertices (intersection of #vertices and #p's neighbours), ordered as they were ordered in #vertices
     */
    void intersectWithNeighbours(VertexId p, const VertexSet& vertices, VertexSet& result) const {
        // global function intersectWithAdjecency(VectorT, VectorT, VectorT) must be specified
        intersectWithAdjecency(vertices, adjacencyMatrix[p], result);
    }
    
    /**
     * @brief Returns weather the specified two vertices are neighbours
     * @param p     1st vertex
     * @param q     2nd vertex
     * @return      true if #p and #q are neighbours, false otherwise
     */
    bool areNeighbours(VertexId p, VertexId q) const {
        return (adjacencyMatrix[p][q]);
    }

    void setNeighbours(VertexId p, VertexId q, bool neighbours) {
        adjacencyMatrix[p][q] = neighbours;
    }
    
    /**
     * @brief A function which tells weather the intersection between #p's neighbours and another set of vertices is empty or not
     * @param p         an input vertex
     * @param vertices  an input set of vertices
     * @return          true if an intersection exist (#p is a neighbour of at least one of vertices listed in #vertices), false otherwise
     */
    bool intersectionExists(VertexId p, const VertexSet& vertices) const {
        size_t n = vertices.size();
        for (size_t i = 0; i < n; ++i) {
            if (adjacencyMatrix[p][vertices[i]])
                return true;
        }
        return false;
    }
    
    /**
     * @brief Change the order of vertices (renumber them)
     * @param order the order mapping; e.g. order = [3, 1, 2] will map old vertex 3 into new vertex 1, old vertex 1 into new vertex 2, and old vertex 2 into new vertex 3.
     */
    template<class Vec>
    void orderVertices(const Vec& order) {
        // check order vector
        size_t n = getNumVertices();
        if (order.size() != n) {
            //std::clog << " getNumVertices()=" << getNumVertices() << ", but order.size()=" << order.size() << "\n";
            throw std::runtime_error("Invalid size vector in orderVertices");
		  }
            
        // create a vertex mapping table that will be used to renumber vertices back to original
        if (mapping.size() == 0) {
            // create default mapping that maps i → i
            mapping.resize(n);
            for (size_t i = 0; i < n; ++i)
                mapping[i] = i;
        }
        decltype(mapping) mapping2(n);
        
        // remap to temporary adjacencyMatrix
        std::vector<VectorT> adjacencyMatrix2;
        adjacencyMatrix2.resize(n);
        for (size_t i = 0; i < n; ++i) {
            adjacencyMatrix2[i].resize(n);
            invAdjacencyMatrix[i].clear();
            invAdjacencyMatrix[i].resize(n);
            mapping2[i] = mapping[order[i]];
            auto& adjRowI = adjacencyMatrix[order[i]];
            for (size_t j = 0; j < n; ++j) {
                adjacencyMatrix2[i][j] = adjRowI[order[j]] == true;
                invAdjacencyMatrix[i][j] = (i != j) & !adjacencyMatrix2[i][j];
            }
            // the line above includes the condition (i != j) because:
            // adjacency inverse is used to filter out vertices (operator &) and it is useful if 
            // given a vertex, it filters out its neighbours as well as the vertex itself
            // therefore reset the edge linking vertices to themselves
        }
        std::swap(adjacencyMatrix2, adjacencyMatrix);
        std::swap(mapping2, mapping);

        auto oldDeg = degrees;
        for (size_t i = 0; i < degrees.size(); ++i) 
            degrees[i] = oldDeg[order[i]];

        GraphLabels<VertexId> newLabels;
        // only performproper re-labelling if number of labels matches number of vertices
        if (labels.vertexLabels.size() == n) {
            // map vertex labels
            newLabels.vertexLabels.resize(n);
            for (size_t i = 0; i < n; ++i)
                newLabels.vertexLabels[i] = labels.vertexLabels[order[i]];
        }
        if (labels.edgeLabels.size() == n) {
            // map edge labels
            newLabels.edgeLabels.resize(n);
            for (size_t i = 0; i < n; ++i) {
                if (labels.edgeLabels[i].size() != n) 
                    throw std::runtime_error("Error in edgeLabels, which is not a n×n matrix");
                auto& labI = labels.edgeLabels[order[i]];
                for (size_t j = 0; j < n; ++j) {
                    labels.edgeLabels[i][j] = labI[order[j]];
                }
            }

        }        
    }
    
    void sortVerticesByDegree() {
        size_t n = degrees.size();
        std::vector<int> order(n);

        std::iota(order.begin(), order.end(), 0);
        std::sort(order.begin(), order.end(), [&](size_t a, size_t b) {
            return degrees[a] > degrees[b];
        });

        orderVertices(order);
    }

    int getDegree(int vertex) {
        return degrees[vertex];
    }
    
    /**
     * @brief Reorder the vertices back to the original order.
     */
    void orderVertices() {
        if (mapping.size() > 0) {
            std::vector<size_t> order(getNumVertices());
            for (size_t i = 0; i < order.size(); ++i)  {
                order[mapping[i]]=i;
            }
            orderVertices(order);
        }
    }
    
    /**
     * @brief Remap vertices #v (in-place) back to the original ordering (and naming)
     * @param v an input set of vertices to be mapped and also the output set of mapped vertices
     */
    void remap(VectorT& v) const {
        if (mapping.size() == 0 || v.size() == 0) return;
        
        if (isTypeASet<VectorT>::value) {
            VectorT rv;
            rv.reserve(mapping.size());
            if (mapping.size() < v.size())
                throw std::runtime_error("Mapping failed, mapping is not known for all vertices");
            
            for (size_t i = 0; rv.size() < v.size(); ++i) {
                if (v[i]) rv.add(mapping[i]);
            }
            std::swap(rv, v);
        } else {
            for (size_t i = 0; i < v.size(); ++i) v[i] = mapping[v[i]];
        }
    }
    
    /**
     * @brief Remap vertices #v (in-place) back to the original ordering (and naming)
     * @param v an input set of vertices to be mapped and also the output set of mapped vertices
     */
    void remap0basedTo1based(VectorT& v) const {
        if (v.size() == 0) return;
        
        if (isTypeASet<VectorT>::value) {
            VectorT rv;
            rv.reserve(degrees.size()+1);
            
            for (size_t i = 0; rv.size() < v.size(); ++i) {
                if (v[i]) rv.add(i+1);
            }
            std::swap(rv, v);
        } else {
            for (size_t i = 0; i < v.size(); ++i) v[i] = v[i]+1;
        }
    }

    /**
     * @brief Get the Labelling Info object
     * 
     * @return const GraphLabels<VertexId>& 
     */
    const GraphLabels<VertexId>& getLabellingInfo() const {
        return labels;
    }

    /**
     * @brief Get the Labelling Info object
     * 
     * @return  GraphLabels<VertexId>& 
     */
    GraphLabels<VertexId>& getLabellingInfo() {
        return labels;
    }

    /**
     * @brief Export the adjacency matrix in a general format (vector of vector of char)
     * 
     * @return std::vector<std::vector<char> > 
     */
    std::vector<std::vector<char> > exportAdjacencyMatrix() const {
        std::vector<std::vector<char> > a;

        // copy adjacency matrix into a
        size_t n = adjacencyMatrix.size();
        a.resize(n); 
        for (size_t i = 0; i < n; ++i) {
            a[i].resize(n, false);
            for (size_t j = 0; j < adjacencyMatrix[i].size(); ++j) {
                a[i][j]=(bool)adjacencyMatrix[i][j];
            }
        }

        return a;
    }

    void removeVerticesWithLowDegree(int n, std::vector<int> maxClique) {
        size_t originalNumVertices = getNumVertices();
        std::vector<bool> verticesToRemove(originalNumVertices, false);
        int numRemoved = 0;
        // Mark vertices for removal
        for (size_t i = 0; i < originalNumVertices; ++i) {
            if (degrees[i] < n) {
                verticesToRemove[i] = true;
                numRemoved++;
            }
        }

        if (numRemoved == 0) return;

        std::vector<int> filteredVertices = filterVertices(maxClique);

        for (size_t i = 0; i < filteredVertices.size(); ++i) {
            if (!verticesToRemove[i]){
                verticesToRemove[i] = true;
                numRemoved++;
            }
        }


        std::vector<VectorT> newAdjacencyMatrix;
        std::vector<VectorT> newInvAdjacencyMatrix;
        std::vector<int> newDegrees;
        std::vector<VertexId> newMapping;
        GraphLabels<VertexId> newLabels;
        

        newAdjacencyMatrix.reserve(originalNumVertices - numRemoved);
        newInvAdjacencyMatrix.reserve(originalNumVertices - numRemoved);
        newDegrees.reserve(originalNumVertices - numRemoved);
        newMapping.reserve(originalNumVertices - numRemoved);


        std::vector<size_t> oldToNewIndex(originalNumVertices, -1);

        size_t newIndex = 0;
        for (size_t i = 0; i < originalNumVertices; ++i) {
            if (!verticesToRemove[i]) {
                oldToNewIndex[i] = newIndex++;
                newDegrees.push_back(degrees[i]);
                newMapping.push_back(mapping.empty() ? i : mapping[i]);
                if (labels.vertexLabels.size() == originalNumVertices)
                    newLabels.vertexLabels.push_back(labels.vertexLabels[i]);

                newAdjacencyMatrix.emplace_back();
                newInvAdjacencyMatrix.emplace_back();
            }
        }



        for (size_t i = 0; i < newIndex; ++i) {
            newAdjacencyMatrix[i].resize(newIndex, false);
            newInvAdjacencyMatrix[i].resize(newIndex, false);
            for (size_t j = 0; j < newIndex; ++j) {
                newAdjacencyMatrix[i][j] = adjacencyMatrix[getOldIndex(i, oldToNewIndex)][getOldIndex(j, oldToNewIndex)];
                newInvAdjacencyMatrix[i][j] = invAdjacencyMatrix[getOldIndex(i, oldToNewIndex)][getOldIndex(j, oldToNewIndex)];
            }
        }

        adjacencyMatrix = std::move(newAdjacencyMatrix);
        invAdjacencyMatrix = std::move(newInvAdjacencyMatrix);
        degrees = std::move(newDegrees);
        mapping = std::move(newMapping);
        labels = std::move(newLabels);

        debugOut();
    }

    size_t getOldIndex(size_t newIndex, const std::vector<size_t>& oldToNewIndex) const {
        for (size_t i = 0; i < oldToNewIndex.size(); ++i) {
            if (oldToNewIndex[i] == newIndex) {
                return i;
            }
        }
        return -1;
    }

    std::vector<int> filterVertices(const std::vector<int>& cliqueVertices) {
        std::vector<bool> isInClique(getNumVertices(), false);
        for (int v : cliqueVertices) {
            isInClique[v] = true;
        }

        std::vector<int> removedVertices;

        for (int v = 0; v < getNumVertices(); ++v) { // Go over vertices to eliminate
            bool hasValidNeighbor = false;
            if (isInClique[v]) continue;
            
            for (int cliqueV = 0; cliqueV < cliqueVertices.size(); cliqueV++) { // Go over vertices in clique
                if (areNeighbours(v,cliqueV)){
                    hasValidNeighbor = false;
                    break;
                }
                for (int u = 0; u < getNumVertices(); u++) { // Try to find connecting vertex between them
                    if (!isInClique[u] && areNeighbours(cliqueV,u) && areNeighbours(u,v)) {
                        hasValidNeighbor = true;
                        break;
                    }
                }
                if (hasValidNeighbor) break;
            }
            if (hasValidNeighbor) {
                removedVertices.push_back(v);
            }
        }

        
        return removedVertices;
    }

};


#endif // GRAPH_H
