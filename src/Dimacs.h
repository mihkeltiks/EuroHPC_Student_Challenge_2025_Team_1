#ifndef DIMACS_LOADER_H
#define DIMACS_LOADER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <utility>

class DimacsLoader {
public:
    typedef std::pair<int, int> Edge;

private:
    std::vector<Edge> edges;
    std::vector<int> degrees;
    unsigned int numVertices;
    unsigned long maxVertexIndex;
    unsigned long long int adjacencyMatrixSizeLimit;
    std::string error;
    bool errorFlag;
    bool edgeNotSpecified;

public:
    DimacsLoader();
    ~DimacsLoader();
    
    bool load(const char* fname);
    unsigned int getNumVertices() const { return numVertices; }
    unsigned int getMaxVertexIndex() const { return maxVertexIndex; }
    unsigned int getNumEdges() const { return edges.size(); }
    std::vector<std::vector<char>> getAdjacencyMatrix() const;
    std::vector<int> getDegrees() const { return degrees; }
    const std::string& getError() const { return error; }
    bool verticesAreMappedFrom1based() const { return false; }

private:
    bool parseProblemLine(std::istringstream& ss, std::string& line);
    bool parseSpecsLine(std::istringstream& ss, std::string& line);
    bool parseLine(std::istringstream& ss, std::string& line);
};

DimacsLoader::DimacsLoader() : numVertices(0), adjacencyMatrixSizeLimit(1000000000), errorFlag(false), edgeNotSpecified(false) {}
DimacsLoader::~DimacsLoader() {}

bool DimacsLoader::load(const char* fname) {
    std::ifstream file(fname);
    if (!file) {
        error = "ifstream invalid - file cannot be opened for reading";
        return false;
    }
    
    auto loop = [&file, this](bool (DimacsLoader::*f)(std::istringstream&, std::string&)) -> bool {
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                std::istringstream ss(line);
                ss.ignore(2);
                if (!(this->*f)(ss, line)) {
                    error = "Error parsing line: " + line;
                    return false;
                }
            }
        }
        return !errorFlag;
    };
    
    return loop(&DimacsLoader::parseLine);
}

std::vector<std::vector<char>> DimacsLoader::getAdjacencyMatrix() const {
    if (maxVertexIndex * maxVertexIndex > adjacencyMatrixSizeLimit)
        throw std::runtime_error("Cannot create adjacency matrix: too many vertices");
    
    std::vector<std::vector<char>> matrix(maxVertexIndex, std::vector<char>(maxVertexIndex, 0));
    for (const auto& edge : edges) {
        matrix[edge.first][edge.second] = 1;
        matrix[edge.second][edge.first] = 1;
    }
    return matrix;
}

bool DimacsLoader::parseProblemLine(std::istringstream& ss, std::string& line) {
    if (line[0] == 'p') {
        if (!edgeNotSpecified) {
            std::string word;
            ss >> word;
            if (word != "edge") {
                error = "Invalid format - missing edge specification";
                errorFlag = true;
                return false;
            }
        }
        size_t nEdges = 0;
        ss >> numVertices >> nEdges;
        maxVertexIndex = numVertices + 1;
        edges.reserve(nEdges);
        degrees.resize(maxVertexIndex, 0);
    }
    return true;
}

bool DimacsLoader::parseSpecsLine(std::istringstream& ss, std::string& line) {
    if (line[0] == 'e') {
        unsigned int v1, v2;
        ss >> v1 >> v2;
        if (!ss || v1 == 0 || v2 == 0 || v1 > numVertices || v2 > numVertices) {
            error = "Malformed edge specification: " + line;
            errorFlag = true;
            return false;
        }
        edges.emplace_back(v1, v2);
        degrees[v1]++;
        degrees[v2]++;
    }
    return true;
}

bool DimacsLoader::parseLine(std::istringstream& ss, std::string& line) {
    return parseProblemLine(ss, line) && parseSpecsLine(ss, line);
}

#endif // DIMACS_LOADER_H
