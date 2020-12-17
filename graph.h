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
	edge(int f,int t, int c) {
		from = f;
		to = t;
		core = c;
	}
};

class Graph {
public:
    int vertexNum;    // 节点个数
    int edgeNum;    // 边个数
    vector<int> allcores;    // 所有节点核值
    vector<int> updatedVertex;
    int visitVerNum;
    int visitEdgeNum;
    int visitandremoved;
    vector<vertex> vertices;    // 节点
    /** Edges associated with the vertices stored at this node
    edges[u][i] means the i-th neighbor of vertex u*/
    vector<vector<int> > edges;    // 边

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

    ~Graph() {

    }

    /** Returns the number of nodes in the graph.
     */
    int getVertexNum()
    {
        return vertexNum;
    }

    /** Returns the number of edges in the graph.
     */
    int getEdgeNum()
    {
        return edgeNum;
    }

    /** Returns the core numbers of nodes in the graph.
     */
    vector<int> GetAllcores() {
        return allcores;
    }

    /** Add an edge into the graph.
     * @param from: the starting node of the edge
     * @param to: the ending node of the edge
     * @return: true if succeed otherwise false
     */
    bool addEdge(int from, int to) {
        if (from == to) {
            return false;
        }
        // Add a new vertex
        while (from + 1 > vertexNum) {
            vertexNum++;
            vertex temp = vertex();
            vertices.push_back(temp);
            edges.push_back(vector<int>(0));
        }
        // Search if the edge is already present; if so, stop here and return
        for (int i = 0; i < edges[from].size(); i++) {
            if (edges[from][to] == to) {
                return false;
            }
        }
        // store the edge
        edges[from].push_back(to);
        edgeNum++;
        return true;
    }

    /** Add edges sets
     * @param insertedEdges: 待添加的边集
     */
    void insertEdgeSet(unordered_map<int, vector<pair<int,int> > > insertedEdges) {
        unordered_map<int, vector<pair<int,int> > >::iterator iter;
        for (iter = insertedEdges.begin(); iter != insertedEdges.end(); ++iter) {
            vector<pair<int,int> > edges = iter->second;
            for (int j=0; j<edges.size(); j++) {
                int u = edges[j].first;
                int v = edges[j].second;
                addEdge(u,v);
                addEdge(v,u);
            }
        }
    }

    /** Delete an edge from the graph
     * @param from: the starting node of the edge
     * @param to: the ending node of the edge
     * @return: true if succeed otherwise false
     */ 
    bool deleteEdge(int from, int to) {
        vector <int>::iterator Iter;
        for (Iter=edges[from].begin(); Iter!=edges[from].end(); Iter++) {
            if (*Iter == to) {
                edges[from].erase(Iter);
                edgeNum--;
                Iter = edges[from].begin();
                return true;
            }
        }
        //if there is no edge <from, to>
        if(Iter == edges[from].end()){
            return false;
        }
    }

    /** Delete edges sets
     * @param deletedEdges: 待删除的边集
     */
    void deleteEdgeSet(unordered_map<int, vector<pair<int,int> > > deletedEdges) {
        unordered_map<int, vector<pair<int,int> > >::iterator iter;
        for (iter = deletedEdges.begin(); iter != deletedEdges.end(); ++iter) {
            vector<pair<int,int> > edges = iter->second;
            for (int j=0; j<edges.size(); j++) {
                int u = edges[j].first;
                int v = edges[j].second;
                deleteEdge(u,v);
                deleteEdge(v,u);
            }
        }
    }

    /** compute core number for the graph, static method
     * 同时每个节点struct的core也更新为节点对应的核值
     * @return: vector<int>对应每个节点的核值
    */
    vector<int> ComputeCores() {
        int a, b;
        vector<int> bin, pos, vert, deg;
        a = b = 0;
        int n = vertexNum;
        int maxdegree = 0;
        for (int u = 0; u < n; u++) {
            pos.push_back(0);
            vert.push_back(0);
            int usize = edges[u].size();
            deg.push_back(usize);
            if(usize > maxdegree) {
                maxdegree = usize;
            }
        }
        //cout<<"maxdegree: "<<maxdegree<<endl;
        for (int k = 0; k <= maxdegree; k++) {
            bin.push_back(0);
        }
        for (int u = 0; u < n; u++) {
            bin[deg[u]]++;
        }
        int start = 0;
        for (int k = 0; k <= maxdegree; k++) {
            int num = bin[k];
            bin[k] = start;
            start += num;
        }
        for (int u = 0; u < n; u ++) {
            pos[u] = bin[deg[u]];
            vert[pos[u]] = u;
            bin[deg[u]]++;
        }
        for (int d = maxdegree; d>0; d--) {
            bin[d] = bin[d-1];
        }
        bin[0] = 0;
        int du,pu,pw,w;
        for (int i = 0; i < n; i++) {
            int v = vert[i];
            for (int j=0; j<edges[v].size(); j++) {
                int u = edges[v][j];
                if (deg[u] > deg[v]) {
                    du = deg[u];
                    pu = pos[u];
                    pw = bin[du];
                    w = vert[pw];
                    if (u != w) {
                        pos[u] = pw;
                        vert[pu] = w;
                        pos[w] = pu;
                        vert[pw] = u;
                    }
                    bin[du]++;
                    deg[u]--;
                }
            }
        }
        for (int i = 0; i < n; i++) {
            vertices[i].core = deg[i];
        }
        allcores = deg;
        return deg;
    }

    /** 检查struct vertex core变量存放的是否是正确的核值
     * @return: 如果检查通过返回true，否则返回false
    */
    bool CheckCores() {
        vector<int> cores(vertexNum,0);
        for (int v=0; v<vertexNum; v++) {
            cores[v] = vertices[v].core;
        }
        if (cores != ComputeCores()) {
            cout << "check failed!" << endl;
            // 暂停作用，相当于system("pause")
            int a;
            cin >> a;
            return false;
        }
        return true;
    }

// --------------------------------------------------------------------------
// main Methods for centralized algorithm(TRAVERSAL)
// --------------------------------------------------------------------------

    /* compute MCD for all vertices
    */
    void computeMcd() {
        for (int v=0; v<vertexNum; v++) {
            for (int j=0; j<edges[v].size(); j++)
            {
                int w=edges[v][j];
                int corev=vertices[v].core;
                int corew=vertices[w].core;
                if(corew >= corev){
                    vertices[v].mcd++;
                }
            }
        }
    }

    /**
    *compute PCD for all vertices
    */
    void computePcd(){
        for(int v = 0;v < vertexNum;v++){
            for(int j=0;j<edges[v].size();j++)
            {
                int w=edges[v][j];
                int corev=vertices[v].core;
                int corew=vertices[w].core;
                int mcdw=vertices[w].mcd;
                if(corew > corev ||
                (corew == corev && mcdw> corev)){
                        vertices[v].pcd++;
                }
            }
        }
    }

    /**
    *compute MCD for a vertex v
    */
    int computeMcd(int v){
        vertices[v].mcd= 0;
        int corev = vertices[v].core;
        int sizev = edges[v].size();
        for(int j=0;j<sizev;j++)
        {
            int w = edges[v][j];
            int corew = vertices[w].core;
            if(corew >= corev){
                vertices[v].mcd++;
            }
        }
        return sizev;
    }

    /**
    *compute PCD for a vertex v, all MCDs are already known
    */
    int computePcd(int v){
        int visitNum = 0;
        vertices[v].pcd = 0;
        int corev = vertices[v].core;
        int sizev = edges[v].size();
        for(int j=0;j< sizev;j++){
            int w = edges[v][j];
            int corew = vertices[w].core;
            if(vertices[w].mcd == 0){
                visitNum += computeMcd(w);
            }
            int mcdw = vertices[w].mcd;
            if(corew > corev || (corew == corev && mcdw > corev)){
                vertices[v].pcd++;
            }
        }
        return visitNum;
    }

    /** prepareMcd for an edge <u,v>
    **/
    void prepareMcd(int u, int v) {
        computeMcd(v);
        computeMcd(u);
        for(int j=0;j<edges[u].size();j++)
        {
            int w=edges[u][j];
            computeMcd(w);
        }	
        for(int j=0;j<edges[v].size();j++)
        {
            int w=edges[v][j];
            computeMcd(w);
        }
    }

    /**
    preparePcd for an edge <u,v>
    **/
    void preparePcd(int u,int v){
        computePcd(v);
        computePcd(u);
        for(int j=0;j<edges[u].size();j++)
        {
            int w=edges[u][j];
            computePcd(w);
        }	
        for(int j=0;j<edges[v].size();j++)
        {
            int w=edges[v][j];
            computePcd(w);
        }
    }

    /** delete all selected edges and conduct the centralized algorithm
    */
    void Deletion(vector<pair<int,int> > allNewEdges)
    {//delete edges
        computeMcd();
        computePcd();
        visitEdgeNum=0;
        visitandremoved = 0;
        for(int k=0;k < allNewEdges.size();k ++){
            int a = allNewEdges[k].first;
            int b = allNewEdges[k].second;
            if(deleteEdge(a,b) && deleteEdge(b,a)){
                prepareMcd(a,b);
                TravelDelete(a,b);
                for(int i=0;i<vertexNum;i++){						
                    if((vertices[i].visited)&&(vertices[i].removed)){
                        vertices[i].core--;
                        updatedVertex.push_back(i);
                        computeMcd(i);
                        for(int j=0;j<edges[i].size();j++)
                        {
                            int w=edges[i][j];
                            computeMcd(w);
                        }
                    }
                    if(vertices[i].visited){
                        visitVerNum++;
                    }
                    if(vertices[i].removed){
                        visitVerNum++;
                    }
                }
                resetVertex();
                //cout<<updatedVertex.size()<<" vertex decrease cores"<<endl;
                updatedVertex.clear();
                //CheckCores();
            }
        }
        resetRcds();
    }


};

#endif // GRAPH_H_INCLUDED