
#include <vector>
#include <unordered_set>
#include <queue>
#include <iostream>
#include <omp.h>
#include "dynamicvoronoi.h"
#include "tca/graph.hpp"
#include "tca/voronoi.hpp"

using namespace std;

#define NUM_NBRS 4
#define NUM_THREADS 2

inline bool is_node(DynamicVoronoi& dv, Index& ind)
{
    int i = ind.i, j = ind.j;
    int nn = dv.getNumVoronoiNeighborsAlternative(i, j);
    bool is_alt = dv.isVoronoiAlternative(i, j);
    return is_alt && nn >= 3;

}

void determine_nodes(DynamicVoronoi& dv, vector<Index>& nodes,
        unordered_set<Index, IndexHash>& node_set)
{
    int nn;
    bool is_alt;

    for (int i = 0; i < dv.getSizeX(); i++)
    {
        for (int j = 0; j < dv.getSizeY(); j++)
        {
            nn = dv.getNumVoronoiNeighborsAlternative(i, j);
            is_alt = dv.isVoronoiAlternative(i, j);
            if (is_alt && nn >= 3)
            {
                nodes.push_back(Index(i, j));
                node_set.insert(Index(i, j));
            }
        }
    }
}

bool neighbourhood(Index ind, DynamicVoronoi& dv, bool nbrs[4])
{
    int i = ind.i;
    int j = ind.j;

    for (int k = 0; k < NUM_NBRS; k++)
    {
        nbrs[k] = false;
    }

    if (dv.isVoronoiAlternative(i, j))
    {

        if (j > 0 and dv.isVoronoiAlternative(i, j - 1)) {
            nbrs[0] = true;
        }

        if (i < dv.getSizeX() - 1 and dv.isVoronoiAlternative(i + 1, j))
        {
            nbrs[1] = true;
        }

        if (j < dv.getSizeY() - 1 and dv.isVoronoiAlternative(i, j + 1))
        {
            nbrs[2] = true;
        }

        if (i > 0 and dv.isVoronoiAlternative(i - 1, j))
        {
            nbrs[3] = true;
        }
        return true;
    }

    return false;
}

inline Index index_lookup(Index a, int i)
{
    switch (i)
    {
        case 0:
            return Index(a.i, a.j - 1);
        case 1:
            return Index(a.i + 1, a.j);
        case 2:
            return Index(a.i, a.j + 1);
        case 3:
            return Index(a.i - 1, a.j);
        default:
            return a;
    }
}

Index next_in_path(Index cur, Index prev, DynamicVoronoi& dv)
{
    bool nbrs[NUM_NBRS];
    neighbourhood(cur, dv, nbrs);
    for (int i = 0; i < NUM_NBRS; i++)
    {
        if (nbrs[i])
        {
            Index maybe_next = index_lookup(cur, i);
            if (not (prev == maybe_next))
            {
                return maybe_next;
            }
        }
    }
}

void find_enclosing_nodes(Index& ind, DynamicVoronoi& dv,
        vector<Index>& nodes)
{
    unordered_set<Index, IndexHash> seen;
    queue<Index> q;
    q.push(ind);
    Index w, e, n, s;
    while (not q.empty())
    {
        Index e = q.front(), w = q.front();
        e.i--;
        q.pop();
        while (seen.count(w) == 0 and !dv.isVoronoiAlternative(w.i, w.j))
        {
            n = Index(w.i, w.j + 1);
            s = Index(w.i, w.j - 1);
            if(seen.count(n) == 0 and !dv.isVoronoiAlternative(n.i, n.j))
            {
                q.push(n);
            }
            else
            {
                if (is_node(dv, n))
                {
                    nodes.push_back(n);
                }
            }
            if(seen.count(s) == 0 and !dv.isVoronoiAlternative(s.i, s.j))
            {
                q.push(s);
            }
            else
            {
                if (is_node(dv, s))
                {
                    nodes.push_back(s);
                }
            }

            seen.insert(w);
            w.i++;
        }
        while (seen.count(e) == 0 and !dv.isVoronoiAlternative(e.i, e.j))
        {
            n = Index(e.i, e.j + 1);
            s = Index(e.i, e.j - 1);
            if(seen.count(n) == 0 and !dv.isVoronoiAlternative(n.i, n.j))
            {
                q.push(n);
            }
            else
            {
                if (is_node(dv, n))
                {
                    nodes.push_back(n);
                }
            }
            if(seen.count(s) == 0 and !dv.isVoronoiAlternative(s.i, s.j))
            {
                q.push(s);
            }
            else
            {
                if (is_node(dv, s))
                {
                    nodes.push_back(s);
                }
            }
            seen.insert(e);
            e.i--;
        }
        if (is_node(dv, w))
        {
            nodes.push_back(w);
        }
        if (is_node(dv, e))
        {
            nodes.push_back(e);
        }
    }
}

void connect_start_and_goal(Index& start, Index& goal, DynamicVoronoi& dv,
        Graph& g)
{

}

void generate_graph(Index& start, Index& goal, DynamicVoronoi& dv, Graph& G)
{
    dv.occupyCell(start.i, start.j);
    dv.occupyCell(goal.i, goal.j);
    dv.update();
    dv.updateAlternativePrunedDiagram();
    vector<Index> ns;
    find_enclosing_nodes(goal, dv, ns);
    cout << ns.size() << endl;
    bool nbrs[NUM_NBRS];
    vector<Index> nodes;
    unordered_set<Index, IndexHash> node_set;
    determine_nodes(dv, nodes, node_set);

    for (int i = 0; i < nodes.size(); i++)
    {
        unordered_set<Index, IndexHash> seen;
        neighbourhood(nodes[i], dv, nbrs);

        for (int j = 0; j < NUM_NBRS; j++)
        {
            if (nbrs[j])
            {
                vector<Index> path;
                Index prev = nodes[i];
                Index cur = index_lookup(prev, j);
                while (true)
                {
                    if (node_set.count(cur) > 0)
                    {
                        G.add_edge(nodes[i], cur, path, path.size());
                        break;
                    }
                    path.push_back(cur);
                    Index nxt = next_in_path(cur, prev, dv);
                    prev = cur;
                    cur = nxt;
                }
            }
        }
    }
}
