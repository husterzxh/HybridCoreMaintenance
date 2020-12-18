#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sys/time.h>

using namespace std;

Graph graph;

int main(int argc, char *argv[]) {
    if (argc < 6) {
        cout << "Usage: -h(hybrid)/-hp(hybrid parallel) ";
        cout << "graphFile insertedEdgeFile deletedEdgeFile threadNum" << endl;
    }

    string fName = argv[2];    // 图文件
    string insertedEdge_file = argv[3];    // 插入边文件
    string deletedEdge_file = argv[4];    // 删除边文件
    string graphFile = fName;
    string insEdgeFile = insertedEdge_file;
    string delEdgeFile = deletedEdge_file;

    ifstream finGraph(graphFile.data());    // 图的输入
    ifstream finInsEdge(insEdgeFile.data());    // 插入边的输入
    ifstream finDelEdge(delEdgeFile.data());    // 删除边的输入

    // open files, read graph and edge file and compute core numbers
    {
        if (!finGraph) {
            cout << "Error opening" << graphFile << " for input." << endl;
            return 0;
        }
        int a, b;
        while (finGraph >> a >> b) {
            if (a == b) continue;
            graph.addEdge(a, b);
            graph.addEdge(b, a);
        }
    }

}