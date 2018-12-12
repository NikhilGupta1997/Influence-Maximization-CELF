#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <algorithm>
#include <string>

#define max 65536

using namespace std;

// A Node stucture that creates a graph
struct node {
	int index;
	int degree;
	bool activated;
	bool visited;

	// Initialize a new node
	node(int a) {
		index = a;
		degree = 0;
		activated = false;
		visited = false;
	}
};

// Graph Class consisting of nodes and edges
class Graph {
  public:
	vector<node*> nodelist;
	vector<node*> *adjlist;
	float weight;

	// Initialize a new Graph
	Graph() {
		adjlist = new vector<node*>[max];
	}

	// Find a node by index
	node* findnode(int a) {
		int nsize = nodelist.size();
		int i;
		for (i=0; i<nsize; i++) {
			if (nodelist[i]->index==a)
				return nodelist[i];
		}
		return NULL; // return nsize if node does not exist
	}

	// Add a node to graph
	void addnode(int a) {
		if (findnode(a)==NULL) {
			node* newnode = new node(a);
			nodelist.push_back(newnode);
		}
	}

	// Add an edge between 2 nodes
	void addEdge(int u, int v) {
		adjlist[u].push_back(findnode(v));
		findnode(u)->degree++;
	}

	// Check to see if edge exists between two nodes
	bool findedge(int u, int v) {
		int usize = adjlist[u].size();
		for (int i=0; i<usize; i++) {
			if (adjlist[u][i]->index == v)
				return true;
		}
		return false;
	}

	// Activate a node by index
	void activatenode(int a) {
		node* mynode = findnode(a);
		if (mynode != NULL) {
			mynode->activated = true;
		}
	}

	// Get number of activated nodes
	int get_activated_count() {
		int count = 0;
		for(int i = 0; i < nodelist.size(); i++) {
			if(nodelist[i]->activated)
				count++;
		}
		return count;
	}

	// Deactivate all nodes
	void deactivate_graph() {
		for(int i = 0; i < nodelist.size(); i++) {
			nodelist[i]->activated = false;
		}
	}

	// Activation Probability
	bool try_activation() {
		return rand() % 1000 < 1000*weight;
	}

	// Traverse nodes with activation of neighbors
	void traverse(node* curr, vector<node*> &traversed) {
		int a = curr->index;
		curr->visited = true;
		traversed.push_back(curr);
		int usize = adjlist[a].size();
		for(int i = 0; i < usize; i++) {
			if(!adjlist[a][i]->activated && !adjlist[a][i]->visited) {
				if(try_activation()) {
					traverse(adjlist[a][i], traversed);
				}
			}
		}
	}

	// Activate list of traversed nodes
	void activate(vector<node*> &traversed) {
		for(int i = 0; i < traversed.size(); i++) {
			traversed[i]->activated = true;
		}
	}

	// Set list of traversed nodes to unvisited
	void unvisit(vector<node*> &traversed) {
		for(int i = 0; i < traversed.size(); i++) {
			traversed[i]->visited = false;
		}
	}

} master_graph;

// Initialize Graph
void read_input(string filename) {
	fstream in;
	char file[1024];
	strcpy(file, filename.c_str());
	in.open(file);
	if(!in.is_open()) {
        return;
    }
    else {
    	int node1, node2;
    	while (!in.eof()) {
    		in >> node1;
    		in >> node2;
    		master_graph.addnode(node1);
    		master_graph.addnode(node2);
    		master_graph.addEdge(node1, node2);
    	}
    }
    in.close();
}

struct sort_pred {
    bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
        return left.second > right.second;
    }
};

void create_table(vector<node*> *traversed, vector< pair<int,int> > &table, int size) {
	for(int i = 0; i < size; i++) {
			table.push_back(make_pair(i, traversed[i].size()));
	}
	sort(table.begin(), table.end(), sort_pred());
}

int count_unactivated(vector<node*> &vec) {
	int count = 0;
	int vsize = vec.size();
	for(int i = 0; i < vsize; i++) {
		if(!vec[i]->activated) {
			count ++;
		}
	}
	return count;
}

void update_table(vector< pair<int,int> > &table, vector<node*> *traversed, int index) {
	master_graph.activate(traversed[index]);
	int tsize = table.size();
	for(int i = 0; i < tsize; i++) {
		int curr = table[i].first;
		table[i].second = count_unactivated(traversed[curr]);
		if(table[i].second > table[i+1].second) {
			break;
		}
	}
	sort(table.begin(), table.end(), sort_pred());
}

void get_max(vector< pair<int,int> > &table, vector<int> &maxval, int size) {
	for(int i = 0; i < size; i++) {
		maxval.push_back(table[i].first);
	}
}

void add_results(int results[], vector<int> &maxval) {
	for(int i = 0; i < maxval.size(); i++) {
		results[maxval[i]]++;
	}
}

int main() {
	srand(time(NULL));

	// Initialize Graph
	read_input("nethept.inf"); // Change input file based on dataset
	master_graph.weight = 0.1; // Change weight to effect influence reach
	int k = 200; // Number of seeds
	long activated = 0;
	int size = master_graph.nodelist.size(); // Number of nodes

	vector<node*> *traversed;
	traversed = new vector<node*>[max];
	vector< pair<int,int> > table;
	vector<int> maxval;
	int count = 1, limit = 100; // Run 100 iterations
	while(count <= limit) {
		class Graph input_graph(master_graph);
		cout<<"iteration "<<count<<endl;
		node* curr;
		for(int i = 0; i < size; i++) { // simulate the influence reach of each node in the graph
			curr = input_graph.findnode(i);
			input_graph.traverse(curr, traversed[i]);
			input_graph.unvisit(traversed[i]);
		}
		create_table(traversed, table, size); //  Create table storing marginal influence gains
		for(int j = 0; j < k; j++) {
			maxval.push_back(table[0].first);
			update_table(table, traversed, maxval[j]); // Lazy greedy update
		}
		for(int i = 0; i < size; i++)
			traversed[i].clear();
		maxval.clear();
		table.clear();
		count++;
		int temp_activates = input_graph.get_activated_count();
		activated += temp_activates;
		cout<<"activated = "<<temp_activates<<endl;
		input_graph.deactivate_graph();
	}
	cout<<"Average Graph Spread = "<< activated / limit <<endl;
}