#pragma once

#include <bits/stdc++.h>
#include "node.h"

using namespace std;

class Layer{
public:
	int node_count;
	// Construct the layer
	Layer(int id);
	int id;
	map<int, shared_ptr<Node> > nodes;
	void insert_node(int id);
	void connect_nodes(int node_from, int node_to);
};