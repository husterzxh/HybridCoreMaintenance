#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED
#include<vector>
#include<map>
#include<stack>
#include<iostream>
#include<time.h>
#include<algorithm>
#include<fstream>
#include <sys/time.h>
#include<unordered_map>
using namespace std;


//sort the corepair according to core number by ascent
bool cmp_by_core(const pair<int,int>& v1, const pair<int,int>& v2) {
    if(v1.second == v2.second)
        return v1.first < v2.first;
    else
        return v1.second > v2.second;
}

struct vertex{
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

struct edge{
	int from;
	int to;
	int core;
	edge(int f,int t, int c){
		from = f;
		to = t;
		core = c;
	}
};

class Graph
{

//--------------------------------------------------------------------------
//Variables
//--------------------------------------------------------------------------

/** Number of vertexes and edges */
public: int vertexNum;
        int edgeNum;
		vector<int> allcores;
		vector<int> updatedVertex;
		int visitVerNum;
		int visitEdgeNum;
		int visitandremoved;
		int edgeCount = 0;

vector<vertex> vertices;
/** Edges associated with the vertexes stored at this node
edges[u][i] means the i-th neighbor of vertex u*/
vector<vector<int> > edges;

//--------------------------------------------------------------------------
//Constructor
//--------------------------------------------------------------------------
Graph()
{
	vertexNum = 0;
	edgeNum = 0;
	visitVerNum = 0;
	visitEdgeNum=0;
	visitandremoved = 0;
}

Graph(int vNum, int eNum){
    vertexNum=vNum;
    edgeNum=eNum;
    for(int i=0;i<vertexNum;i++){
        vertex temp=vertex();
        vertices.push_back(temp);
        edges.push_back(vector<int>(0));
    }
}

~Graph(){
	
}

/**
 * Adds an edge to the graph.
 *
 * @param from
 *          the starting node of an edge
 * @param to
 *          the final node of an edge
 */

 bool addEdge(int from, int to)
{
    if(from == to) {
		// cout << from << " " << to << endl;
		return false;
	}
	// Update size,add another  vertex
	while(from+1 > vertexNum) {
		vertexNum ++;
		vertex temp=vertex();
        vertices.push_back(temp);
        edges.push_back(vector<int>(0));
    }
        // Search if the edge is already present; if so, stop here and return
	for (int i = 0; i < edges[from].size(); i++) {
		if (edges[from][i] == to) {
			// cout << from << " " << to << endl;
			return false;
		}		
	}
	// Finally store the edge
	edges[from].push_back(to);
	edgeNum++;
	return true;
}


 bool deleteEdge(int from, int to){
    vector <int>::iterator Iter;
    for(Iter=edges[from].begin();Iter!=edges[from].end();Iter++){
      if (*Iter== to){
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

void deleteEdgeSet(unordered_map<int, vector<pair<int,int> > > deletedEdges){
	unordered_map<int, vector<pair<int,int> > >::iterator iter;
	for(iter = deletedEdges.begin();iter != deletedEdges.end();++iter){
		vector<pair<int,int> > edges = iter->second;
		for(int j=0;j<edges.size();j++){
			int u = edges[j].first;
			int v = edges[j].second;
			deleteEdge(u,v);
			deleteEdge(v,u);
		}				
	}
}

void insertEdgeSet(unordered_map<int, vector<pair<int,int> > > deletedEdges){
	unordered_map<int, vector<pair<int,int> > >::iterator iter;
	for(iter = deletedEdges.begin();iter != deletedEdges.end();++iter){
		vector<pair<int,int> > edges = iter->second;
		for(int j=0;j<edges.size();j++){
			int u = edges[j].first;
			int v = edges[j].second;
			addEdge(u,v);
			addEdge(v,u);
		}			
	}
}

/**
 * Returns the number of nodes in the graph.
 */
 int GetvertexNum()
{
	return vertexNum;
}

 int GetedgeNum()
{
	return edgeNum;
}
/**
*compute core number for the graph, static method
*/
vector<int> ComputeCores(){
    int a,b;
    vector<int> bin,pos,vert,deg;
    a = b = 0;
    int n = vertexNum;
    int maxdegree = 0;
    for(int u = 0;u < n;u++){
        pos.push_back(0);
        vert.push_back(0);
        int usize = edges[u].size();
        deg.push_back(usize);
        if(usize > maxdegree){
            maxdegree = usize;
        }
    }
    //cout<<"maxdegree: "<<maxdegree<<endl;
    for(int k = 0;k <= maxdegree;k ++){
        bin.push_back(0);
    }
    for(int u = 0;u < n;u ++){
        bin[deg[u]]++;
    }
    int start  = 0;
    for(int k = 0;k <= maxdegree;k ++){
        int num = bin[k];
        bin[k] = start;
        start += num;
    }
    for(int u = 0;u < n ;u ++){
        pos[u] = bin[deg[u]];
        vert[pos[u]] = u;
        bin[deg[u]]++;
    }
    for(int d = maxdegree;d>0;d--){
        bin[d]=bin[d-1];
    }
    bin[0] = 0;
    int du,pu,pw,w;
    for(int i = 0;i < n;i++){
        int v = vert[i];
        for(int j=0;j<edges[v].size();j++){
            int u = edges[v][j];
            if(deg[u] > deg[v]){
                du = deg[u];
                pu = pos[u];
                pw = bin[du];
                w = vert[pw];
                if(u != w){
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
    for(int i=0;i < n;i++){
        vertices[i].core = deg[i];
    }
	allcores = deg;
	return deg;
}

bool CheckCores(){
	vector<int> cores(vertexNum,0);
	for(int v=0;v<vertexNum;v++){
		cores[v] = vertices[v].core;
	}
	if(cores != ComputeCores()){
		cout<<"check failed!"<<endl;
		int a;
		cin>>a;
		return false;
	}
	return true;
}

vector<int> GetAllcores(){
	return allcores;
}

// --------------------------------------------------------------------------
// main Methods for centralized algorithm
// --------------------------------------------------------------------------

/**
*delete all selected edges and conduct the centralized algorithm
*/
void Deletion(vector<pair<int,int> > allNewEdges, map<int, vector<int> > &orderEdges)
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
			TravelDelete(a,b,orderEdges);
			for(int i=0;i<vertexNum;i++){						
				if((vertices[i].visited)&&(vertices[i].removed)){
					// vertices[i].core--;
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
	cout << "numbers of visited edges: " << visitEdgeNum << endl;
	resetRcds();
}

/**
*insert all selected edges and conduct the centralized algorithm
*/
void Insertion(vector<pair<int,int> > allNewEdges)
{
	computeMcd();
	computePcd();
	visitEdgeNum=0;
	visitVerNum = 0;
	for(int k=0;k < allNewEdges.size();k ++){
		int a = allNewEdges[k].first;
		int b = allNewEdges[k].second;
		if(addEdge(a,b) && addEdge(b,a)){
			//computeMcd();
			//computePcd();
			prepareMcd(a,b);
			preparePcd(a,b);									
			TravelInsert(a,b);
			//update core numbers
			for(int i=0;i<vertexNum;i++){						
				if((vertices[i].visited)&&(!vertices[i].removed)){
					vertices[i].core++;
					//cout<<i<<endl;
					updatedVertex.push_back(i);
				}
				if(vertices[i].visited){
					visitVerNum++;
				}
				if(vertices[i].removed){
					visitVerNum++;
				}
			}
			//recompute MCD for u and its 1-hop neighbor
			for(int i=0;i<updatedVertex.size();i++){
				int u = updatedVertex[i];
				computeMcd(u);
				for(int j=0;j<edges[u].size();j++)
				{
					int w=edges[u][j];
					computeMcd(w);
				}								
			}
			//recompute MCD for u and its 1,2-hop neighbor
			for(int i=0;i<updatedVertex.size();i++){
				int u = updatedVertex[i];						
				//if((vertices[u].visited)&&(!vertices[u].removed)){
					computePcd(u);//for u
					for(int j=0;j<edges[u].size();j++)
					{
						int w=edges[u][j];
						computePcd(w);//1-hop neighbor
						for(int p=0;p<edges[w].size();p++)
						{
							int v=edges[w][p];
							computePcd(v);//2-hop neighbor					
						}
					}				
				//}
			}
			resetVertex();
			//cout<<updatedVertex.size()<<" vertex increase cores"<<endl;
			updatedVertex.clear();
			//CheckCores();
		}
	}
}

//deal with one edge insertion 
void TravelInsert(int u1, int u2)
{
    int r=u1;
    int coreu1=vertices[u1].core;
    int coreu2=vertices[u2].core;
    if(coreu1 > coreu2){
        r=u2;
    }
    stack<int> s;
    s.push(r);
    int K=vertices[r].core;
	//cout<<"K = "<<K<<endl;
    vertices[r].visited=true;
    vertices[r].cd=vertices[r].pcd;
    int cdr = vertices[r].cd;
    while(!s.empty()){
        int v=s.top();
        s.pop();
        int cdv = vertices[v].cd;
        if(cdv > K){
            for(int j=0;j<edges[v].size();j++){
                int w = edges[v][j];
                int corew = vertices[w].core;
                int mcdw = vertices[w].mcd;
                if(corew == K && mcdw > K &&(!vertices[w].visited)){
                    s.push(w);
                    vertices[w].visited = true;
                    vertices[w].cd += vertices[w].pcd;
					//visitEdgeNum++;
                }
            }
			//visitEdgeNum+=edges[v].size();
        }
        else{
            if(!vertices[v].removed){
                InsertRemovement(K,v);
            }
        }
    }
}

void InsertRemovement(int k, int v)
{
    vertices[v].removed=true;
    for(int i=0;i<edges[v].size();i++){
        int w=edges[v][i];
        int corew = vertices[w].core;
        if(corew == k){
			//visitVerNum++;
            vertices[w].cd--;
            int cdw = vertices[w].cd;
			if(cdw == k && !vertices[w].removed){
                InsertRemovement(k,w);
            }
		}
    }
	visitEdgeNum+=edges[v].size();
}

//deal with one edge deletion
void TravelDelete(int u1, int u2, map<int, vector<int> > &orderEdges)
{
    int r = u1;
    int coreu1 = vertices[u1].core;
    int coreu2 = vertices[u2].core;
    int k = coreu1;
    if(coreu1 > coreu2){ 
		r=u2;
        k=coreu2;
    }
    if(coreu1 != coreu2){
		//visitVerNum++;
        vertices[r].visited = true;
        vertices[r].cd = vertices[r].mcd;
        int cdr = vertices[r].cd;
		if(cdr < k){
            DeleteRemove(k,r,orderEdges);
        }
    }
    else{
       vertices[u1].visited = true;
       vertices[u1].cd = vertices[u1].mcd;
       int cdu1 = vertices[u1].cd;
       if(cdu1 < k){
            DeleteRemove(k,u1,orderEdges);
       }
       vertices[u2].visited = true;
       vertices[u2].cd = vertices[u2].mcd;
       int cdu2 = vertices[u2].cd;
       if(!vertices[u2].removed && cdu2 < k ){
            DeleteRemove(k,u2,orderEdges);
       }
	   //visitVerNum+=2;
    }
}

void DeleteRemove(int k, int v, map<int, vector<int> > &orderEdges)
{
	int flag = 1;
	if (orderEdges.find(v) != orderEdges.end()) {
		vector<int> neighbors = orderEdges[v];
		for (auto it = neighbors.begin(); it != neighbors.end(); it++) {
			visitEdgeNum++;
			if (vertices[*it].core > k) {
				flag = 0;
				if (!addEdge(v, *it) || !addEdge(*it, v)) {
					cout << "add edge failed when delete edges " << endl;
				}
				vector<int> neighbors2 = orderEdges[*it];
				for (auto it2 = neighbors2.begin(); it2 != neighbors2.end(); it2++) {
					if (*it2 == v) {
						neighbors2.erase(it2);
						orderEdges[*it] = neighbors2;
						break;
					}
				}
				vertices[v].cd++;
				neighbors.erase(it);
				orderEdges[v] = neighbors;
				edgeCount++;
				break;
			}
			else if (vertices[*it].core == k) {
				flag = 0;
				if (!addEdge(v, *it) || !addEdge(*it, v)) {
					cout << "add edge failed when delete edges " << endl;
				}
				vector<int> neighbors2 = orderEdges[*it];
				for (auto it2 = neighbors2.begin(); it2 != neighbors2.end(); it2++) {
					if (*it2 == v) {
						neighbors2.erase(it2);
						orderEdges[*it] = neighbors2;
						break;
					}
				}
				vertices[v].cd++;
				vertices[*it].cd++;
				neighbors.erase(it);
				orderEdges[v] = neighbors;
				edgeCount++;
				break;
			}
			else {
				;
			}
		}
	}

	if (flag == 1) {
		vertices[v].removed = true;
		vertices[v].core--;
		for(int i=0;i< edges[v].size();i++){
			int w= edges[v][i];
			int corew = vertices[w].core;
			if(corew == k){
				//visitVerNum++;
				if(!vertices[w].visited){
					vertices[w].cd += vertices[w].mcd;
					vertices[w].visited = true;
				}
				vertices[w].cd--;
				int cdw = vertices[w].cd;
				if(cdw < k && ! vertices[w].removed){
					DeleteRemove(k,w,orderEdges);
				}
			}
		}
		visitEdgeNum+=edges[v].size();
	}

}

/**
prepareMcd for an edge <u,v>
**/
void prepareMcd(int u,int v){
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

/**
*compute MCD for all vertices
*/

void computeMcd(){
    for(int v=0;v<vertexNum;v++){
        for(int j=0;j<edges[v].size();j++)
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


// --------------------------------------------------------------------------
// Methods for parallel algorithm
// --------------------------------------------------------------------------

/**
*decrease core numbers for vertices that are visited and removed
*/
void delCores(){
	for(int i=0;i<vertexNum;i++){
		if((vertices[i].visited)&&(vertices[i].removed)){
			vertices[i].core--;
			allcores[i]= vertices[i].core;
			//cout<<i<<" -- core"<<endl;
		}
		if(vertices[i].visited){
			visitVerNum++;
		}
        if(vertices[i].removed){
			visitVerNum++;
		}/**/
		vertices[i].visited=false;
        vertices[i].removed=false;
        vertices[i].mcd=0;
        vertices[i].pcd=0;
        vertices[i].cd=0;
	}
}
/**
*increase core numbers for vertices that are visited but not removed
*/
void insCores(){
	for(int i=0;i<vertexNum;i++){
		if((vertices[i].visited)&&(!vertices[i].removed)){
			vertices[i].core++;
			allcores[i]= vertices[i].core;
			//cout<<i<<" ++ core"<<endl;
		}
		if(vertices[i].visited){
			visitVerNum++;
		}
        if(vertices[i].removed){
			visitVerNum++;
		}/**/
		vertices[i].visited=false;
        vertices[i].removed=false;
        vertices[i].mcd=0;
        vertices[i].pcd=0;
        vertices[i].cd=0;
	}
}

/**
*given a superior edge set, compute the MCD for vertices in the EXPTree
*/
void computeInsertMcd(vector<pair<int,int> > superioredges){
	int edgenums = superioredges.size();
    map<int,bool> visited;	
    for(int i = 0;i < edgenums;i++){
        stack<int> s;
        int a = superioredges[i].first;
        int b = superioredges[i].second;
        int sizea = edges[a].size();
        int sizeb = edges[b].size();
        int corea = vertices[a].core;
        int coreb = vertices[b].core;
        int r = a;
        int corer = corea;
        int sizer = sizea;
        if(corea > coreb){
            r = b;
            corer = coreb;
            sizer = sizeb;
        }
		if(!visited[r]){
			s.push(r);
			//vertices[r].visited = true;
			visited[r] = true;
			for(int j=0;j<sizer;j++)
			{
				int w = edges[r][j];
				int corew = vertices[w].core;
				if(corew >= corer){
					vertices[r].mcd++;
				}
			}
		   while(!s.empty()){
				int v=s.top();
				s.pop();
				int sizev = edges[v].size();
				int corev = vertices[v].core;
				for(int j=0;j<sizev;j++){
					int p = edges[v][j];
					if(vertices[p].core == corev && !visited[p]){
						s.push(p);
						//vertices[p].visited = true;
						visited[p] = true;
						if(!vertices[p].mcd){
							int sizep = edges[p].size();
							int corep = vertices[p].core;
							for(int k = 0;k < sizep;k ++){
								int w = edges[p][k];
								int corew = vertices[w].core;
								if(corew >= corep){
									vertices[p].mcd++;
								}
							}
						}
					}
				}
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

 void resetVertex(){
    for(int v = 0;v < vertexNum;v++){
		vertices[v].visited=false;
        vertices[v].removed=false;
        vertices[v].cd=0;
    }
}
 
void resetRcds(){
    for(int v = 0;v < vertexNum;v++){
        vertices[v].mcd=0;
        vertices[v].pcd=0;
		vertices[v].cd=0;        
    }
}

void WriteCores(string corefile){
	ofstream fcore(corefile.data());
	vector<pair<int,int> > corepair;
	for(int i=0;i<vertexNum;i++){
		corepair.push_back(make_pair(i,vertices[i].core));
	}
	sort(corepair.begin(),corepair.end(),cmp_by_core);
	for(int i=0;i<corepair.size();i++){
		fcore<<corepair[i].first<<","<<corepair[i].second<<endl;
	}
	fcore.close();
}

};


#endif // GRAPH_H_INCLUDED
