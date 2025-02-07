#ifndef GRAPH_LABELS_H
#define GRAPH_LABELS_H


#include <set>
#include <vector>
#include <cstddef> // for size_t

// #include "MaximumCliqueBase.h"


/**
 * @brief Information about labels (both vertex and edge) for a graph.
 * Labels are simple integers.
 * TODO: currently the vectors of labels are public so that Graph<...> can access them easily
 * 
 * @tparam LabelRepresentation an integer type of sufficient bitwidth to hold all possible labels
 */
template<class LabelRepresentation>
class GraphLabels {
public:
    std::vector<std::vector<LabelRepresentation>> edgeLabels;
    std::vector<LabelRepresentation> vertexLabels;

public:
    template<class LR2>
    GraphLabels<LabelRepresentation>& operator=(const GraphLabels<LR2>& other) {
        edgeLabels.resize(other.edgeLabels.size());
        for (size_t i = 0; i < edgeLabels.size(); ++i) {
            edgeLabels[i].resize(other.edgeLabels[i].size()); 
            std::copy(other.edgeLabels[i].begin(), other.edgeLabels[i].end(), edgeLabels[i].begin());
        }

        vertexLabels.resize(other.vertexLabels.size());
        std::copy(other.vertexLabels.begin(), other.vertexLabels.end(), vertexLabels.begin());

        return *this;
    };

    /**
     * @brief Calling this function will perform the count of vertex labels. O(n)
     * 
     * @return size_t
     */
    size_t getNumVertexLabels() const {
        std::set<unsigned> labelsSet;
        
        for (auto const& e : vertexLabels) {
            labelsSet.insert(e);
        }
        
        return labelsSet.size();
    }

    /**
     * @brief Calling this function will count the edge labels. O(n²)
     * 
     * @return size_t 
     */
    size_t getNumEdgeLabels() const {
        std::set<unsigned> labelsSet;
    
        for (auto const& edgeList : edgeLabels) {
            for (auto const& e : edgeList) {
                labelsSet.insert(e);
            }
        }
        
        return labelsSet.size()-(labelsSet.count(0) > 0 ? 1 : 0);
    }

    void initializeEdgeLabels(size_t size) {
        edgeLabels.resize(size);
        for (auto& e : edgeLabels)
            e.resize(size);
    }

    /**
     * @brief Calling this function will count the edge labels. O(n²)
     * 
     * @return std::vector<LabelRepresentation> 
     */
    const std::vector<LabelRepresentation>& getVertexLabels() const {
        return vertexLabels;
    }

    /**
     * @brief Get a vector of edge labels (will make a new vector), O(n)
     * 
     * @return std::vector<std::vector<LabelRepresentation> > 
     */
    const std::vector<std::vector<LabelRepresentation> >& getEdgeLabels() const {
        return edgeLabels;
    }

    /**
     * @brief Calling this will clear all edge labels 
     */
    void clearEdgeLabels() {
        edgeLabels.clear();
    }

    /**
     * @brief Calling this will clear all vertex labels (graph will no longer be vertex-labelled)
     */
    void clearVertexLabels() {
        vertexLabels.clear();
    }

    /**
     * @brief This function will map labels to numbers from 1-n
     */
    void normalizeLabels() {
        // Edge labels go first
        {
            std::map<unsigned, unsigned> edgeLabelMap;
            // always map 0 to 0 (edge does not exist)
            edgeLabelMap[0] = 0;
            unsigned lastNumber = 0;

            for (auto const& edgeList : edgeLabels) {
                for (auto& quaziColor : edgeList) {
                    auto iMap = edgeLabelMap.find(quaziColor);
                    if (iMap == edgeLabelMap.end()) {
                        // new number mapping
                        lastNumber++;
                        edgeLabelMap[quaziColor] = lastNumber;
                        quaziColor = lastNumber;
                    } else {
                        // already a valid number mapping
                        quaziColor = iMap->second;
                    }
                }
            }
        }

        // Vertex labels
        {
            std::map<unsigned, unsigned> vertexLabelMap;
            // always map 0 to 0 (edge does not exist)
            unsigned lastNumber = 0;

            for (auto& quaziColor : vertexLabels) {
                auto iMap = vertexLabelMap.find(quaziColor);
                if (iMap == vertexLabelMap.end()) {
                    // new number mapping
                    lastNumber++;
                    vertexLabelMap[quaziColor] = lastNumber;
                    quaziColor = lastNumber;
                } else {
                    // already a valid number mapping
                    quaziColor = iMap->second;
                }
            }
        }
    }
        
    /**
     * @brief Turn the graph into undirected: make adjacency matrix symmetric across the diagonal
     */
    void remapTo_undirected() {
        size_t size = vertexLabels.size();

        // copy edge labels over the diagonal
        for (size_t i = 0; i < size; ++i)
            for (size_t j = 0; j < size; ++j)
                if (edgeLabels[i][j] > 0)
                    edgeLabels[j][i] = edgeLabels[i][j];
    }

    /**
     * @brief Turn the graph into unlabelled: clear vertex and edge labels
     */
    void remapTo_unlabelled() {
        size_t size = vertexLabels.size();

        // clamp the edge labels, remove vertex labels
        for (size_t i = 0; i < size; ++i) {
            vertexLabels[i] = 0;
            for (size_t j = 0; j < size; ++j)
                if (edgeLabels[i][j] > 0)
                    edgeLabels[i][j] = 1;
        }
    }

//     /**
//      * @brief remap labels into a more sensible range (this makes several assumptions, be sure to understand them before using it)
//      * 
//      * Only use this function to make the loaded graphs comparable to those used by Mccreesh 2016
//      * Assumptions:
//      *  - labels are distributed across the whole 16-bit integer range
//      *  - 
//      */
//     void remapTo_CP2011() {
//         // Warning: labelling information is changed to decrease the number of labels
//         // Originally this reduction algorithm was used by Ciaran McCreesh in his 2016 paper
//         // to be like the CP 2011 labelling scheme...
//         size_t size = vertexLabels.size();

//         // calculate an appropriate number of labels to use
//         int m = size * 33 / 100;
//         int p = 1, k1 = 16, k2 = 0;
//         while (p < m && k1 > 0) {
//             p *= 2;
//             k1 = 16-k2;
//             k2++;
//         }

//         for (auto& l : vertexLabels)
//             l >>= k1;

//         for (auto& e : edgeLabels)
//             for (auto& l : e)
//                 if (l > 0) 
//                     l = (l >> k1) + 1;
//     }
};

#endif //GRAPH_LABELS_H