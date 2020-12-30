#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <dirent.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include "graph.h"
#include "newgraph.h"
#include "threadpool.h"

using namespace std;

#define CLOCK_PER_MS 1

void FindThreads(vector<params> &param);

void DeleteThread();

void InsertThread();

void InsertEdgesIntoGraph(vector<vector<pair<int, int> > > &insertedEdges);

bool DeleteEdgesFromGraph(vector<vector<pair<int, int> > > &deletedEdges);

void *GetKSuperiorEdges(void *param);

void *TravelInsert(void *sameCoreEdges);

int InsertRemove(int k, int r);

void *TravelDelete(void *sameCoreEdges);

int DeleteRemove(int k, int r);

bool DecreseCore(int v, int k);

map<int, vector<int> > toOrderEdges(vector<pair<int, int> > insEdges);

vector<pair<int, int> > toVectorEdges(map<int, vector<int> > orderEdges);

void PreProcess();


Graph graph;
newGraph newgraph;
vector<vector<pair<int, int> > > superiorEdges;
//int thread_num = 16;
ThreadPool *pool;
// map<int, vector<int> > orderEdges;
map<int, vector<int> > orderEdges;
map<int, vector<int> > delOrderEdges;
pthread_mutex_t orderMutex;

//create finding threads to find superior edges,unused 
void FindThreads(vector<params> &param) {
  int num_threads = param.size();
  pthread_t tids[num_threads];
  for (int i = 0; i < num_threads; i++) {
    int ret = pthread_create(&tids[i], NULL, GetKSuperiorEdges, &param[i]);
    if (ret != 0) {
      cout << "pthread_create error:error_code=" << ret << endl;
    } else {
      //cout <<tids[i]<<" thread_create"<<endl;
    }
  }
  for (int i = 0; i < num_threads; ++i) {
    pthread_join(tids[i], NULL);//(void**)&superiorEdges[i]);
  }
}

//delete superior edges from new graph and insert them into original graph
void InsertEdgesIntoGraph(vector<vector<pair<int, int> > > &insertedEdges) {
  int insertedEdgesSize = insertedEdges.size();
  for (int i = 0; i < insertedEdgesSize; i++) {
    vector<pair<int, int> > kedges = insertedEdges[i];
    int ksize = kedges.size();
    for (int k = 0; k < ksize; k++) {
      int u = kedges[k].first;
      int v = kedges[k].second;
      if (!newgraph.deleteEdge(u, v) || !newgraph.deleteEdge(v, u)) {
        cout << "delete " << u << "," << v << endl;
        cout << "wrong delete edges in new graph!\n";
        return;
      }
      if (!graph.addEdge(u, v) || !graph.addEdge(v, u)) {
        cout << "insert " << u << "," << v << endl;
        cout << "wrong inserting edges in original graph!\n";
        return;
      }
    }
  }
}

//delete superior edges from new graph and original graph
bool DeleteEdgesFromGraph(vector<vector<pair<int, int> > > &deletedEdges) {
  for (int i = 0; i < deletedEdges.size(); i++) {
    vector<pair<int, int> > kedges = deletedEdges[i];
    int ksize = kedges.size();
    for (int k = 0; k < ksize; k++) {
      int u = kedges[k].first;
      int v = kedges[k].second;
      if (!newgraph.deleteEdge(u, v) || !newgraph.deleteEdge(v, u)) {
        cout << "wrong delete " << u << "," << v << " from new graph!" << endl;
        return false;
      }
      if (!graph.deleteEdge(u, v) || !graph.deleteEdge(v, u)) {
        cout << "wrong delete " << u << "," << v << "from original graph!" << endl;
        return false;
      }
    }
  }
  return true;
}

/**
* given a core number k, find the k-superiorEdges in newgraph(the inserted/deleted edges) and add to superiorEdges
*/
vector<pair<int, int> > GetKSuperiorEdges(int k) {
  VECINT rootvertex;
  vector<bool> IsPicked;
  vector<pair<int, int> > ksuperiorEdges;
  int verNumber = 0;
  for (int i = 0; i < newgraph.vertexNum; i++) {
    if (newgraph.vertices[i].core == k) {
      rootvertex.push_back(newgraph.vertices[i].id);
    }
    IsPicked.push_back(false);
  }
  verNumber = rootvertex.size();
  for (int i = 0; i < verNumber; i++) {
    int idu = rootvertex[i];
    int index_u = newgraph.index_map[idu];
    if (!IsPicked[index_u]) {
      vector<int> adjacent = newgraph.vertices[index_u].adjacent;
      int uSize = adjacent.size();
      for (int j = 0; j < uSize; j++) {
        int index_v = adjacent[j];
        int idv = newgraph.vertices[index_v].id;
        int corev = newgraph.vertices[index_v].core;
        if (corev >= k && !IsPicked[index_v]) {
          IsPicked[index_u] = true;
          //if corev == k, set v picked
          if (corev == k) {
            IsPicked[index_v] = true;
          }
          ksuperiorEdges.push_back(make_pair(idu, idv));
          break;
        }
      }
    }
  }
  return ksuperiorEdges;
}

/**
find a superior edge set
**/
vector<pair<int, int> > GetSuperiorEdges() {
  VECINT rootvertex;
  int verNumber = newgraph.vertexNum;
  vector<bool> IsPicked(verNumber, false);
  vector<pair<int, int> > superiorEdges;
  for (int i = 0; i < verNumber; i++) {
    int idu = newgraph.vertices[i].id;
    int index_u = newgraph.index_map[idu];
    int coreu = newgraph.vertices[index_u].core;
    if (!IsPicked[index_u]) {
      vector<int> adjacent = newgraph.vertices[index_u].adjacent;
      int uSize = adjacent.size();
      for (int j = 0; j < uSize; j++) {
        int index_v = adjacent[j];
        int idv = newgraph.vertices[index_v].id;
        int corev = newgraph.vertices[index_v].core;
        if (corev < coreu && !IsPicked[index_v]) {
          superiorEdges.push_back(make_pair(idu, idv));
          IsPicked[index_v] = true;
        } else if (corev >= coreu && !IsPicked[index_v] && !IsPicked[index_u]) {
          IsPicked[index_u] = true;
          //if corev == k, set v picked
          if (corev == coreu) {
            IsPicked[index_v] = true;
          }
          superiorEdges.push_back(make_pair(idu, idv));
        }
      }
    }
  }
  return superiorEdges;
}

/**
* given a core number k, find the k-superiorEdges in newgraph(the inserted/deleted edges) and add to superiorEdges
*/
void *GetKSuperiorEdges(void *param) {
  params par = *(params *) param;
  int k = par.k;
  int index = par.i;
  VECINT rootvertex;
  vector<bool> IsPicked;
  int verNumber = 0;
  for (int i = 0; i < newgraph.vertexNum; i++) {
    if (newgraph.vertices[i].core == k) {
      rootvertex.push_back(newgraph.vertices[i].id);
    }
    IsPicked.push_back(false);
  }
  verNumber = rootvertex.size();
  for (int i = 0; i < verNumber; i++) {
    int idu = rootvertex[i];
    int index_u = newgraph.index_map[idu];
    if (!IsPicked[index_u]) {
      vector<int> adjacent = newgraph.vertices[index_u].adjacent;
      int uSize = adjacent.size();
      for (int j = 0; j < uSize; j++) {
        int index_v = adjacent[j];
        int idv = newgraph.vertices[index_v].id;
        int corev = newgraph.vertices[index_v].core;
        if (corev >= k && !IsPicked[index_v]) {
          IsPicked[index_u] = true;
          //if corev == k, set v picked
          if (corev == k) {
            IsPicked[index_v] = true;
          }
          superiorEdges[index].push_back(make_pair(idu, idv));
          break;
        }
      }
    }
  }
}

/**
*after inserting an edge set, search vertices whose core numbers are same as the roots and
*find vertices that will change cores
*executed by a thread
*@param sameCoreEdges:the roots of new inserted edges, for each root conducts the InsertOneEdge algorithm
*
*/
void *TravelInsert(void *sameCoreEdges) {
  vector<pair<int, int> > rootEdges = *(vector<pair<int, int> > *) sameCoreEdges;
  stack<int> s;
  int k = 0, r = 0;
  long dur = 0;
  int rootsize = rootEdges.size();
  struct timeval f_start, f_end, f_mid;
  for (int i = 0; i < rootsize; i++) {
    int u = rootEdges[i].first;
    int v = rootEdges[i].second;
    int coreu = graph.vertices[u].core;
    int corev = graph.vertices[v].core;
    r = u;
    k = coreu;
    if (coreu > corev) {
      r = v;
      k = corev;
    }
    //only vertex that was not updated need to be travel again
    if (!(graph.vertices[r].visited && !graph.vertices[r].removed)) {
      graph.vertices[r].visited = true;
      if (!graph.vertices[r].pcd) {
        graph.computePcd(r);
      }
      if (graph.vertices[r].cd >= 0)
        graph.vertices[r].cd = graph.vertices[r].pcd;//pcdpair[r];//
        //if r has been attacked before, its cd should be updated not initialized
      else
        graph.vertices[r].cd += graph.vertices[r].pcd;//pcdpair[r];//
      s.push(r);
      int cdr = graph.vertices[r].cd;
      while (!s.empty()) {
        int v = s.top();
        s.pop();
        int cdv = graph.vertices[v].cd;
        if (cdv > k) {
          int sizev = graph.edges[v].size();
          for (int j = 0; j < sizev; j++) {
            int w = graph.edges[v][j];
            int corew = graph.vertices[w].core;

            int mcdw = graph.vertices[w].mcd;//mcdpair[w];//
            if (corew == k && mcdw > k && (!graph.vertices[w].visited)) {
              s.push(w);
              graph.vertices[w].visited = true;
              if (!graph.vertices[w].pcd) {
                graph.computePcd(w);
              }
              graph.vertices[w].cd += graph.vertices[w].pcd;//pcdpair[w];//
            }
          }
        } else if (!graph.vertices[v].removed) {
          InsertRemove(k, v);
        }
      }
    }
  }
}

/*
*for no thread pool
*/
int TravelInsert(vector<pair<int, int> > rootEdges) {
  stack<int> s;
  int k = 0;
  int r = 0;
  int rootsize = rootEdges.size();
  graph.computeInsertMcd(rootEdges);
  for (int i = 0; i < rootsize; i++) {
    int u = rootEdges[i].first;
    int v = rootEdges[i].second;
    int coreu = graph.vertices[u].core;
    int corev = graph.vertices[v].core;
    r = u;
    k = coreu;
    if (coreu > corev) {
      r = v;
      k = corev;
    }
    //only vertex that was not updated need to be travel again
    if (!(graph.vertices[r].visited && !graph.vertices[r].removed)) {
      graph.vertices[r].visited = true;
      if (!graph.vertices[r].pcd) {
        graph.computePcd(r);
      }
      if (graph.vertices[r].cd >= 0)
        graph.vertices[r].cd = graph.vertices[r].pcd;//pcdpair[r];//
        //if r has been attacked before, its cd should be updated not initialized
      else
        graph.vertices[r].cd += graph.vertices[r].pcd;//pcdpair[r];//
      s.push(r);
      int cdr = graph.vertices[r].cd;
      while (!s.empty()) {
        int v = s.top();
        s.pop();
        int cdv = graph.vertices[v].cd;
        if (cdv > k) {
          int sizev = graph.edges[v].size();
          for (int j = 0; j < sizev; j++) {
            int w = graph.edges[v][j];
            int corew = graph.vertices[w].core;
            int mcdw = graph.vertices[w].mcd;//mcdpair[w];//
            if (corew == k && mcdw > k && (!graph.vertices[w].visited)) {
              s.push(w);
              graph.vertices[w].visited = true;
              if (!graph.vertices[w].pcd) {
                graph.computePcd(w);
              }
              graph.vertices[w].cd += graph.vertices[w].pcd;//pcdpair[w];//
            }
          }
        } else if (!graph.vertices[v].removed) {
          InsertRemove(k, v);
        }
      }
    }
  }
}


/*
*use mcd
*/
void *TravelInsertMcd(void *sameCoreEdges) {
  vector<pair<int, int> > rootEdges = *(vector<pair<int, int> > *) sameCoreEdges;
  stack<int> s;
  int k = 0, r = 0;
  long dur = 0;
  int rootsize = rootEdges.size();
  struct timeval f_start, f_end, f_mid;
  //graph.computeInsertMcd(rootEdges);
  //gettimeofday(&f_start, NULL);
  for (int i = 0; i < rootsize; i++) {
    int u = rootEdges[i].first;
    int v = rootEdges[i].second;
    int coreu = graph.vertices[u].core;
    int corev = graph.vertices[v].core;
    r = u;
    k = coreu;
    if (coreu > corev) {
      r = v;
      k = corev;
    }
    //only vertex that was not updated need to be travel again
    if (!(graph.vertices[r].visited && !graph.vertices[r].removed)) {
      graph.vertices[r].visited = true;
      if (!graph.vertices[r].mcd) {
        graph.computeMcd(r);
      }
      if (graph.vertices[r].cd >= 0)
        graph.vertices[r].cd = graph.vertices[r].mcd;//pcdpair[r];//
        //if r has been attacked before, its cd should be updated not initialized
      else
        graph.vertices[r].cd += graph.vertices[r].mcd;//pcdpair[r];//
      s.push(r);
      int cdr = graph.vertices[r].cd;
      while (!s.empty()) {
        int v = s.top();
        s.pop();
        int cdv = graph.vertices[v].cd;
        if (cdv > k) {
          int sizev = graph.edges[v].size();
          for (int j = 0; j < sizev; j++) {
            int w = graph.edges[v][j];
            int corew = graph.vertices[w].core;
            if (corew == k && (!graph.vertices[w].visited)) {
              if (!graph.vertices[w].mcd) {
                graph.computeMcd(w);
              }
              int mcdw = graph.vertices[w].mcd;//mcdpair[w];//
              if (mcdw > k) {
                s.push(w);
                graph.vertices[w].visited = true;
                graph.vertices[w].cd += graph.vertices[w].mcd;
              } else {
                if (!graph.vertices[w].removed) {
                  InsertRemove(k, w);
                }
              }
            }
          }
        } else if (!graph.vertices[v].removed) {
          InsertRemove(k, v);
        }
      }
    }
  }
  //gettimeofday(&f_end, NULL);
  //dur =(f_end.tv_sec - f_start.tv_sec)*1000000+(f_end.tv_usec - f_start.tv_usec);
}

/*
*use mcd
*/
int TravelInsertMcd(vector<pair<int, int> > &rootEdges) {
  int visitedVerNum = 0;
  int visitedEdgeNum = 0;
  stack<int> s;
  int k = 0, r = 0;
  long dur = 0;
  int rootsize = rootEdges.size();
  struct timeval f_start, f_end, f_mid;
  //graph.computeInsertMcd(rootEdges);
  //gettimeofday(&f_start, NULL);
  for (int i = 0; i < rootsize; i++) {
    int u = rootEdges[i].first;
    int v = rootEdges[i].second;
    int coreu = graph.vertices[u].core;
    int corev = graph.vertices[v].core;
    r = u;
    k = coreu;
    if (coreu > corev) {
      r = v;
      k = corev;
    }
    //only vertex that was not updated need to be travel again
    if (!(graph.vertices[r].visited && !graph.vertices[r].removed)) {
      graph.vertices[r].visited = true;
      if (!graph.vertices[r].mcd) {
        graph.computeMcd(r);
      }
      if (graph.vertices[r].cd >= 0)
        graph.vertices[r].cd = graph.vertices[r].mcd;//pcdpair[r];//
        //if r has been attacked before, its cd should be updated not initialized
      else
        graph.vertices[r].cd += graph.vertices[r].mcd;//pcdpair[r];//
      s.push(r);
      int cdr = graph.vertices[r].cd;
      while (!s.empty()) {
        int v = s.top();
        s.pop();
        int cdv = graph.vertices[v].cd;
        if (cdv > k) {
          int sizev = graph.edges[v].size();
          for (int j = 0; j < sizev; j++) {
            int w = graph.edges[v][j];
            int corew = graph.vertices[w].core;
            if (corew == k && (!graph.vertices[w].visited)) {
              if (!graph.vertices[w].mcd) {
                graph.computeMcd(w);
              }
              int mcdw = graph.vertices[w].mcd;//mcdpair[w];//
              if (mcdw > k) {
                //visitedVerNum++;
                s.push(w);
                graph.vertices[w].visited = true;
                graph.vertices[w].cd += graph.vertices[w].mcd;
              } else {
                if (!graph.vertices[w].removed) {
                  visitedEdgeNum += InsertRemove(k, w);
                }
              }
            }
          }
          //visitedEdgeNum+=sizev;
        } else if (!graph.vertices[v].removed) {
          visitedEdgeNum += InsertRemove(k, v);
        }
      }
    }
  }
  return visitedEdgeNum;
}

int InsertRemove(int k, int r) {
  int visitedEdgeNum = 0;
  stack<int> s;
  s.push(r);
  graph.vertices[r].removed = true;
  while (!s.empty()) {
    int v = s.top();
    s.pop();
    for (int i = 0; i < graph.edges[v].size(); i++) {
      int w = graph.edges[v][i];
      int corew = graph.vertices[w].core;
      if (corew == k) {
        graph.vertices[w].cd--;
        int cdw = graph.vertices[w].cd;
        if (cdw == k && !graph.vertices[w].removed) {
          graph.vertices[w].removed = true;
          s.push(w);
        }
        //visitedVerNum++;
      }
    }
    visitedEdgeNum += graph.edges[v].size();
  }
  return visitedEdgeNum;
}

/**
*after deleting a superior edge set, search vertices whose core numbers are same as the roots and
*find vertices that will change cores
*executed by a thread
*@param sameCoreEdges:the roots of deleted edges, for each root conducts the DeleteOneEdge algorithm
*
**/
void *TravelDelete(void *sameCoreEdges) {
  int visitedVerNum = 0;
  vector<pair<int, int> > rootEdges = *(vector<pair<int, int> > *) sameCoreEdges;
  int rootsize = rootEdges.size();
  struct timeval f_start, f_end;
  //gettimeofday(&f_start, NULL);
  for (int i = 0; i < rootsize; i++) {
    int u1 = rootEdges[i].first;
    int u2 = rootEdges[i].second;
    int coreu1 = graph.vertices[u1].core;
    int coreu2 = graph.vertices[u2].core;
    int r = u1;
    int k = coreu1;
    if (coreu1 > coreu2) {
      r = u2;
      k = coreu2;
    }
    if (coreu1 != coreu2) {
      if (!graph.vertices[r].visited) {
        graph.vertices[r].visited = true;
        if (!graph.vertices[r].mcd) {
          graph.computeMcd(r);
        }
        graph.vertices[r].cd = graph.vertices[r].mcd;
        visitedVerNum++;
      }
      if (!graph.vertices[r].removed) {
        int cdr = graph.vertices[r].cd;
        if (cdr < k) {
          visitedVerNum += DeleteRemove(k, r);
        }
      }
    } else {
      if (!graph.vertices[u1].visited) {
        graph.vertices[u1].visited = true;
        if (!graph.vertices[u1].mcd) {
          graph.computeMcd(u1);
        }
        graph.vertices[u1].cd = graph.vertices[u1].mcd;
        visitedVerNum++;
      }
      if (!graph.vertices[u1].removed) {
        int cdu1 = graph.vertices[u1].cd;
        if (cdu1 < k) {
          visitedVerNum += DeleteRemove(k, u1);
        }
      }
      if (!graph.vertices[u2].visited) {
        graph.vertices[u2].visited = true;
        if (!graph.vertices[u2].mcd) {
          graph.computeMcd(u2);
        }
        graph.vertices[u2].cd = graph.vertices[u2].mcd;
        visitedVerNum++;
      }
      if (!graph.vertices[u2].removed) {
        int cdu2 = graph.vertices[u2].cd;
        if (cdu2 < k) {
          visitedVerNum += DeleteRemove(k, u2);
        }
      }
    }
  }
  //gettimeofday(&f_end, NULL);
  //long dur =(f_end.tv_sec - f_start.tv_sec)*1000000+(f_end.tv_usec - f_start.tv_usec);
  //pthread_exit((void*)dur);
}

int DeleteRemove(int k, int r) {
  int visitedEdgeNum = 0;
  stack<int> s;
  // graph.vertices[r].removed = true;
  pthread_mutex_lock(&orderMutex);
  bool flag = DecreseCore(r, k);
  pthread_mutex_unlock(&orderMutex);
  if (flag) {
    graph.vertices[r].removed = true;
    s.push(r);
  }

  while (!s.empty()) {
    int v = s.top();
    s.pop();
    for (int i = 0; i < graph.edges[v].size(); i++) {
      int w = graph.edges[v][i];
      int corew = graph.vertices[w].core;
      if (corew == k) {
        //visitedVerNum++;
        if (!graph.vertices[w].visited) {
          if (!graph.vertices[w].mcd) {
            graph.computeMcd(w);
          }
          int mcdw = graph.vertices[w].mcd;
          graph.vertices[w].cd += mcdw;
          graph.vertices[w].visited = true;
        }
        graph.vertices[w].cd--;
        int cdw = graph.vertices[w].cd;
        if (cdw < k && !graph.vertices[w].removed) {
          pthread_mutex_lock(&orderMutex);
          flag = DecreseCore(w, k);
          pthread_mutex_unlock(&orderMutex);
          if (flag) {
            graph.vertices[w].removed = true;
            s.push(w);
          }
          // graph.vertices[w].removed = true;
          // s.push(w);
        }
      }
    }
    visitedEdgeNum += graph.edges[v].size();
  }
  return visitedEdgeNum;
}

/* 判断节点r是否应该减少核值，如果节点v上有优边插入，则不应该减少核值
/* 返回值：true 应该减少核值，false 不应该减少核值
*/
bool DecreseCore(int v, int k) {
  bool flag = true;
  if (orderEdges.find(v) != orderEdges.end()) {
    vector<int> neighbors = orderEdges[v];
    for (auto it = neighbors.begin(); it != neighbors.end(); it++) {
      if (graph.vertices[*it].core > k) {
        flag = false;
        if (!graph.addEdge(v, *it) || !graph.addEdge(*it, v)) {
          cout << "add edge failed when delete edges" << endl;
          break;
        }
        vector<int> neighbors2 = orderEdges[*it];
        for (auto it2 = neighbors2.begin(); it2 != neighbors2.end(); it2++) {
          if (*it2 == v) {
            neighbors2.erase(it2);
            orderEdges[*it] = neighbors2;
            break;
          }
        }
        graph.vertices[v].cd++;
        neighbors.erase(it);
        orderEdges[v] = neighbors;
        break;
      }
      // else if (graph.vertices[*it].core == k) {
      //   flag = false;
      //   if (!graph.addEdge(v, *it) || !graph.addEdge(*it, v)) {
      //     cout << "add edge failed when delete edges " << endl;
      //     break;
      //   }
      //   vector<int> neighbors2 = orderEdges[*it];
      //   for (auto it2 = neighbors2.begin(); it2 != neighbors2.end(); it2++) {
      //     if (*it2 == v) {
      //       neighbors2.erase(it2);
      //       orderEdges[*it] = neighbors2;
      //       break;
      //     }
			// 	}
      //   graph.vertices[v].cd++;
      //   graph.vertices[*it].cd++;
      //   neighbors.erase(it);
      //   orderEdges[v] = neighbors;
      //   break;
      // }
      else {
        ;
      }
    }
  }
  return flag;
}

/**
matching thread using thread pool
**/
void MatchingThread(unordered_map<int, vector<pair<int, int> > > CoreEdges, void *Travel(void *)) {
  int num_threads = CoreEdges.size();
  pthread_t threads[num_threads];

  unordered_map<int, vector<pair<int, int> > >::iterator iter;
  int i = 0;
  vector<std::future<void *> > results;
  //cout<<"pool size "<<pool.<<endl;
  for (iter = CoreEdges.begin(); iter != CoreEdges.end(); ++iter) {
    results.push_back(pool->enqueue(Travel, &(iter->second)));
  }
  for (auto &&result: results)
    result.get();
  /*for( iter = CoreEdges.begin(); iter != CoreEdges.end(); ++iter){
    vector<pair<int,int> > sameCoreEdge = iter->second;
    for(int j = 0;j<sameCoreEdge.size();j++){
      cout<<"edge: "<<sameCoreEdge[j].first<<","<<sameCoreEdge[j].second<<endl;
    }

  }

  for(iter = CoreEdges.begin(); iter != CoreEdges.end(); ++iter,++i){
    int ret = pthread_create(&threads[i], NULL, Travel,&(iter->second));
  }
  void* retStr;
  long maxtime = 0;
  for(i = 0; i < num_threads; ++i ){
    pthread_join(threads[i],&retStr);
    //cout<<(long)retStr<<endl;
    //maxtime += (long)retStr;
    if((long)retStr > maxtime){
      maxtime = (long)retStr;
    }
  }
  //cout<<maxtime<<endl;*/
}

/**
superior thread using thread pool
**/
void SuperiorThread(void *Travel(void *)) {
  int num_tasks = superiorEdges.size();
  vector<std::future<void *> > results;
  //cout<<"pool size "<<pool.<<endl;
  for (int i = 0; i < num_tasks; i++) {
    results.push_back(pool->enqueue(Travel, &superiorEdges[i]));
  }
  for (auto &&result: results)
    result.get();
}

void selectnewedge(int newedgenum, string edgefile) {
  ofstream fout(edgefile.data());
  vector<int> vertices;
  map<int, int> vermap;
  int k = 0, i = 0;
  for (i = 0; i < graph.vertexNum; i++) {
    vertices.push_back(i);
  }
  std::random_shuffle(vertices.begin(), vertices.end());
  srand((unsigned) time(NULL));

  while (k < newedgenum) {
    int a = vertices[i];
    vermap[a] = 1;
    int randnum = rand() % graph.edges[a].size();
    int b = graph.edges[a][randnum];
    if (vermap[b] != 1) {
      vermap[b] = 1;
    } else {
      b += 1;
    }
    fout << a << " " << b << endl;
    i++;
    k++;
    i %= graph.vertexNum;
  }
}

map<int, vector<int> > toOrderEdges(vector<pair<int, int> > insEdges) {
  map<int, vector<int> > orderEdges;
  for (auto it = insEdges.begin(); it != insEdges.end(); it++) {
    int from = it->first;
    int to = it->second;
    if (orderEdges.find(from) == orderEdges.end()) {
      vector<int> neighbors;
      neighbors.emplace_back(to);
      orderEdges[from] = neighbors;
    }
    else {
      vector<int> neighbors = orderEdges[from];
      neighbors.emplace_back(to);
      orderEdges[from] = neighbors;
    }

    if (orderEdges.find(to) == orderEdges.end()) {
      vector<int> neighbors;
      neighbors.emplace_back(from);
      orderEdges[to] = neighbors;
    }
    else {
      vector<int> neighbors = orderEdges[to];
      neighbors.emplace_back(from);
      orderEdges[to] = neighbors;
    }
  }
  return orderEdges;
}

vector<pair<int, int> > toVectorEdges(map<int, vector<int> > orderEdges) {
  vector<pair<int, int> > vectorEdges;
  for (auto it = orderEdges.begin(); it != orderEdges.end(); it++) {
    int from = it->first;
    vector<int> neighbors = it->second;
    for (auto it2 = neighbors.begin(); it2 != neighbors.end(); it2++) {
      int to = *it2;
      if (from < to) {
        vectorEdges.emplace_back(make_pair(from, to));
      }
    }
  }
  return vectorEdges;
}

void PreProcess() {
    for (auto it1 = orderEdges.begin(); it1 != orderEdges.end(); it1++) {
      int node = it1->first;
      vector<int> ne1 = it1->second;
      // 该节点上没有边删除
      if (delOrderEdges.find(node) == delOrderEdges.end()) {
        continue;
      }
      vector<int> ne2 = delOrderEdges[node];
      int nodeCore = graph.vertices[node].core;
      auto itn1 = ne1.begin();
      auto itn2 = ne2.begin();
      while (itn1 != ne1.end() && itn2 != ne2.end()) {
        int core1 = graph.vertices[*itn1].core;
        int core2 = graph.vertices[*itn2].core;
        if (core1 > nodeCore && core2 > nodeCore) {
          vector<int> vec = orderEdges[*itn1];
          for (auto itV = vec.begin(); itV != vec.end(); itV++) {
            if (*itV == node) {
              vec.erase(itV);
              orderEdges[*itn1] = vec;
              break;
            }
          }
          vec = delOrderEdges[*itn2];
          for (auto itV = vec.begin(); itV != vec.end(); itV++) {
            if (*itV == node) {
              vec.erase(itV);
              delOrderEdges[*itn2] = vec;
              break;
            }
          }
          itn1 = ne1.erase(itn1);
          itn2 = ne2.erase(itn2);
        }
        else if (core1 > nodeCore && core2 <= nodeCore) {
          itn2++;
        }
        else if (core1 <= nodeCore && core2 > nodeCore) {
          itn1++;
        }
        else {
          itn1++;
          itn2++;
        }

      }
      orderEdges[node] = ne1;
      delOrderEdges[node] = ne2;
    }
}

int main(int argc, char *argv[]) {
  // if (argc < 5) {
  //   cout << "Usage: -p(parallel)/-c(centralized)/-m(matching) graph_filename edge_filename threadnum" << endl;
  //   return 0;
  // }
  if (argc < 6) {
    cout << "Usage: -p(parallel)/-c(centralized)/-m(matching) graph_filename edge_ins edge_del threadnum" << endl;
    return 0;
  }
  string fname = argv[2];
  string insEdgeFile = argv[3];
  string delEdgeFile = argv[4];
  int thread_num = atoi(argv[5]);
  // string edge_file = argv[3];
  // int thread_num = atoi(argv[4]);
  pool = new ThreadPool(thread_num);
  // string edgefile = edge_file;
  string graphfile = fname;
  string insFile = insEdgeFile;
  string delFile = delEdgeFile; 

  ifstream fingraph(graphfile.data());
  ifstream finInsEdge(insFile.data());
  ifstream finDelEdge(delFile.data());
  // ifstream finedge(edgefile.data());
  vector<pair<int, int> > allNewEdges, alledges;
  vector<pair<int, int> > allInsEdges, allDelEdges;    // 插入边集，删除边集
  // map<int, vector<int> > orderEdges;
  vector<int> allcores;
  map<pair<int, int>, int> edgemap;
  struct timeval t_start, t_end, f_start, f_end;
  long dur;
  {//open files, read graph and edge file and compute core
    int a, b;
    if (!fingraph) {
      cout << "Error opening " << graphfile << " for input" << endl;
      return 0;
    }
    while (fingraph >> a >> b) {
      if (a == b) continue;
      graph.addEdge(a, b);
      graph.addEdge(b, a);
      if (edgemap[make_pair(b, a)] != 1 && edgemap[make_pair(a, b)] != 1) {
        alledges.emplace_back(make_pair(a, b));
        edgemap[make_pair(a, b)] = 1;
        edgemap[make_pair(b, a)] = 1;
      }
    }
    graph.ComputeCores();
    allcores = graph.GetAllcores();
    //get new graph and set cores
    // if (!finedge) {
    //   cout << "Error opening " << edgefile << " for input" << endl;
    //   return 0;
    // }
    // while (finedge >> a >> b) {
    //   allNewEdges.emplace_back(make_pair(a, b));
    // }
    // newgraph.Map_index(allNewEdges);
    // newgraph.SetCores(allcores);
    if (!finInsEdge) {
      cout << "Error opening " << insFile << "for input" << endl;
      return 0;
    }
    while (finInsEdge >> a >> b) {
      allInsEdges.emplace_back(make_pair(a, b));
    }
    if (!finDelEdge) {
      cout << "Error opening " << delFile << "for input" << endl;
      return 0;
    }
    while (finDelEdge >> a >> b) {
      allDelEdges.emplace_back(make_pair(a, b));
    }
  }

  // //k superior algorithm
  // if (strcmp(argv[1], "-k") == 0) { //delete the edges first and then insert them back
  //   int newEdgeNum = newgraph.GetedgeNum() / 2;
  //   vector<pair<int, int> > ksuperior;
  //   std::cout << edge_file << "\t" << newEdgeNum << "\t";
  //   {
  //     //delete the edges
  //     gettimeofday(&t_start, NULL);
  //     long findTime = 0;
  //     int round = 0;
  //     while (newEdgeNum) {
  //       round++;
  //       vector<params> param;
  //       gettimeofday(&f_start, NULL);
  //       param = newgraph.GetParams();
  //       int index = random() % param.size();
  //       int k = param[index].k;
  //       ksuperior = GetKSuperiorEdges(k);
  //       superiorEdges.push_back(ksuperior);
  //       gettimeofday(&f_end, NULL);
  //       long dur = (f_end.tv_sec - f_start.tv_sec) * 1000000 + (f_end.tv_usec - f_start.tv_usec);
  //       findTime += dur;
  //       DeleteEdgesFromGraph(superiorEdges);

  //       {//deleting threads and update cores for new graph
  //         TravelDelete(&ksuperior);
  //         graph.delCores();
  //         graph.resetVertex();
  //         allcores = graph.GetAllcores();
  //         newgraph.SetCores(allcores);
  //         newEdgeNum = newgraph.GetedgeNum();
  //         superiorEdges.clear();
  //       }
  //     }

  //     gettimeofday(&t_end, NULL);
  //     dur = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
  //     std::cout << findTime << "\t";
  //     std::cout << dur << "\t" << round << "\t";
  //   }

  //   // check core number
  //   graph.ComputeCores();
  //   {
  //     //reconstruct new graph and set cores
  //     allcores = graph.GetAllcores();
  //     newgraph.clear();
  //     newgraph.Map_index(allNewEdges);
  //     newgraph.SetCores(allcores);
  //   }

  //   {//insertion
  //     newEdgeNum = newgraph.GetedgeNum();
  //     gettimeofday(&t_start, NULL);
  //     long findTime = 0.0;
  //     int round = 0;
  //     while (newEdgeNum) {
  //       round++;
  //       vector<params> param;
  //       gettimeofday(&f_start, NULL);
  //       param = newgraph.GetParams();
  //       int index = random() % param.size();
  //       int k = param[index].k;
  //       ksuperior = GetKSuperiorEdges(k);
  //       superiorEdges.push_back(ksuperior);
  //       gettimeofday(&f_end, NULL);
  //       long dur = (f_end.tv_sec - f_start.tv_sec) * 1000000 + (f_end.tv_usec - f_start.tv_usec);
  //       findTime += dur;
  //       InsertEdgesIntoGraph(superiorEdges);

  //       {//insertion threads and update cores for new graph
  //         TravelInsertMcd(ksuperior);
  //         graph.insCores();
  //         graph.resetVertex();
  //         allcores = graph.GetAllcores();
  //         newgraph.SetCores(allcores);
  //         newEdgeNum = newgraph.GetedgeNum();
  //         superiorEdges.clear();
  //       }
  //     }
  //     gettimeofday(&t_end, NULL);
  //     dur = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
  //     std::cout << findTime << "\t";
  //     std::cout << dur << "\t" << round << "\n";
  //   }
  // }

  //   //superior algorithm without parallel
  if (strcmp(argv[1], "-s") == 0) { //delete the edges first and then insert them back
    //string output = path + fname.substr(0,2) + "_time_np.txt";
    orderEdges = toOrderEdges(allInsEdges);
    allNewEdges.swap(allDelEdges);
    newgraph.Map_index(allNewEdges);
    newgraph.SetCores(allcores);
    int newEdgeNum = newgraph.GetedgeNum() / 2;
    // std::cout << edge_file << "\t" << newEdgeNum << "\t";
    //cout<<newEdgeNum<<endl;
    int visitedEdgeNum = 0;
    {//delete the edges
      gettimeofday(&t_start, NULL);
      long findTime = 0.0;
      int round = 0;
      graph.visitVerNum = 0;
      while (newEdgeNum) {
        round++;
        vector<params> param;
        gettimeofday(&f_start, NULL);
        superiorEdges.push_back(GetSuperiorEdges());
        /*param = newgraph.GetParams();
        for(int index=0;index<param.size();index++){
          int k = param[index].k;
          superiorEdges.push_back(GetKSuperiorEdges(k));
        }*/
        gettimeofday(&f_end, NULL);
        long dur = (f_end.tv_sec - f_start.tv_sec) * 1000000 + (f_end.tv_usec - f_start.tv_usec);
        findTime += dur;
        DeleteEdgesFromGraph(superiorEdges);

        {//deleting threads and update cores for new graph
          for (int i = 0; i < superiorEdges.size(); i++) {
            TravelDelete(&superiorEdges[i]);
          }
          graph.delCores();
          allcores = graph.GetAllcores();
          newgraph.SetCores(allcores);
          newEdgeNum = newgraph.GetedgeNum();
          superiorEdges.clear();
        }
      }
      gettimeofday(&t_end, NULL);
      dur = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
      // std::cout << graph.visitVerNum << "\t";
      cout << "Delete: " << dur << endl;
    }

    // check core number
    // graph.ComputeCores();
    graph.CheckCores();
    

    {//reconstruct new graph and set cores
      allcores = graph.GetAllcores();
      newgraph.clear();
      allNewEdges.clear();
      allInsEdges = toVectorEdges(orderEdges);
      allNewEdges.swap(allInsEdges);
      newgraph.Map_index(allNewEdges);
      newgraph.SetCores(allcores);
    }

    {//insertion
      graph.visitVerNum = 0;
      visitedEdgeNum = 0;
      newEdgeNum = newgraph.GetedgeNum();
      gettimeofday(&t_start, NULL);
      long findTime = 0.0;
      int round = 0;
      while (newEdgeNum) {
        round++;
        vector<params> param;
        gettimeofday(&f_start, NULL);
        superiorEdges.push_back(GetSuperiorEdges());

        gettimeofday(&f_end, NULL);
        long dur = (f_end.tv_sec - f_start.tv_sec) * 1000000 + (f_end.tv_usec - f_start.tv_usec);
        findTime += dur;
        InsertEdgesIntoGraph(superiorEdges);

        {//insertion threads and update cores for new graph
          for (int i = 0; i < superiorEdges.size(); i++) {
            TravelInsertMcd(superiorEdges[i]);
          }
          graph.insCores();
          allcores = graph.GetAllcores();
          newgraph.SetCores(allcores);
          newEdgeNum = newgraph.GetedgeNum();
          superiorEdges.clear();

        }
      }
      // std::cout << graph.visitVerNum << "\n";
      gettimeofday(&t_end, NULL);
      dur = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
      cout << "Insertion: " << dur << endl;
    }

    graph.CheckCores();
  }

  //   //superior algorithm with parallel thread pool
  else if (strcmp(argv[1], "-sp") == 0) { //delete the edges first and then insert them back
    int init_ins_count = allInsEdges.size();
    int init_del_count = allDelEdges.size();
    int pre_ins_count;
    int pre_del_count;
    int final_ins_count;
    
    orderEdges = toOrderEdges(allInsEdges);
    delOrderEdges = toOrderEdges(allDelEdges);

    // 预处理
    gettimeofday(&t_start, NULL);
    PreProcess();
    gettimeofday(&t_end, NULL);
    dur = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
    cout << "PreProcess\t" << dur << endl;

    allInsEdges = toVectorEdges(orderEdges);
    allDelEdges = toVectorEdges(delOrderEdges);
    pre_ins_count = allInsEdges.size();
    pre_del_count = allDelEdges.size();
    allNewEdges.swap(allDelEdges);
    newgraph.Map_index(allNewEdges);
    newgraph.SetCores(allcores);
    int newEdgeNum = newgraph.GetedgeNum() / 2;
    //int delta = newgraph.GetDelta();
    // std::cout << edge_file << "\t" << newEdgeNum << "\t";
    //cout<<newEdgeNum<<endl;
    {//delete the edges
      gettimeofday(&t_start, NULL);
      long findTime = 0.0;
      int round = 0;
      while (newEdgeNum) {
        round++;
        vector<params> param;
        gettimeofday(&f_start, NULL);
        param = newgraph.GetParams();
        superiorEdges.resize(param.size());
        FindThreads(param);
        gettimeofday(&f_end, NULL);
        long dur = (f_end.tv_sec - f_start.tv_sec) * 1000000 + (f_end.tv_usec - f_start.tv_usec);
        findTime += dur;
        DeleteEdgesFromGraph(superiorEdges);

        {//deleting threads and update cores for new graph
          SuperiorThread(TravelDelete);
          graph.delCores();
          newgraph.SetCores(graph.allcores);
          newEdgeNum = newgraph.GetedgeNum();
          superiorEdges.clear();
        }
      }

      gettimeofday(&t_end, NULL);
      dur = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
      // std::cout << findTime << "\t";
      // std::cout << dur << "\t" << round << "\t";
      std::cout << "Delete" << '\t' << dur << "\t" << round << std::endl;

    }

    // check core number
    // graph.ComputeCores();
    graph.CheckCores();

    {//reconstruct new graph and set cores
      allcores = graph.GetAllcores();
      newgraph.clear();
      allNewEdges.clear();
      allInsEdges = toVectorEdges(orderEdges);
      final_ins_count = allInsEdges.size();
      allNewEdges.swap(allInsEdges);
      newgraph.Map_index(allNewEdges);
      newgraph.SetCores(allcores);
    }

    {//insertion
      newEdgeNum = newgraph.GetedgeNum();
      gettimeofday(&t_start, NULL);
      long findTime = 0.0;
      int round = 0;
      while (newEdgeNum) {
        round++;
        vector<params> param;
        gettimeofday(&f_start, NULL);
        param = newgraph.GetParams();
        superiorEdges.resize(param.size());
        FindThreads(param);
        gettimeofday(&f_end, NULL);
        long dur = (f_end.tv_sec - f_start.tv_sec) * 1000000 + (f_end.tv_usec - f_start.tv_usec);
        findTime += dur;
        InsertEdgesIntoGraph(superiorEdges);


        {//insertion threads and update cores for new graph
          SuperiorThread(TravelInsertMcd);
          graph.insCores();
          newgraph.SetCores(graph.allcores);
          newEdgeNum = newgraph.GetedgeNum();
          superiorEdges.clear();
        }
      }
      gettimeofday(&t_end, NULL);
      dur = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
      // std::cout << findTime << "\t";
      // std::cout << dur << "\t" << round << "\n";
      std::cout << "Insert" << '\t' << dur << "\t" << round << std::endl;
    }
    graph.CheckCores();
  
    cout << "InitInsEdge\t" << init_ins_count << "\t" << "InitDelEdge\t" << init_del_count << endl;
    cout << "PreInsEdge\t" << pre_ins_count << "\t" << "PreDelEdge\t" << pre_del_count << endl;
    cout << "FinalInsEdge\t" << final_ins_count << endl;
  }

    //centralized algs, traversal alg
  else if (strcmp(argv[1], "-c") == 0) {
    // int newEdgeNum = newgraph.GetedgeNum() / 2;
    // std::cout << edge_file << "\t" << newEdgeNum << "\t";
    double ins_t, del_t;
    map<int, vector<int> > orderEdges;
    {//delete the selected edges
      gettimeofday(&t_start, NULL);
      orderEdges = toOrderEdges(allInsEdges);
      // graph.Deletion(allNewEdges);
      graph.Deletion(allDelEdges, orderEdges);
      gettimeofday(&t_end, NULL);
      del_t = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
      // std::cout << graph.visitVerNum << "\t";
      std::cout << "Delete" << '\t' << del_t << std::endl;
      cout << graph.edgeCount / 2 << endl;
      // graph.CheckCores();
    }
    graph.ComputeCores();
    {//insert edges back
      allInsEdges = toVectorEdges(orderEdges);
      gettimeofday(&t_start, NULL);
      // graph.Insertion(allNewEdges);
      graph.Insertion(allInsEdges);
      gettimeofday(&t_end, NULL);
      ins_t = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
      // std::cout << graph.visitVerNum << "\n";
      std::cout << "Insert" << '\t' << ins_t << std::endl;
      graph.CheckCores();
    }
  }
  //   //matching with threads
  // else if (strcmp(argv[1], "-mp") == 0) {
  //   int newEdgeNum = newgraph.GetedgeNum() / 2;
  //   // std::cout << fname << "\t" << newEdgeNum << "\t";
  //   vector<unordered_map<int, vector<pair<int, int> > > > coloredEdges;

  //   gettimeofday(&t_start, NULL);
  //   coloredEdges = newgraph.GreedyColoring();
  //   gettimeofday(&t_end, NULL);
  //   double colorTime = (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
  //   int max_color = newgraph.GetMaxcolor();
  //   int delta = newgraph.GetDelta();
  //   long  del_t, ins_t;
  //   del_t = ins_t = colorTime;
  //   int round;
  //   {//delete edges
  //     dur = 0;
  //     round = 0;
  //     int curcolor = 1;
  //     gettimeofday(&t_start, NULL);
  //     while (curcolor <= max_color) {
  //       graph.deleteEdgeSet(coloredEdges[curcolor]);
  //       MatchingThread(coloredEdges[curcolor], TravelDelete);
  //       graph.delCores();
  //       curcolor++;
  //       round++;
  //     }
  //     gettimeofday(&t_end, NULL);
  //     del_t += (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
  //     // std::cout << del_t << "\t" << colorTime << "\t" << round << "\t";
  //     std::cout << "Delete" << '\t' << del_t << "\t" << round << std::endl;
  //   }
  //   // check core number
  //   graph.ComputeCores();
  //   {//insert edges back
  //     dur = 0;
  //     round = 0;
  //     int curcolor = 1;
  //     gettimeofday(&t_start, NULL);
  //     while (curcolor <= max_color) {
  //       graph.insertEdgeSet(coloredEdges[curcolor]);
  //       MatchingThread(coloredEdges[curcolor], TravelInsert);
  //       curcolor++;
  //       graph.insCores();
  //       round++;
  //     }
  //     gettimeofday(&t_end, NULL);
  //     ins_t += (t_end.tv_sec - t_start.tv_sec) * 1000000 + (t_end.tv_usec - t_start.tv_usec);
  //     // std::cout << ins_t << "\t" << colorTime << "\t" << round << "\n";
  //     std::cout << "Insert" << '\t' << ins_t << "\t" << round << std::endl;
  //   }
  // }
  else {
    cout << "wrong params!" << endl;
  }
  {//close the file
    fingraph.close();
    finInsEdge.close();
    finDelEdge.close();
  }
  return 0;
}
