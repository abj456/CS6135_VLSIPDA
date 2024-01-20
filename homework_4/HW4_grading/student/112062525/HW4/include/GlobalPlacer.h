#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "Wrapper.hpp"
#include <bits/stdc++.h>
using namespace std;

class GlobalPlacer
{
public:
    GlobalPlacer(wrapper::Placement &placement);

    // void randomPlace(vector<double> &x); // An example of random placement implemented by TA
    void randomPlace(); // An example of random placement implemented by TA
    void place();
    void net_place(unsigned seed);
    void findBestInitPlace(int try_times);

    // isPlaced
    vector<bool> module_isPlaced;
    

    // void plotPlacementResult(const string outfilename, bool isPrompt = false);
    // void plotBoxPLT(ofstream &stream, double x1, double y1, double x2, double y2);


private:
    wrapper::Placement &_placement;
    double bin_max_area = 0.0, bound_width = 0.0, bound_height = 0.0, bin_width = 0.0, bin_height = 0.0;
    unsigned nets_num = 0, bin_cut = 20;
};

#endif // GLOBALPLACER_H
