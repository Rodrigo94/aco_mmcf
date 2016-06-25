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
	file.close();
	// Setup the ants
	for(int i=0; i<ants_count; i++){
		unique_ptr<Ant> a(new Ant(i));
		ants[i] = move(a);
		ants[i]->id = i;
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
			//cout << "from " << node_from << " to " << node_to << " layer " << layer << " commodity_id " << commodity_id << " cost " << cost << " capacity " << capacity << endl;
			cost_table[node_from][node_to][commodity_id] = cost;
			capacity_table[node_from][node_to] = capacity;
			pheromone_table[node_from][node_to] = 0.0;
			probability_table[node_from][node_to] = 1.0;
			file >> u;
		}
	} else {
		exit(1);
	}
	// Set the probabilities table
	for(int i=1; i<nodes_count; i++){
		for(auto& it: probability_table[i]){
			int j=it.first;
			probability_table[i][j] /= probability_table[i].size(); 
			//cout << "Para ir de " << i << " ate " << j << " tem prob " <<  probability_table[i][j] << endl;
		}
	}
	file.close();
	// Then, setup supply
	ifstream file2(supply_file.c_str());
	if(file2.is_open()){
		while(!file2.eof()){
			file2 >> s;
			file2 >> u;
			demand[atoi(s.c_str())] = atoi(u.c_str());
			supply[atoi(s.c_str())] = 0;
		}
	} else {
		exit(1);
	}
	file2.close();
	// Setup the ants
	for(int i=0; i<ants_count; i++){
		ants[i]->commodity_id = (i % commodities_count) + 1;
		ants[i]->package_size = 1;
		ants[i]->priority = 1;
		layers[0]->nodes[1]->ants[i] = move(ants[i]);
	}
}

void ACO::one_step(){
	// For each layer
	for(int i=0; i<layers_count-1; i++){
		//cout << "Camada de no " << layers[i]->id << endl;
		for(auto& node: layers[i]->nodes){
			//cout << "No' de no " << node.second->id << endl;
			for(auto& ant: node.second->ants){
				int ant_id = ant.second->id;
				int source = node.first;
				int target = randomly_select_node(node.first);
				//cout << "Formiga no " << ant.second->id << " e commodity " << ant.second->commodity_id << endl;
				//cout << "\tVai ir pro " << target << endl;
				layers[i+1]->nodes[target]->ants[ant_id] = move(ant.second);
				ant.second.reset(nullptr);
				cout << "Vou tentar deletar a formiga " << ant_id << endl;
				layers[i]->nodes[source]->ants.erase(ant_id);
			}
		}
	}
	// For each layer
	cout << "Agora vou ver as formigas q sobraram " << endl;
	for(int i=0; i<layers_count; i++){
		cout << "Camada de no " << layers[i]->id << endl;
		for(auto& node: layers[i]->nodes){
			cout << "No' de no " << node.second->id << " : " << node.second->ants.size() << endl;
			for(auto& ant: node.second->ants){
				int ant_id = ant.second->id;
				int target = randomly_select_node(node.first);
				cout << "Formiga no " << ant.second->id << " e commodity " << ant.second->commodity_id << endl;
				//cout << "\tVai ir pro " << target << endl;
				//layers[i+1]->nodes[target]->ants[ant_id] = move(ant.second);
			}
		}
	}
}

int ACO::randomly_select_node(int node_from){
	double r = (double)rand() / RAND_MAX;
	double s = 0.0;
	int target;
	for(auto& it: probability_table[node_from]){
		target = it.first;
		s += it.second;
		if(s >= r){
			break;
		}
	}
	return target;
}