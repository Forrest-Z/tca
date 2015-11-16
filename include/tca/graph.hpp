
#ifndef GRAPH_H
#define GRAPH_H

#include <functional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;

class Index {

    public:
        unsigned int i, j;

        Index() {}
        ~Index() {}

        Index(int i, int j) : i(i), j(j) {}
};

class IndexHash {

    public:
        size_t operator() (const Index& index) const {
            stringstream buffer;
            buffer << index.i << " " << index.j;
            return hash<string>()(buffer.str());
        }
};

class Edge {

    public:
        Index a, b;
        Edge() {}
        ~Edge() {}
        Edge(Index a, Index b) : a(a), b(b) {}
};

class EdgeHash {

    public:
        size_t operator() (const Edge& edge) const {
            stringstream buffer;
            buffer << edge.a.i << " " << edge.a.j << " ";
            buffer << edge.b.i << " " << edge.b.j << " ";
            return hash<string>()(buffer.str());
        }
};

typedef vector<Index> IndexPath;

class EdgeData {

    public:
        vector<IndexPath> paths;
        vector<double> dists;

        EdgeData() {};
        ~EdgeData() {};
        EdgeData(vector<IndexPath> paths, vector<double> dists) : paths(paths),
            dists(dists) {}

        void add_path(vector<Index> path, double dist) {
            this->paths.push_back(path);
            this->dists.push_back(dist);
        }
};

inline bool operator== (Index const& lhs, Index const& rhs) {
    return (lhs.i == rhs.i) && (lhs.j == rhs.j);
}

inline bool operator== (Edge const& lhs, Edge const& rhs) {
    return (lhs.a == rhs.a) && (lhs.b == rhs.b);
}

class Graph {

    private:
        unordered_map<Index, unordered_set<Index, IndexHash>, IndexHash> nodes;
        unordered_map<Edge, EdgeData, EdgeHash> edges;

    public:

        Graph() {}
        ~Graph() {}

        void add_edge(Index a, Index b, EdgeData ed) {
            if (nodes.count(a) == 0) {
                nodes[a] = unordered_set<Index, IndexHash>();
                nodes[b] = unordered_set<Index, IndexHash>();
            }

            nodes[a].insert(b);
            nodes[b].insert(a);
            edges[Edge(a, b)] = ed;
            edges[Edge(b, a)] = ed;
        }

        void add_edge(Edge edge, EdgeData ed) {
            this->add_edge(edge.a, edge.b, ed);
        }

        EdgeData *get_edge(Edge edge) {
            if (edges.count(edge) > 0) {
                return &edges[edge];
            } else {
                return NULL;
            }
        }

        EdgeData *get_edge(Index a, Index b) {
            return this->get_edge(Edge(a, b));
        }
};

#endif
