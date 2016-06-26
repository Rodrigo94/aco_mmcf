#pragma once

#include <bits/stdc++.h>
#include "ant.h"
#include "node.h"
#include "layer.h"

using namespace std;

class ACO{
public:
	// Contruct given a filename
	ACO(string& filename);
	// Main parameters for the model
	int epochs;
	int layers_count;
	int ants_count;
	int pheromone_degeneration;
	int commodities_count;
	int nodes_count;
	int density;
	// Structures for this model
	// Every variable maps ids to objects
	map<int, int> supply;
	map<int, int> demand;
	map<int, unique_ptr<Ant> > ants;
	map<int, shared_ptr<Layer> > layers;
	// Main tables
	map<int, map<int, int> > capacity_table;
	map<int, map<int, int> > remaining_capacity_table;
	map<int, map<int, map<int, int> > > cost_table;
	map<int, map<int, map<int, double> > > desirability;
	map<int, map<int, map<int, double> > > pheromone_table;
	// Stores the path of every ants
	map<int, vector<int> > ant_path;
	// Functions that should do most of the work
	void load_model(string& model_file, string& supply_file);
	void start_model();
	void one_step();
	void setup_ants();
	void check_ants();
	void update_tables();
	// Auxiliar functions
	int randomly_select_node(int node_from, int commodity_id);
};
