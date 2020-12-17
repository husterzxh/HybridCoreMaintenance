#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED
#include <vector>
#include <map>
#include <stack>
#include <iostream>
#include <time.h>
#include <algorithm>
#include <fstream>
#include <sys/time.h>
#include <unordered_map>
using namespace std;

struct vertex {
    bool visited;
    bool removed;
    int core;
    int mcd;
    int pcd;
    int cd;
    vertex(){
        visited=removed=false;
        core=mcd=pcd=cd=0;
    }
};

struct edge {
	int from;
	int to;
	int core;
	edge(int f,int t, int c){
		from = f;
		to = t;
		core = c;
	}
};

class Graph {
public:
    int vertexNum;
    int edgeNum;
    vector<int> allcores;    // 所有节点核值
    vector<int> updatedVertex;
    int visitVerNum;
    int visitEdgeNum;
    int visitandremoved;
    vector<vertex> vertices;
    /** Edges associated with the vertices stored at this node
    edges[u][i] means the i-th neighbor of vertex u*/
    vector<vector<int> > edges;

    Graph() {
        vertexNum = 0;
        edgeNum = 0;
        visitVerNum = 0;
        visitEdgeNum=0;
        visitandremoved = 0;
    }

    Graph(int vNum, int eNum) {
        vertexNum=vNum;
        edgeNum=eNum;
        for(int i=0; i<vertexNum; i++) {
            vertex temp=vertex();
            vertices.push_back(temp);
            edges.push_back(vector<int>(0));
        }
    }
};

#endif // GRAPH_H_INCLUDED