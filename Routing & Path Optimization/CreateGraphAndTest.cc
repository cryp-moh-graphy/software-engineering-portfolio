#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "graph.h"

using namespace std;

int graphTestDriver(int argc, char **argv) {
    if (argc != 3) {
        return 0;
    }
    string graph_file = argv[1];
    string query_file = argv[2];

    Graph graph;
    if (!graph.loadGraphFromFile(graph_file)) {
        return 0;
    }

    ifstream qfile(query_file);
    if (!qfile.is_open()) {
        return 0;
    }

    int from, to;
    while (qfile >> from >> to) {
        double weight = 0.0;
      
        if (graph.getEdgeWeight(from, to, weight)) {
            cout << from << " " << to << ": connected " << weight << endl;
        } else {
            cout << from << " " << to << ": not_connected" << endl;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
		    cout << "Usage: " << argv[0] << " <GRAPH_File>" << "<ADJACENCY_QUERYFILE>" << endl;
		    return 0;
    }

    graphTestDriver(argc, argv);

    return 0;
}
