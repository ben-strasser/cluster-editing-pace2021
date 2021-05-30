# Cluster Editing Pace 2021

This repository contains a simple heuristic solution to the cluster editing problem. 
The program is submitted to the [PACE Challenge 2021](https://pacechallenge.org/2021/).

The cluster editing problem consists of removing or inserting as few edges to an undirected graph to obtain a graph where every connected component is a clique.

## Compilation & Usage

The whole program is contained in a single C++ file with no dependencies beyond the STL. 
You can use it as follows:

```
g++ -O3 main.cpp -o prog
./prog < input_in_pace_2021_graph_format.gr > output_in_pace_2021_solution_format.sol
```

The program will run until you hit CTRL+C. It will output the best solution found until then.

## Algorithm

The algorithm represents the edited graph using a node coloring of the input graph.
Two nodes are part of the same clique in the edited graph, if and only if, they share the same color.
The objective is to find a coloring such that the number of induced edge edits is minimum.

The algorithm starts with a coloring where every node has its own color.
It then modifies the solution in rounds until CTRL+C is hit.
The best solution seen over all rounds is outputted.

A round consists of greedily moving nodes.
To move a node `v`, we check for every neighbor `u` of `v` whether changing the color of `v` to the color of `u` decreases the induced edit count.
If this is the case, then `v` changes its color.
The end of a round is reached when no such improvement is possible anymore.
The order in which nodes are moved within a round is randomized.
Different rounds have a different order.

An important corner case is when we can change the color of `v` to the color `u` and the edit count stays the same.
We want to have a consistent tie-breaking in this case.
For this reason, at the start of each round, we determine a random order of colors that is used to break such ties.

After several rounds, our algorithm often gets stuck in a local optima.
To break out of local optima, we modify the edit costs for a few rounds.
Deleting or inserting edges has the double cost.
By increasing insertion costs, we can break up large clusters that should be split.
Similarly, an increased deletion cost allows us to join clusters that should be merged.

