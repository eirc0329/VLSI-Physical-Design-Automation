#include "ExampleFunction.h"
#include <cmath>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <algorithm> 
using namespace std;

// minimize Objective function :WL()+Density()

ExampleFunction::ExampleFunction(wrapper::Placement &placement)
	:_placement(placement)
{
	//core info
	coreWidth   = _placement.boundryRight() - _placement.boundryLeft();
	coreHeight  = _placement.boundryTop() - _placement.boundryBottom();
	num_modules = _placement.numModules();
	num_nets     = _placement.numNets();
	num_pins     = _placement.numPins();

	//Log-Sum-Exp
	eta = (coreHeight*1.5 + coreWidth*1.5)/1450;
	if(num_modules == 12028)eta = ((coreHeight*1.15 + coreHeight*1.04) / 1375);
	if(num_modules == 29347)eta = (coreHeight*1.75 + coreWidth*1.75)/1450;
	exp_terms.resize(_placement.numModules() * 4);

	//bin density
	beta = 0;
	num_bins = 256;
	bin_width = coreWidth / 14;
	bin_height = coreHeight / 14;
	cur_g.resize(_placement.numModules() * 2);
	Db.resize(num_bins);
	Tb = 0;
	for(unsigned i = 0; i < _placement.numModules(); i++){
		Tb += _placement.module(i).area();
	}
	Tb /= (coreWidth * coreHeight);
}

void ExampleFunction::evaluateFG(const vector<double> &x, double &f, vector<double> &g){

	//reset object function & gradient 
	f = 0;
	fill(g.begin(), g.end(), 0);

	/********************************************************/
	/********************Log-sum-Exp*************************/
	/********************************************************/
	
	//calculate four exp_terms for every modules
	for(int i = 0; i < num_modules; i++){
		double curX,curY;
		if (_placement.module(i).isFixed()){
			curX = _placement.module(i).centerX();
			curY = _placement.module(i).centerY();
		}else{
			curX =  x[2*i];
			curY =	x[2*i+1];
		}
		exp_terms[4*i]   = exp(curX / eta);
		exp_terms[4*i+1] = exp((-1)*curX / eta);
		exp_terms[4*i+2] = exp(curY / eta);
		exp_terms[4*i+3] = exp((-1)*curY / eta);
	}

	//calculate Log-sum-Exp's function
	for(int i = 0; i <  num_nets; i++){
		double x_pos = 0, x_neg = 0, y_pos = 0, y_neg = 0;
		for(unsigned j = 0; j < _placement.net(i).numPins(); j++){
			int mod_idx =_placement.net(i).pin(j).moduleId();
			x_pos += exp_terms[4*mod_idx];
			x_neg += exp_terms[4*mod_idx+1];
			y_pos += exp_terms[4*mod_idx+2];
			y_neg += exp_terms[4*mod_idx+3];
		}
		f += (log(x_pos) + log(x_neg) + log(y_pos) + log(y_neg));

	//calculate Log-sum-Exp's gradient
		for(unsigned j = 0; j < _placement.net(i).numPins(); j++){
			auto cur_module = _placement.module(i);
			int mod_idx =_placement.net(i).pin(j).moduleId();
			if(cur_module.isFixed() == 1){
				g[2*mod_idx] = 0;
				g[2*mod_idx + 1] = 0;
			}else{
				g[2*mod_idx] += exp_terms[4 * mod_idx] / (x_pos * eta);
				g[2*mod_idx] -= exp_terms[4 * mod_idx + 1] / (x_neg * eta);
				g[2*mod_idx + 1] += exp_terms[4 * mod_idx + 2] / (y_pos* eta);
				g[2*mod_idx + 1] -= exp_terms[4 * mod_idx + 3] / (y_neg* eta);
			}
		}	
	}

	//if beta == 0 then skip bin destiny part
	if(beta == 0)	return;
	/********************************************************/
	/********************bin destiny*************************/
	/********************************************************/
	double a_x, b_x, a_y, b_y, dx, dy, den_ratio, theta_x, theta_y;

	
	fill(cur_g.begin(), cur_g.end(), 0);
	fill(Db.begin(), Db.end(), 0);

	for (int bin_x = 0; bin_x < 14; bin_x++){
		for (int bin_y = 0; bin_y < 14; bin_y++){
			for (int i = 0; i < num_modules; i++){
				auto cur_module = _placement.module(i);

				a_x = 4 / ((bin_width + cur_module.width()) * (2 * bin_width + cur_module.width()));
				b_x  = 4 / (bin_width * (2 * bin_width + cur_module.width()));
				a_y = 4 / ((bin_height + cur_module.height()) * (2 * bin_height + cur_module.height()));
				b_y  = 4 / (bin_height * (2 * bin_height + cur_module.height()));
				
				dx = x[2*i] - ((bin_x+0.5)*bin_width + _placement.boundryLeft());
				dy = x[2*i+1] - ((bin_y+0.5)*bin_height + _placement.boundryBottom());
				den_ratio = cur_module.area()/(bin_width * bin_height);
				
				//Bell-shaped function
				if (0 <= abs(dx) && abs(dx) <= (bin_width / 2 + cur_module.width() / 2)){
					theta_x = 1 - a_x * abs(dx) * abs(dx);
				}else if(abs(dx) <= (bin_width + cur_module.width() / 2)){
					theta_x = b_x * (abs(dx) - (bin_width + cur_module.width() / 2)) * (abs(dx) - (bin_width + cur_module.width() / 2));
				}else{
					theta_x = 0;
				}

				if (abs(dy) <= (bin_height / 2 + cur_module.height() / 2)){
					theta_y = 1 - a_y * abs(dy) * abs(dy);
				}else if(abs(dy) <= (bin_height + cur_module.height() / 2)){
					theta_y = b_y * (abs(dy) - (bin_height + cur_module.height() / 2)) * (abs(dy) - (bin_height + cur_module.height() / 2));
				}else{
					theta_y = 0;
				}
				
				//Bell-shaped function's gradient
				if(cur_module.isFixed() != 1){
					if(abs(dx) <= (bin_width / 2 + cur_module.width() / 2)){
						cur_g[2*i] = den_ratio * ((-2) * a_x * dx) * theta_y;
					}else if(abs(dx) <= (bin_width + cur_module.width() / 2)){
						if(dx > 0){
							cur_g[2*i] = den_ratio * 2 * b_x * (dx - (bin_width + cur_module.width() / 2)) * theta_y;
						}else{
							cur_g[2*i] = den_ratio * 2 * b_x * (dx + (bin_width + cur_module.width() / 2)) * theta_y;
						}
					}else{
						cur_g[2 * i] = 0;
					}					
			
					if(abs(dy) <= (bin_height/2 + cur_module.height() / 2)){
						cur_g[2 * i + 1] = den_ratio * ((-2) * a_y * dy) * theta_x;
					}else if (abs(dy) <= (bin_height + cur_module.height() / 2)) {
						if(dy > 0){
							cur_g[2*i+1] = den_ratio * 2 * b_y * (dy - (bin_height + cur_module.height() / 2)) * theta_x;
						}else{
							cur_g[2*i+1] = den_ratio * 2 * b_y * (dy + (bin_height + cur_module.height() / 2)) * theta_x;
						}
					}else{
						cur_g[2*i+1] = 0;
					}
				}
				Db[14*bin_y + bin_x] += den_ratio * theta_x * theta_y;
					
			}
			f += beta * (Db[14 * bin_y + bin_x] - Tb) * (Db[14 * bin_y + bin_x] - Tb);
			
			for(int j = 0; j < num_modules; j++){
				g[2*j ] += beta * 2 * (Db[14 * bin_y + bin_x] - Tb) * cur_g[2*j];
				g[2*j+1] += beta * 2 * (Db[14 * bin_y + bin_x] - Tb) * cur_g[2*j+1];
			}
		}
	}
	
}

void ExampleFunction::evaluateF(const vector<double> &x, double &f){

	//reset object function
	f = 0;

	/********************************************************/
	/********************Log-sum-Exp*************************/
	/********************************************************/

	//calculate four exp_terms for every modules
	for(int i = 0; i < num_modules; i++){
		double curX,curY;
		if (_placement.module(i).isFixed()){
			curX = _placement.module(i).centerX();
			curY = _placement.module(i).centerY();
		}else{
			curX =  x[2*i];
			curY =	x[2*i+1];
		}
		exp_terms[4*i]   = exp(curX / eta);
		exp_terms[4*i+1] = exp((-1)*curX / eta);
		exp_terms[4*i+2] = exp(curY / eta);
		exp_terms[4*i+3] = exp((-1)*curY / eta);
	}
	//calculate Log-sum-Exp's function
	for(int i = 0; i <  num_nets; i++){
		double x_pos = 0, x_neg = 0, y_pos = 0, y_neg = 0;
		for(unsigned j = 0; j < _placement.net(i).numPins(); j++){
			int mod_idx =_placement.net(i).pin(j).moduleId();
			x_pos += exp_terms[4*mod_idx];
			x_neg += exp_terms[4*mod_idx+1];
			y_pos += exp_terms[4*mod_idx+2];
			y_neg += exp_terms[4*mod_idx+3];
		}
		f += (log(x_pos) + log(x_neg) + log(y_pos) + log(y_neg));
		
	}

	//if beta == 0 then skip bin destiny part
	if(beta == 0)	return;
	/********************************************************/
	/********************bin destiny*************************/
	/********************************************************/
	double a_x, b_x, a_y, b_y, dx, dy, den_ratio, theta_x, theta_y;


	fill(Db.begin(), Db.end(), 0);
	
	for (int bin_x = 0; bin_x < 14; bin_x++){
		for (int bin_y = 0; bin_y < 14; bin_y++){
			for (int i = 0; i < num_modules; i++){
				auto cur_module = _placement.module(i);
				// if(cur_module.isFixed() == 1)
				// 	continue;

				a_x = 4/((bin_width + cur_module.width()) * (2 * bin_width + cur_module.width()));
				a_y = 4/((bin_height + cur_module.height()) * (2 * bin_height + cur_module.height()));
				b_x  = 4/(bin_width * (2 * bin_width + cur_module.width()));
				b_y  = 4/(bin_height * (2 * bin_height + cur_module.height()));
				
				dx = x[2*i] - ((bin_x+0.5)*bin_width + _placement.boundryLeft());
				dy = x[2*i+1] - ((bin_y+0.5)*bin_height + _placement.boundryBottom());
				den_ratio = cur_module.area()/(bin_width * bin_height);
			
				//Bell-shaped function
				if (0 <= abs(dx) && abs(dx) <= (bin_width / 2 + cur_module.width() / 2)){
					theta_x = 1 - a_x * abs(dx) * abs(dx);
				}else if(abs(dx) <= (bin_width + cur_module.width() / 2)){
					theta_x = b_x * (abs(dx) - (bin_width + cur_module.width() / 2)) * (abs(dx) - (bin_width + cur_module.width() / 2));
				}else{
					theta_x = 0;
				}

				if (abs(dy) <= (bin_height / 2 + cur_module.height() / 2)){
					theta_y = 1 - a_y * abs(dy) * abs(dy);
				}else if(abs(dy) <= (bin_height + cur_module.height() / 2)){
					theta_y = b_y * (abs(dy) - (bin_height + cur_module.height() / 2)) * (abs(dy) - (bin_height + cur_module.height() / 2));
				}else{
					theta_y = 0;
				}
				Db[14*bin_y + bin_x] += den_ratio * theta_x * theta_y;
						
			}
			f += beta * (Db[14 * bin_y + bin_x] - Tb) * (Db[14 * bin_y + bin_x] - Tb);
		}
	}
		
}

unsigned ExampleFunction::dimension()
{
    return _placement.numModules() * 2; // num_blocks*2 
    // each two dimension represent the X and Y dimensions of each block
}
