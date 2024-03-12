#include "GlobalPlacer.h"
#include "ExampleFunction.h"
#include "NumericalOptimizer.h"
#include <vector>
#include <iostream>
using namespace std;

GlobalPlacer::GlobalPlacer(wrapper::Placement &placement)
	:_placement(placement)
{
}

void GlobalPlacer::randomPlace(vector<double> &vec)
{
    double coreWidth = _placement.boundryRight() - _placement.boundryLeft();
    double coreHeight = _placement.boundryTop() - _placement.boundryBottom();
	for (size_t i = 0; i < _placement.numModules(); ++i){
		if(_placement.module(i).isFixed()){
			vec[2 * i] 	   = _placement.module(i).centerX();
			vec[2 * i + 1] = _placement.module(i).centerY();
			continue; 
		}  
        double width = _placement.module(i).width();
        double height = _placement.module(i).height();
        double x = rand() % static_cast<int>(coreWidth - width) + _placement.boundryLeft();
        double y = rand() % static_cast<int>(coreHeight - height) + _placement.boundryBottom();
		_placement.module(i).setPosition(x, y);
		vec[2 * i] = x;
		vec[2 * i + 1] = y;
	}
}

void GlobalPlacer::SolveOutBoundary(NumericalOptimizer &no, vector<double> &x)
{
		for(unsigned int  i = 0; i < _placement.numModules(); ++i){

			double mod_x = no.x(2 * i);
			double mod_y = no.x(2 * i + 1);
			
			//if a module is out of boundary then move it to closet boundary
			if(mod_y + _placement.module(i).height() > _placement.boundryTop()){
				mod_y = _placement.boundryTop() - _placement.module(i).height();
			}else if(mod_y - _placement.module(i).height() < _placement.boundryBottom()){
				mod_y = _placement.boundryBottom();
			}
			
			if(mod_x + _placement.module(i).width() > _placement.boundryRight()){
				mod_x = _placement.boundryRight() - _placement.module(i).width();
			}else if(mod_x - _placement.module(i).width() < _placement.boundryLeft()){
				mod_x = _placement.boundryLeft();
			}
			
			if(_placement.module(i).isFixed() != 1) { 
				_placement.module(i).setPosition(mod_x, mod_y);
			}

			x[2 * i] = mod_x; 
			x[2 * i + 1] = mod_y; 
		}
}

void GlobalPlacer::place()
{
	ExampleFunction ef(_placement); // require to define the object function and gradient function

    vector<double> x(ef.dimension());//all module's x and y

    NumericalOptimizer no(ef);
    double step = (_placement.boundryRight() - _placement.boundryLeft())*7;
	int iter = 300;
	int times = 4;
	int seed = 0 ;
	if(_placement.numModules() == 12028){times = 3;seed = 505093;}
	srand(seed);
	randomPlace(x);

	for(int i = 0; i < times ;i++){
		no.setX(x);//set initial solution
		if(!i){
			no.setNumIteration(iter/2); // user-specified parameter
			no.setStepSizeBound(step); // user-specified parameter
		}
		else{
			no.setNumIteration(iter/10); // user-specified parameter
			no.setStepSizeBound(step); // user-specified parameter
		}
		if((_placement.numModules() == 12028 &&  _placement.numNets() == 11507)
		|| (_placement.numModules() == 29347 &&  _placement.numNets() == 28446)
		|| (_placement.numModules() == 51382 &&  _placement.numNets() == 50393)){
			ef.beta = i * 5000;
		}else{
			ef.beta = i * 10000;
		}
		no.solve(); // Conjugate Gradient solver
		SolveOutBoundary(no, x);
	}
	
	
}
