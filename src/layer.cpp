#include "layer.h"

Layer::Layer(int id) {
	id = id;
}

void Layer::insert_node(int id){
	if(nodes[id] == NULL){
		nodes[id] = make_shared<Node>(id);
		nodes[id]->id = id;
		node_count++;
	}
}


