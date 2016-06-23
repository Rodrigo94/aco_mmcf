#pragma once

#include <bits/stdc++.h>

using namespace std;

class Ant{
public:
	int id;
	int commodity;
	int package_size;
	int total_paid;
	int current_node;
	void go_to_next_node();
	void leave_trail();
};