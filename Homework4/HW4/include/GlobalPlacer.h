#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "Wrapper.hpp"
#include "NumericalOptimizer.h"
#include <vector>

using namespace std;

class GlobalPlacer 
{
public:
    GlobalPlacer(wrapper::Placement &placement);

    void randomPlace(vector<double> &vec);
	void place();
    void SolveOutBoundary(NumericalOptimizer &no, vector<double> &x);

private:
    wrapper::Placement& _placement;
};

#endif // GLOBALPLACER_H
