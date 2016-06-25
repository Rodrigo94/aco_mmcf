#include "aco.h"

int main(){
	srand (time(NULL));
	string filename1("data/aco.conf");
	string filename2("data/aco.model");
	string filename3("data/aco.supply");
	ACO A(filename1);
	A.load_model(filename2, filename3);
	cout << (double)rand() / RAND_MAX << endl;
	A.one_step();
  return 0;
}