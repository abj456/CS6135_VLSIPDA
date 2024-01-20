#ifndef EXAMPLEFUNCTION_H
#define EXAMPLEFUNCTION_H

#include "Wrapper.hpp"
#include "NumericalOptimizerInterface.h"
#include "GlobalPlacer.h"

// class Bin {
// public:    
//     double x_center;
//     double y_center;
// };

class ExampleFunction : public NumericalOptimizerInterface
{
public:
    ExampleFunction(wrapper::Placement& placement, GlobalPlacer& globalplacer);
    wrapper::Placement& _placement;
    GlobalPlacer& _globalplacer;

    void evaluateFG(const vector<double>& x, double& f, vector<double>& g);
    void evaluateF(const vector<double>& x, double& f);
    unsigned dimension();
    void increase_lambda(int inc);

private:
    double calculateLSE_F(const vector<double>& x);
    double calculateBinDensity_F(const vector<double>& x);

    double calculateLSE_FG(const vector<double>& x, vector<double>& g);
    double calculateBinDensity_FG(const vector<double>& x, vector<double>& g);

    unsigned num_modules = 0;
    int lambda = 0;
    unsigned bin_cut = 0;
    unsigned bin_total_num = 0;
    double eta = 0.0, bound_width = 0.0, bound_height = 0.0;
    double core_area = 0.0;
    double bin_width = 0.0, bin_height = 0.0, bin_area = 0.0, avg_density = 0.0;
    vector<double> grad, x_exp;
};

#endif // EXAMPLEFUNCTION_H
