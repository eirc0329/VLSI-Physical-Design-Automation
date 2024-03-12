#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H
#include "Wrapper.hpp"
#include "NumericalOptimizerInterface.h"
#include <vector>
using namespace std;

class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(wrapper::Placement &placement);
	wrapper::Placement &_placement;
	
    void evaluateFG(const vector<double> &x, double &f, vector<double> &g);
    void evaluateF(const vector<double> &x, double &f);
    unsigned dimension();
	
	//core info
	double coreWidth;
	double coreHeight;
	int num_modules;
	int num_nets;
	int num_pins;  

	//Log-Sum-Exp
	double eta;
	vector<double> exp_terms;
	
	//bin density
	int beta;
	int num_bins;
	double bin_width;
	double bin_height;
	vector<double> cur_g;
	vector<double> Db;
	double Tb;

};
#endif // EXAMPLEFUNCTION_H
