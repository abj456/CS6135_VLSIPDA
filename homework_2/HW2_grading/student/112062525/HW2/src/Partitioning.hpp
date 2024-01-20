#include <bits/stdc++.h>
#include "input.hpp"

using namespace std;


class FM_Algo {
public:
    Input* input;
    string check_filename;

    int maxPartialSum;
    int steps_for_maxPartialSum;
    int partition_times;
    vector<int> maxGains;
    vector<Cell*>selectedBaseCells;

    FM_Algo(Input* input);
    void run();
    void partition();
    void loopPartition();

    /*----initialization----*/
    void init();
    void initPartition();
    void initNetDistribution();
    void initCellGains();
    void initBucketLists();

    // after getting base cell, update all the others' gains
    void updateCellGains(Cell* bc);
    // OK
    void updateDieSizes(string FromDie, string ToDie, Cell* cell);
    // OK
    Cell* getBaseCell();
    // OK
    Cell* getBaseCellfromDie(string dieName);

    bool isBalanced_debugFlag = true;
    // OK
    bool isBalanced(long long A, long long B);

    // OK
    void calculateMaxPartialSum();
    // OK
    int calculateCutSize();
    void calculateCellGains();

    // OK
    void unlock_all_cells();
    // OK
    void cancel_moving_selectedBaseCell_after(int steps);

    /*----debug function----*/
    void check_after_loopPart();
    void checkPartition();
    void checkInitialization();

    /*----output function----*/
    void outputResult(string filename);

    /*----runtime info----*/
    clock_t main_start, input_start, fm_start;
    double total_time, input_time, fm_time, max_fm_time;
    void set_max_fmTime(double setting);
    void runtime_report();
};
