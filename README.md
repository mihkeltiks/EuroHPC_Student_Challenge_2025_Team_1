# EuroHPC Student Challenge 2025 Team 1
## Graph Vertex Coloring

- Supplementary materials for the challenge are in the `/supplementary_materials` folder.
- Problem instances are in the `/instances` and `/instances_optional` folder.


### Problem to solve
A vertex coloring is an assignment of labels or colors to each vertex of a graph such that no edge connects two identically colored vertices. The goal of this challenge is to minimize the number of colors for a given graph.

### Running
Makefiles are provided for Intel and GNU compilers. To compile on VEGA and run on a simple triangular graph with 1 thread:

```bash
ml oneapi/compiler/latest
CXX=icx make -j
OMP_NUM_THREADS=1 ./chromatic ../instances/triangle.clq
```

A batch script `vega.batch` is also provdided.
