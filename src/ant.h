#pragma once

#include <bits/stdc++.h>

using namespace std;

class Ant{
public:
	// Construct the ant
	Ant(int id);
	int id;
	int commodity_id;
	int package_size;
	int total_paid;
	double average_cost;
	int priority;
};