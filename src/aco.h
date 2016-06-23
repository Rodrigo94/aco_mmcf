#pragma once

#include <bits/stdc++.h>

using namespace std;

class ACO{
public:
	ACO();
	int epochs;
	int layers_count;
	int ants_count;
	int pheromone_degeneration;
	vector<int> ants;
	vector<int> nodes;
	vector<int> tunnels;
	vector<int> layers;
	void load_rules();
	void start_model();
	void one_step();
};
