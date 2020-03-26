/* Program Name : Snake and Ladder : Quickest way up
   Approach     : Dijkstra/BFS
   Question     : https://www.hackerrank.com/challenges/the-quickest-way-up
   Author       : Utsav Chokshi
   Date         : 12/12/2015
*/
/****
Sample Test case :
4
3
32 62
42 68
12 98
7
95 13
97 25
93 37
79 27
75 19
49 47
67 17
4
8 52
6 80
26 42
2 72
9
51 19
39 11
37 29
81 3
59 5
79 23
53 7
43 33
77 21
0
6
99 69
98 68
97 67
96 66
95 65
94 64
1
2 83
64 99
1
84 63
****/
/****
Expected output :
3
5
-1
4
****/



#include <iostream>
#include <list>

using namespace std;

struct Edge{

	int dest;    // Destination Vertex
	int weight;  // weight
};

class Graph{
private:
	int V;        //Number of vertices
	list<struct Edge> *adjList; 

public:
	Graph(int numVertices){
		V = numVertices;
		adjList = new list<struct Edge>[V]();
	}
	
	//Adds an edge to Graph
	void addEdge(int u, int v, int w){
		struct Edge* temp = new struct Edge;
		temp->dest = v;
		temp->weight = w;
		adjList[u].push_back(*temp);
	}
		
	// Apply dijksta on source vertex which is 1.
	void dijkstra(int S){

		int max = 1000;
		bool* visited = new bool[V]();
		int* dist = new int[V]();

		for(int index=0; index<V; index++){
			dist[index] = max;
		}

		visited[S] = true;
		
		// Intialization
		list<struct Edge>::iterator it;
		
		//Handles edges for snake and ladder.(If such edge exists, do not consider any other edge.)
		if(!adjList[S].empty()){
			for( it = adjList[S].begin() ; it != adjList[S].end(); it++){
				struct Edge temp = (struct Edge)*it;
				dist[temp.dest] = temp.weight;
			}
		}
		// Hnadles edges for next 6 vertices.
		else{
			for(int subIndex=1; subIndex<=6 ; subIndex++){
				dist[S+subIndex] = 1;
			}
		}

		for(int index=1; index<V; index++){
			int u = findmin(visited,dist);
			visited[u] = true;
			
			// Stop dijkstra's algo once '100' vertex is discovered as minimum distance vertex.	
			if(u==100){
				break;
			}
			
			// Updates shortest distance of all vertices adjacent to given vertex

			//Handles edges for snake and ladder.(If such edge exist, do not consider any other edge.)
			if(!adjList[u].empty()){ 
				for( it = adjList[u].begin() ; it != adjList[u].end(); it++){ 
					struct Edge temp = (struct Edge)*it;
					if(visited[temp.dest] == false && dist[temp.dest] > dist[u]+ temp.weight){
						dist[temp.dest] = dist[u]+temp.weight;
					}
			    }
		    }
		    // Hnadles edges for next 6 vertices.
		    else{
		 		for(int subIndex=1; subIndex<=6 ; subIndex++){
					if(u+subIndex<=100 && visited[u+subIndex] == false && dist[u+subIndex] > dist[u]+ 1){
						dist[u+subIndex] = dist[u]+1;
					}	
				}   	
		    }
		}
		
		//Incase, no solution found.
		if(dist[100] == 1000){
			cout << "-1" << endl;
		}		
		else{
			cout << dist[100] << endl;
		}
	}
	
	// Finds minimum vertex out of nodes not visited.
	int findmin(bool* visited, int* dist){

		int min = 1005;
		int minIndex = 2;

		for(int index=1; index<V; index++){
			if(visited[index] == false){
				if(dist[index] < min){
					minIndex = index;
					min = dist[index];
				}
			}
		}
		return minIndex;
	}
};

int main(){

	int T = 0; //Number of test cases
	cin >> T;
	int N = 0; // Number of ladders
	int M = 0; // Number of snakes
	int x=0, y=0;
	int index = 0;

	while(T--){

		// Vertices are numbered from 1 to 100.
		Graph g1(101);
		cin >> N;
		

		// Adding edges from ladder start to end point with weight 0.
		for(index=0; index<N; index++){
			cin >> x >> y;
			g1.addEdge(x,y,0);
		}

		cin >> M;
		
		// Adding edged from snake mouth to tale with weight 0.
		for(index=0; index<M; index++){
			cin >> x >> y;
			g1.addEdge(x,y,0);
		}

		g1.dijkstra(1);
	}

	return 0;
}
