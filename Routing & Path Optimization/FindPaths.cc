#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <limits>

#include "graph.h"

using namespace std;

int pathfindDriver(int argc, char **argv) {
    if (argc != 3) {
        return 0;
    }
    string graph_file = argv[1];
    int start_vertex = stoi(argv[2]); // Parse the command-line flag string into an actual integer node ID.

    Graph graph;
    if (!graph.loadGraphFromFile(graph_file)) {
        return 0;
    }

    vector<double> distances;
    vector<int> path_prefixes;

    graph.dijkstra(start_vertex, distances, path_prefixes);

    // Iterating sequentially from 1 up to N naturally ensures our console 
    // output matches the sorted vertex order required by the project PDF.
    for (int i = 1; i <= graph.getNumVertices(); ++i) {
        // Vertex 1 cannot be reached from vertex 7.
        // Its distance remains at infinity, satisfying this condition.
        // This cleanly triggers the required output: "1: not_possible".
        if (distances[i] == numeric_limits<double>::infinity()) {
            cout << i << ": not_possible" << endl; 
        } else {
            // Suppose our path tracking array contains these parent pointers:
            //   path_prefixes[3] = 5
            //   path_prefixes[5] = 1
            //   path_prefixes[1] = -1
            //
            // Tracing backwards through the loop:
            //   - Starts at curr = 3 -> path vector receives [3]
            //   - Moves to curr = path_prefixes[3] (5) -> path vector receives [3, 5]
            //   - Moves to curr = path_prefixes[5] (1) -> path vector receives [3, 5, 1]
            //   - Moves to curr = path_prefixes[1] (-1) -> loop terminates.
            vector<int> path;
            for (int curr = i; curr != -1; curr = path_prefixes[curr]) {
                path.push_back(curr);
            }
            
            // Because our backtracking sequence stacked elements backwards ([3, 5, 1]),
            // reading from rbegin() to rend() flips the output display into correct 
            // chronological order: "1 5 3".
            cout << i << ":";
            for (auto it = path.rbegin(); it != path.rend(); ++it) {
                cout << " " << *it;
            }
            cout << " cost: " << distances[i] << endl;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
		    cout << "Usage: " << argv[0] << " <GRAPH_FILE>" << "<STARTING_VERTEX>" << endl;
		    return 0;
    }

    pathfindDriver(argc, argv);

    return 0;
}
