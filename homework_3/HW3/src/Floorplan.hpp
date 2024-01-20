#ifndef FLOORPLAN_HPP
#define FLOORPLAN_HPP

#include "total.hpp"

using namespace std;


class Floorplan {
public:
    Floorplan(Input*);
    Input* input_info;
    // BsTree* bstree, * fm_bstree, * sm_bstree;
    // Contour* contour;

    deque<Module*> planned_modules;
    deque<SoftModule*> sm_deque;
    deque<FixedModule*> fm_deque;
    
    string output_file;

    long long wirelength;

    // bool isOverlap(Module* m, long long cur_x, long long cur_y);
    // bool isOverlap(Module* m, long long cur_x, long long cur_y, pair<long long, long long> possible_hw_pair);

    
    void init_deque();
    void initFloorplan();
    void SA(double initialT, double frozenT, int k);
    unordered_map<string, Module*> perturb(unordered_map<string, Module*> cur_sm);
    bool isAccept(double T, long long delta_c);
    void reduce(double& T);

    void run(int argc, char* argv[]);

    long long calWireLength(unordered_map<string, Module*> &sm);

    void outputFloorplan(unordered_map<string, Module*> &all_modules);

    bool is_overlap(SoftModule* sm, Module* fm);
    bool is_overBound_x(Module* m);
    bool is_overBound_y(Module* m);

    clock_t main_start, initFloorplan_start, SA_start;
    double total_time, input_time, initFloorplan_time, SA_time, max_SA_time;
    void set_max_SA_time(double set_time);
    void runtime_report();
    double cal_time(clock_t start);
};

#endif