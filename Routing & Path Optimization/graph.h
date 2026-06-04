#ifndef STUDENT_GRAPH
#define STUDENT_GRAPH

#include "binary_heap.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

class Graph {
public:
    // Simple struct to pack a target vertex and its weight.
    // Much easier to read edge.to and edge.weight than dealing with pair.first/second.
    struct Edge {
        int to;
        double weight;
    };

    // Bundle vertex and its tentative cost together for the priority queue.
    struct DijkstraNode {
        int vertex;
        double cost;

        // Without this default constructor, the build breaks immediately because the compiler 
        // doesn't know how to fill those generic blank slots before we start inserting real nodes.
        DijkstraNode() : vertex(0), cost(0.0) {}
        DijkstraNode(int v, double c) : vertex(v), cost(c) {}

        // For example, when percolating down, it evaluates: `if (array[child+1] < array[child])`.
        // If Node A has cost 0.2 and Node B has cost 10.1, we want Node A to take priority.
        // Overloading '<' to compare costs directly guarantees that the cheapest path always floats up.
        bool operator<(const DijkstraNode& other) const {
            return cost < other.cost;
        }
    };

    Graph() : num_vertices_(0) {}

    // If I allocate an array of size 5, valid indices are 0 to 4. But the input references nodes 1 to 5.
    // By sizing the vector to (num_vertices_ + 1) -> 6, indices 1 to 5 become completely safe to access.
    // This saves me from having to offset index keys with a '- 1' everywhere, preventing off-by-one errors.
    void init(int num_vertices) {
        num_vertices_ = num_vertices;
        adj_.assign(num_vertices_ + 1, std::vector<Edge>());
    }

    void addEdge(int from, int to, double weight) {
        // Guard rails to catch any broken or out-of-bounds node references.
        if (from >= 1 && from <= num_vertices_ && to >= 1 && to <= num_vertices_) {
            adj_[from].push_back({to, weight});
        }
    }

    // 1. getline() reads the string row. ss extracts `from = 1`.
    // 2. The inner while loop triggers: `while(ss >> to >> weight)`
    //    - First iteration: extracts `to = 2`, `weight = 0.2`. Calls addEdge(1, 2, 0.2).
    //    - Second iteration: extracts `to = 4`, `weight = 10.1`. Calls addEdge(1, 4, 10.1).
    //    - Third iteration: extracts `to = 5`, `weight = 0.5`. Calls addEdge(1, 5, 0.5).
    // Loop terminates when the end of the line string is reached.
    bool loadGraphFromFile(const std::string& filename) {
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            return false;
        }
        std::string line;
        if (std::getline(infile, line)) {
            std::stringstream ss(line);
            int num_vertices;
            if (ss >> num_vertices) {
                init(num_vertices);
            }
        }
        while (std::getline(infile, line)) {
            if (line.empty()) continue; // Skips trailing blank rows
            std::stringstream ss(line);
            int from;
            if (!(ss >> from)) continue; 
            
            int to;
            double weight;
            while (ss >> to >> weight) {
                addEdge(from, to, weight);
            }
        }
        return true;
    }

    // Linear search down a specific adjacency row to retrieve a connection's weight for Q1 queries.
    bool getEdgeWeight(int from, int to, double& weight) const {
        if (from < 1 || from > num_vertices_ || to < 1 || to > num_vertices_) {
            return false;
        }
        for (const auto& edge : adj_[from]) {
            if (edge.to == to) {
                weight = edge.weight;
                return true;
            }
        }
        return false;
    }

    int getNumVertices() const {
        return num_vertices_;
    }

    // Initial State:
    //   distances array:     [inf,   0, inf, inf, inf, inf]
    //   path_prefixes array: [ -1,  -1,  -1,  -1,  -1,  -1]
    //   Heap contents:       [(v:1, c:0.0)]
    //
    // Iteration 1:
    //   pq.deleteMin() returns vertex 1 (cost 0.0).
    //   Evaluating neighbors of 1:
    //     - Neighbor 2: 0.0 + 0.2 = 0.2 < inf. Update distances[2]=0.2, prefix[2]=1. Push (v:2, c:0.2).
    //     - Neighbor 4: 0.0 + 10.1 = 10.1 < inf. Update distances[4]=10.1, prefix[4]=1. Push (v:4, c:10.1).
    //     - Neighbor 5: 0.0 + 0.5 = 0.5 < inf. Update distances[5]=0.5, prefix[5]=1. Push (v:5, c:0.5).
    //
    // Iteration 2:
    //   pq.deleteMin() returns vertex 2 (cost 0.2).
    //   Evaluating neighbors of 2:
    //     - Neighbor 1: 0.2 + 1.5 = 1.7 > distances[1] (which is 0). Ignore.
    void dijkstra(int start_vertex, std::vector<double>& distances, std::vector<int>& path_prefixes) const {
        distances.assign(num_vertices_ + 1, std::numeric_limits<double>::infinity());
        path_prefixes.assign(num_vertices_ + 1, -1);

        if (start_vertex < 1 || start_vertex > num_vertices_) {
            return;
        }

        distances[start_vertex] = 0.0;
        BinaryHeap<DijkstraNode> pq;
        pq.insert(DijkstraNode(start_vertex, 0.0));

        while (!pq.isEmpty()) {
            DijkstraNode current;
            pq.deleteMin(current); 

            int u = current.vertex;
            double d = current.cost;

            // Suppose vertex 4 is initially added with a cost of 10.1 (via node 1). 
            // Later, we find a shorter path to vertex 4 through vertex 5 that costs only 5.0. 
            // We can't update the heap item directly, so we push a duplicate node: (v:4, c:5.0).
            //
            // The heap now contains both (v:4, c:5.0) and (v:4, c:10.1).
            // 1. (v:4, c:5.0) pops first because it is cheaper. distances[4] updates to 5.0.
            // 2. Later, (v:4, c:10.1) bubbles up and pops. 
            //    Our condition runs: `if (10.1 > distances[4])` -> `10.1 > 5.0` is true!
            // This lets us discard the outdated duplicate immediately, preventing redundant edge updates and timeouts.
            if (d > distances[u]) {
                continue;
            }

            for (const auto& edge : adj_[u]) {
                int v = edge.to;
                double weight = edge.weight;
                if (distances[u] + weight < distances[v]) {
                    distances[v] = distances[u] + weight;
                    path_prefixes[v] = u; 
                    pq.insert(DijkstraNode(v, distances[v]));
                }
            }
        }
    }

private:
    int num_vertices_;
    // Outer vector maps directly to the source vertex ID, 
    // and the inner vector tracks all outbound connections from that specific node.
    std::vector<std::vector<Edge>> adj_;
};

#endif
