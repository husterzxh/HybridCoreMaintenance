
#ifndef NEWGRAPH_H_INCLUDED
#define NEWGRAPH_H_INCLUDED
#include<vector>
#include<map>
#include<iostream>
#include<stack>
#include<algorithm>
#include<pthread.h>
#include<time.h>
#include<unordered_map>
#include"graph.h"
using namespace std;

typedef vector<int> VECINT;

struct params{
    int k;
    int i;
    params(){
        k = i = 0;
    }
    params(int K,int index){
        k = K;
        i = index;
    }
};

struct newvertex{
    int id;
    int core;
    //adjacent list of this vertex, store the neighbor`s index
    vector<int> adjacent;
	VECINT colors;
    newvertex(){
        id = core = 0;
        adjacent.resize(0);
		colors.resize(0);
    }

};


class newGraph
{

/** Number of vertexes and edges */
public: int vertexNum;
        int edgeNum;
		int max_color,delta;
        map<int,int> index_map;
        vector<newvertex> vertices;
		
//--------------------------------------------------------------------------
//Constructor
//--------------------------------------------------------------------------
public: newGraph()
{
	vertexNum = edgeNum = max_color = 0;
}

public: void init(int vNum, int eNum){
    vertexNum=vNum;
    edgeNum=eNum;
    for(int i=0;i<vertexNum;i++){
        newvertex temp=newvertex();
        vertices.push_back(temp);
	}
}

void clear(){
    vertices.clear();
    index_map.clear();   
}
// --------------------------------------------------------------------------
// Methods
// --------------------------------------------------------------------------
/**
*
*construct the index map
**/
void Map_index(vector<pair<int,int> > allNewEdges){
    int from, to, index;
    from = to = index = 0;
    for(size_t i=0;i<allNewEdges.size();i++){
        from = allNewEdges[i].first;
        to = allNewEdges[i].second;
        if(index_map.find(from) == index_map.end()){
            index_map[from] = index;
            //fout<<from<<" "<<index<<endl;
            newvertex temp=newvertex();
            temp.id = from;
            vertices.push_back(temp);
            index++;
        }
        if(index_map.find(to) == index_map.end()){
            index_map[to] = index;
            //fout<<to<<" "<<index<<endl;
            newvertex temp=newvertex();
            temp.id = to;
            vertices.push_back(temp);
            index++;
        }
    }
    vertexNum = index_map.size();
	for(int i=0;i<allNewEdges.size();i++){
		int a = allNewEdges[i].first;
		int b = allNewEdges[i].second;
		addEdge(a,b);
		addEdge(b,a);
	}
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
	//find the index of vertex from
	int index_f = index_map[from];
	int index_t = index_map[to];
	// Search if the edge is already present; if so, stop here and return
	// false.
	int len = vertices[index_f].adjacent.size();
	for (int i = 0; i < len; i++) {
		if (vertices[index_f].adjacent[i] == index_t)
			return false;
	}

	// Finally store the edge
	vertices[index_f].adjacent.push_back(index_t);
	edgeNum++;
	//cout<<"("<<from<<","<<to<<") added!"<<endl;
	return true;
}

bool deleteEdge(int from, int to){
    bool flag = true;
    int index_f = index_map[from];
	int index_t = index_map[to];
	//delete <from, to>
    vector <int>::iterator Iter = std::find(vertices[index_f].adjacent.begin(),vertices[index_f].adjacent.end(), index_t);
    if(Iter != vertices[index_f].adjacent.end()){
        vertices[index_f].adjacent.erase(Iter);
        edgeNum--;
        //cout<<"delete "<<from<<","<<to;
    }
    else{
        flag = false;
    }
    return flag;
}

/**
 * Returns the number of nodes in the graph.
 */
int GetvertexNum()
{
	return vertexNum;
}

/**
 * Returns the number of edges in the graph.
 */
int GetedgeNum()
{
	return edgeNum;
}

//set cores for new graph, if its a new vertex, core is default 0
void SetCores(vector<int> allcores){
	for(int u=0;u<vertexNum;u++){
		int idu = vertices[u].id;
		if(idu+1 > allcores.size()){
			vertices[u].core = 0;
		}else{
			vertices[u].core = allcores[idu];
		}
	}
}

//get root core numbers and 
vector<params> GetParams(){
	VECINT cores;
	vector<params> param;
	int index_p = 0;
	for(int u=0;u<vertexNum;u++){
		int coreu = vertices[u].core;
		int corer = coreu;
		int uSize = vertices[u].adjacent.size();
		//vertex u has a neighbor with larger core number
		bool flag = false;
		for(int i=0;i<uSize;i++){
			int v = vertices[u].adjacent[i];
			int corev = vertices[v].core;
			if(corev >= coreu){
				flag = true;
				break;
			}
		}
		if(flag){
			VECINT::iterator iter=std::find(cores.begin(),cores.end(),corer);
			// if the core is not in rootcores, add it
			if(iter == cores.end()){
				cores.push_back(corer);
				param.push_back(params(corer,index_p));
				index_p++;
			}
		}
	}
	return param;
}
	
/**
*greedy coloring, use at most delta+1 colors,
*return edges in a matrix, edges with same color are in a row
***/
vector<unordered_map<int, vector<pair<int,int> > > >GreedyColoring()
{
    vector<unordered_map<int, vector<pair<int,int> > > >coloredEdges;
    vector<VECINT> usedColors(vertexNum+1);
    //initialize colors
	delta = 0;
    for(int u=0;u<vertexNum;u++){
        int uSize = vertices[u].adjacent.size();
        vertices[u].colors.resize(uSize,0);
		if(uSize > delta){
			delta = uSize;
		}
    }
	//color the first vertex
	/*for(int i = 0;i<vertices[0].adjacent.size();i++){
		vertices[0].colors[i] = i+1;
		usedColors[0].push_back(i+1);
		int v = vertices[0].adjacent[i];
		VECINT::iterator it=std::find(vertices[v].adjacent.begin(),vertices[v].adjacent.end(),0);
        int index = (int)(it-vertices[v].adjacent.begin());
        vertices[v].colors[index] = i+1;
		usedColors[v].push_back(i+1);
		max_color = i+1;
	}*/
	coloredEdges.resize(max_color+1);
	//color vertices
    for(int u=0;u<vertexNum;u++){
        int uSize = vertices[u].adjacent.size();
        int idu = vertices[u].id;
		int coreu = vertices[u].core;
		vertices[u].colors.resize(uSize,0);
        //color each <u,v>
        for(int i=0;i<uSize;i++){
            int v = vertices[u].adjacent[i];
			int corev = vertices[v].core;
            int idv = vertices[v].id;
            int colorv = vertices[u].colors[i];//colors[u][i];
			//if the i-th neighbor is uncolored,color it
            if(!colorv){
                vector<bool> IsColorUsed(vertexNum,false);
				//mark the colors that have been used by u & v
                for(int j=0;j<usedColors[u].size();j++){
                    IsColorUsed[usedColors[u][j]] = true;                    
                }
				for(int j=0;j<usedColors[v].size();j++){ 
                    IsColorUsed[usedColors[v][j]] = true;                    
                }
                //find the first unused color c, give it to <u,v> and <v,u>
                int c = 1;
                while(c < vertexNum && IsColorUsed[c]){
                    c++;
                }
                vertices[u].colors[i] = c;//color[u][i]=c
				usedColors[u].push_back(c);
                IsColorUsed[c] = true;
                if(c > max_color){
                    max_color = c;
					coloredEdges.resize(max_color+1);
                }
				int corer = min(corev,coreu);
				coloredEdges[c][corer].push_back(make_pair(idu,idv));
                //find index of u in v`s neighbor list
                VECINT::iterator it=std::find(vertices[v].adjacent.begin(),vertices[v].adjacent.end(), u);
                int index = (int)(it-vertices[v].adjacent.begin());
                vertices[v].colors[index] = c;//color[v][index] = c;
				usedColors[v].push_back(c);
			}
        }
    }
	return coloredEdges;
}
	
const int GetMaxcolor(){
	return max_color;
}

const int GetDelta(){
	return delta;
}

//divide edges in a matching into a map, key is core number value is edges with the core
unordered_map<int, vector<pair<int,int> > > GetSameCoreEdges(vector<edge> sameColorEdges)
{
	unordered_map<int, vector<pair<int,int> > > sameCoreEdges;
    for(int i = 0;i<sameColorEdges.size();i++){
		edge anedge = sameColorEdges[i];
		int core = anedge.core;
		sameCoreEdges[core].push_back(make_pair(anedge.from,anedge.to));
	}	
    return sameCoreEdges;
}

};

#endif // GRAPH_H_INCLUDED
