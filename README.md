# EuroHPC Student Challenge 2025
## Graph Vertex Coloring

- Supplementary materials for the challenge are in the `/supplementary_materials` folder.
- Problem instances are in the `/instances` and `/instances_optional` folder.


### Problem to solve
A vertex coloring is an assignment of labels or colors to each vertex of a graph such that no edge connects two identically colored vertices. The goal of this challenge is to minimize the number of colors for a given graph.

### Branch and bound
A parallel implementation of the Branch and bound method is expected for solving the graph vertex coloring problem. The MPI is recommended.

### Time limit
The optimal solution or best computed coloring needs to be delivered within the time limit of 10.000s regardless of computational resources used (CPUs, cores). This time limit needs to be implemented in the algorithm/code and should be allowed to be set by users as a parameter of a solver.

### Instances
The problem instances are in `/instances` and `/instances_optional` folders. The instances are benchmark vertex coloring instances from https://mat.tepper.cmu.edu/COLOR/instances. The webpage provides chromatic numbers for problem instances if known.  html in the DIMACS ASCII input fromat. 
Results (optimal coloring or best computed) should be delivered for all instances from the `/instances` folder.

### Output format
The output format for each problem instance is expected as an ASCII file as folows:

For the problem instance `problem_instance_xyz` an output file `problem_instance_xyz.output` needs to be generated:
    
```
problem_instance_file_name: problem_instance_xyz
number_of_vertices: 20
number_of_edges: 40
time_limit_sec: 10000
number_of_worker_processes: 5
number_of_cores_per_worker: 6
wall_time_sec: 2456
is_optimal: true
is_within_time_limit: true
number_of_colors: 10
0 1
1 9
2 3
. 
.
.
19 5
```
The `wall_time_sec` should represent the total running time of the solver including input/output data processingm worker scheduling and communication. We recommend to measure this value on the master process right after `MPI_Init()` and before `MPI_Finalize()` if MPI is used. Note, that `is_optimal` should not be set to `true` if the solver deasn't complete the computation within the requested time limit. The computed coloring should provided as a two column `vertex color` map be right bellow the `number_of_colors` parmeter.

### Repositories

Each team is required to use one of the following repositories for code developing and releesing the final release. The final version needs to be tagged as vM.m.p (using Major.minor.patch convention). 

- https://github.com/Rudolfovoorg/EuroHPC_Student_Challenge_2025_Team_1
- https://github.com/Rudolfovoorg/EuroHPC_Student_Challenge_2025_Team_2
- https://github.com/Rudolfovoorg/EuroHPC_Student_Challenge_2025_Team_3
- https://github.com/Rudolfovoorg/EuroHPC_Student_Challenge_2025_Team_4
- https://github.com/Rudolfovoorg/EuroHPC_Student_Challenge_2025_Team_5
- https://github.com/Rudolfovoorg/EuroHPC_Student_Challenge_2025_Team_6


### Q&A:
For any further questions please contact janez.povh@rudolfov.eu or roman.kuzel@rudolfovo.eu.

### References

https://mat.tepper.cmu.edu/COLOR/instances

https://mathworld.wolfram.com/VertexColoring.html

https://en.wikipedia.org/wiki/Branch_and_bound


