#pragma once

#include "ant.h"
#include <bits/stdc++.h>

using namespace std;

class Node{
public:
	// Create a node
	Node(int id);
	int id;
	// Structures for this node
	// Every variable maps ids to objects
	map<int, unique_ptr<Ant> > ants;
};