#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sys/time.h>
#include "graph.h"
#include "newgraph.h"

using namespace std;

Graph graph;    // 原图
newGraph insGraph;    // 要插入的图
newGraph delGraph;    // 要删除的图

int main(int argc, char *argv[]) {
    if (argc < 6) {
        cout << "Usage: -h(hybrid)/-hp(hybrid parallel) ";
        cout << "graphFile insertedEdgeFile deletedEdgeFile threadNum" << endl;
    }

    string fName = argv[2];    // 图文件
    string insertedEdge_file = argv[3];    // 待插入边文件
    string deletedEdge_file = argv[4];    // 待删除边文件
    string graphFile = fName;
    string insEdgeFile = insertedEdge_file;
    string delEdgeFile = deletedEdge_file;

    ifstream finGraph(graphFile.data());    // 图的输入
    ifstream finInsEdge(insEdgeFile.data());    // 待插入边的输入
    ifstream finDelEdge(delEdgeFile.data());    // 待删除边的输入
    vector<int> allcores;    // 所有节点的核值
    vector<pair<int,int> > allEdges, allInsEdges, allDelEdges;

    // open files, read graph and edge file and compute core numbers
    {
        if (!finGraph) {
            cout << "Error opening" << graphFile << " for input." << endl;
            return 0;
        }
        int a, b;
        // 添加节点和边，构建图
        while (finGraph >> a >> b) {
            if (a == b) continue;
            graph.addEdge(a, b);
            graph.addEdge(b, a);
        }
        // 计算原图中每个节点的核值
        allcores = graph.ComputeCores();
        // 读取待插入边文件
        if (!finInsEdge) {
            cout << "Error opening " << insEdgeFile << " for input." << endl;
            return 0;
        }
        while (finInsEdge >> a >> b) {
            if (a == b) continue;
            allInsEdges.emplace_back(make_pair(a, b));
        }
        // 读取待删除边文件
        if (!finDelEdge) {
            cout << "Error opening " << delEdgeFile << " for input." << endl;
            return 0;
        }
        while (finDelEdge >> a >> b) {
            if (a == b) continue;
            allDelEdges.emplace_back(make_pair(a, b));
        }
    }

    // 混合插入与删除
    {
        
    }

}