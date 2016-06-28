#include "aco.h"

// Constructs the model given a list of parameters
ACO::ACO(string& filename) {
	// Load the main parameters of the model
	map<string, int> aco_configs;
	ifstream file(filename.c_str());
	string k,v;
	if(file.is_open()){
		while(!file.eof()){
			file >> k;
			file >> v;
			aco_configs[k] = atoi(v.c_str());
		}
	} else {
		exit(1);
	}
	epochs = aco_configs["EPOCHS"];
	ants_count = aco_configs["ANTS"];
	nodes_count = aco_configs["NODES"];
	layers_count = aco_configs["LAYERS"];
	pheromone_degeneration = aco_configs["PHEROMONE_DEGRADATION"];
	commodities_count = aco_configs["COMMODITIES"];
	density = aco_configs["DENSITY"];
	pheromone_constant = aco_configs["PHEROMONE_CONSTANT"];
	pheromone_max = aco_configs["PHEROMONE_MAX"];
	pheromone_min = aco_configs["PHEROMONE_MIN"];
	file.close();
	// Setup the ants
	for(int i=0; i<ants_count; i++){
		unique_ptr<Ant> a(new Ant(i));
		ants[i] = move(a);
		ants[i]->id = i;
		ants[i]->commodity_id = (i % commodities_count) + 1;
		ants[i]->package_size = 1;
		ant_path[i] = vector<int>();
	}
	// Setup the layers
	int nodes_per_layer = int(nodes_count/(layers_count-2));
	for(int i=0; i<layers_count; i++){
		layers[i] = make_shared<Layer>(i);
		layers[i]->id = i;
	}
	// Setup the nodes
	for(int i=1; i<=nodes_count+2; i++){
		int layer_id = int(ceil(double(i-1)/nodes_per_layer));
		layers[layer_id]->insert_node(i);
	}
}

// Build a model given a network structure
void ACO::load_model(string& model_file, string& supply_file){
	// Firstly setup arcs capacities, costs etc
	ifstream file(model_file.c_str());
	string s,t,k,c,u;
	int node_from, node_to, commodity_id, cost, capacity;
	if(file.is_open()){
		while(!file.eof()){
			file >> s;
			file >> s;
			file >> t;
			file >> k;
			file >> c;
			file >> u;
			node_from = atoi(s.c_str()); 
			node_to = atoi(t.c_str()); 
			commodity_id = atoi(k.c_str());
			cost = atoi(c.c_str());
			capacity = atoi(u.c_str());
			capacity_table[node_from][node_to] = capacity;
			remaining_capacity_table[node_from][node_to] = capacity;
			cost_table[node_from][node_to][commodity_id] = cost;
			desirability[node_from][node_to][commodity_id] = 1./cost;
			pheromone_table[node_from][node_to][commodity_id] = double(pheromone_min);
			file >> u;
		}
	} else {
		exit(1);
	}
	file.close();
	// Then, setup supply
	ifstream file2(supply_file.c_str());
	if(file2.is_open()){
		while(!file2.eof()){
			file2 >> s;
			file2 >> u;
			demand[atoi(s.c_str())] = atoi(u.c_str());
			supply[atoi(s.c_str())] = 1;
		}
	} else {
		exit(1);
	}
	file2.close();
}

void ACO::one_step(){
	setup_ants();
	// For each layer
	for(int i=0; i<layers_count-1; i++){
		for(auto& node: layers[i]->nodes){
			for(auto& ant: node.second->ants){
				int ant_id = ant.second->id;
				int source = node.first;
				// Randomly selects a target node that has at least 1 unit of capacity free
				int target = randomly_select_node(node.first,ant.second->commodity_id);
				// The package must respect ant's limit 
				int package = min(ant.second->package_size, remaining_capacity_table[source][target]);
				// Update the supply if the ant drops anything
				if(package > ant.second->package_size){
					supply[ant.second->commodity_id] -= (package - ant.second->package_size);
				}
				ant.second->package_size = package;
				// Move the ant pointer
				layers[i+1]->nodes[target]->ants[ant_id] = move(ant.second);
				ant.second.reset(nullptr);
				layers[i]->nodes[source]->ants.erase(ant_id);
				// Update ant's path
				ant_path[ant_id].push_back(target);
				// Update arc's capacity
				remaining_capacity_table[source][target] -= package;
			}
		}
	}
	check_ants();
	update_tables();
}

void ACO::setup_ants(){
	for(int i=0; i<ants_count; i++){
		ant_path[i].clear();
		ant_path[i] = vector<int>();
		layers[0]->nodes[1]->ants[i] = move(ants[i]);
		ants[i].reset(nullptr);
		ants.erase(i);
	}
}

void ACO::check_ants(){
	// Send ants back home and evaluate them
	for(int i=0; i<ants_count; i++){
		ants[i] = move(layers[layers_count-1]->nodes[nodes_count+2]->ants[i]);
		layers[layers_count-1]->nodes[nodes_count+2]->ants[i].reset(nullptr);
		layers[layers_count-1]->nodes[nodes_count+2]->ants.erase(i);
		// Compute the cost of this ant
		int ant_cost = 0;
		int previous_node = 1;
		int commodity_id = ants[i]->commodity_id;
		for(int j=0; j<layers_count; j++){
			int next_node = ant_path[i][j];
			ant_cost += cost_table[previous_node][next_node][commodity_id];
			previous_node = next_node;
		}
		ant_cost *= ants[i]->package_size;
		ants[i]->total_paid = ant_cost;
		if(demand[commodity_id] - supply[commodity_id] > 0){
			ants[i]->package_size += 1;
			supply[commodity_id] += 1;
		}
	}
}

void ACO::update_tables(){
	// Updates the pheromone table:
	for(auto& node_from : pheromone_table){
		for(auto& node_to : node_from.second){
			for(auto& commodity : node_to.second){
				int i = node_from.first;
				int j = node_to.first;
				int k = commodity.first;
				double old_pheromone = pheromone_table[i][j][k];
				double new_pheromone = pheromone_table[i][j][k] * (1. - double(pheromone_degeneration) / 100.);
				if(new_pheromone < double(pheromone_min)){
					new_pheromone = double(pheromone_min);
				}
				pheromone_table[i][j][k] = new_pheromone;
			}
		}
	}
	// Updates the pheromone dropped by the ants
	for(int i=0; i<ants_count; i++){
		double delta_pheromone = double(pheromone_constant)/ double(ants[i]->total_paid);
		int commodity_id = ants[i]->commodity_id;
		int previous_node = 1;
		for(int j=0; j<layers_count; j++){
			int next_node = ant_path[i][j];
			double old_pheromone = pheromone_table[previous_node][next_node][commodity_id];
			double new_pheromone = delta_pheromone + pheromone_table[previous_node][next_node][commodity_id];
			if(new_pheromone > double(pheromone_max)){
				new_pheromone = double(pheromone_max);
			}
			pheromone_table[previous_node][next_node][commodity_id] = new_pheromone;
			previous_node = next_node;
		}
	}
	// Reset the remaining capacities table
	for(auto& node_from : remaining_capacity_table){
		for(auto& node_to : node_from.second){
			remaining_capacity_table[node_from.first][node_to.first] = capacity_table[node_from.first][node_to.first];
		}
	}
}

int ACO::randomly_select_node(int node_from, int commodity_id){
	double r = (double)rand() / RAND_MAX;
	double s,sum = 0.0;
	int target;
	for(auto& it: pheromone_table[node_from]){
		if(remaining_capacity_table[node_from][it.first] > 0){
			sum += (pheromone_table[node_from][it.first][commodity_id] * desirability[node_from][it.first][commodity_id]);
		}
	}
	for(auto& it: pheromone_table[node_from]){
		if(remaining_capacity_table[node_from][it.first] > 0){
			target = it.first;
			s += (pheromone_table[node_from][it.first][commodity_id] * desirability[node_from][it.first][commodity_id]) / sum;
			if(s >= r){
				break;
			}
		}
	}
	return target;
}