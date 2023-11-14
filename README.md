# Speeding-up-dynamic-transitive-closure-for-bounded-degree-graphs

**Implementation of "Speeding up dynamic transitive closure for bounded degree graphs" using C++ LEDA lib** 

In this project an algorithm for solving two problems in dynamically maintaining the transitive closure of a digraph is presented:
In the first problem a sequence of edge insertions is performed on an initially empty graph, interspersed with p transitive closure queries of the form:
"is there a path from a to b in the graph". Our algorithm solves this problem in time O(dm*+p), where d is the maximum outdegree of the resulting graph G and m* is the number of edges in the transitive closure of G.
In the second problem, a sequence of edge deletions is performed on an acyclic graph, interspersed with p transitive closure queries.
Once again we solve this problem in time O(dm*+p), where d is the maximum outdegree of the initial graph G and m* is the number of edges in the transitive closure of G.
For bounded degree graphs, this improves upon previous results. Our algorithms also work when insertions and deletions to the graph are intermixed.
Finally, we show how to implement the operation findpath(x, y) which retrieves some path from x to y in time proportional to the length of the path.
