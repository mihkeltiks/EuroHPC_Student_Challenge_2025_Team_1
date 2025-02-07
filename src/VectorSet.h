#ifndef VSET_H
#define VSET_H


// #include "MaximumCliqueBase.h"


// Set of vertices is based on std::vector
// used as VertexSetRepresentation in MaximumCliqueProblem
template<class T>
class VectorSet : public std::vector<T> {
public:
    typedef T VertexId;
    
    using std::vector<T>::resize;
    using std::vector<T>::reserve;
    void add(const T& value) {this->push_back(value);}
    T pop() {T temp=this->back(); this->pop_back(); return temp;}
    void remove(const T& value) {
        if (this->back() == value) this->pop_back();
        else {
            auto temp = std::find(this->begin(), this->end(), value);
            if (temp != this->end())
                this->erase(temp);
        }
    }
    using std::vector<T>::size;
    using std::vector<T>::empty;
    using std::vector<T>::operator[];
    using std::vector<T>::clear;
    using std::vector<T>::back;
    template<class AdjSet>
    friend void intersectWithAdjecency (const VectorSet& v, const AdjSet& adj, VectorSet& result) {
        auto n = v.size();
        result.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            if (adj[v[i]])
                result.add(v[i]);
        }
    }
    
    bool contains(VertexId v) const {
        return std::find(this->cbegin(), this->cend(), v) != this->cend();
    }

    // only required for debugging
    bool isIntersectionOf(const VectorSet& bigSet) {
        size_t n = bigSet.size();
        if (n < 1) return false;
        --n;
        for (size_t i = 0; i < size(); ++i) {
            for (size_t j = 0; bigSet[j] != (*this)[i]; ++j) {
                if (j == n) return false;
            }
        }
        return true;
    }
};

/**
 * @brief Clear target vector and fill it with range [min ... max)
 * 
 * @tparam T 
 * @param vec 
 * @param min 
 * @param max 
 */
template<class T>
void fillWithRange(VectorSet<T>& vec, int min, int max) {
    vec.clear();
    vec.resize(max-min);
    for (int i = min; i < max; i++) 
        vec[i-min] = i;
}

template<class Ostream, class T>
Ostream& operator<< (Ostream& out, const VectorSet<T>& vec) {
    if (vec.size() > 0) {
        out << "[" << vec[0];
        for (size_t i = 1; i < vec.size(); ++i)
            out << "," << vec[i]; 
        out << "]";
    } else 
        out << "[/]";
    return out;
}


#endif // VSET_H
