#pragma once

#include <bits/stdc++.h>

using namespace std;

class Tunnel{
public:
	int id;
	int s;
	int t;
	int cost;
	int pheromone;
	int capacity;
	void degenerate_pheromone();
};

