#ifndef INPUT_HPP
#define INPUT_HPP

// #include <bits/stdc++.h>
#include "total.hpp"

using namespace std;

class Chip;
class SoftModule;
class FixedModule;
class Module;
class Net;

class Input {
public:
    Chip* chip;
    unordered_map<string, string> module_type;
    unordered_map<string, SoftModule*> soft_modules;
    unordered_map<string, FixedModule*> fixed_modules;
    unordered_map<string, Module*> modules;
    vector<Net*> nets;

    Input();
    void loadInput(int argc, char* argv[]);
    void loadChipInfo(ifstream& inputfile);
    void loadSoftModules(ifstream& inputfile);
    void loadFixedModules(ifstream& inputfile);
    void loadNets(ifstream& inputfile);

    void checkInput();

    double input_time;
};

class Net {
public:
    int num_modules;
    long long net_weight;
    // unordered_map<string, string> module_info;
    unordered_map<string, Module*> connected_modules;
    Net(int num_modules, long long n_weight);
};

class Module {
public: 
    string name;
    string module_type;
    long long x_coor, y_coor;
    long long width, height;
    vector<Net*> nets;
    Module();

    unordered_map<string, Module*> overlapped_fm;
};

class FixedModule : public Module{
public:
    FixedModule(string name, long long x, long long y, long long w, long long h);
    
    string pos_attribute;
    
};

class SoftModule : public Module{
public:
    // string name;
    long long min_area;
    // long long x_coor, y_coor;
    // long long width, height;
    long long min_w, min_h, max_w, max_h;
    
    int admis_hw_idx;
    

    SoftModule(string name, long long min_area);
    SoftModule(SoftModule* original_sm);
    
    void adjustMinArea();
    void findAllShape();
    void tryShape();
    void retryShape();
    void resetPossibleShape();
    vector<pair<long long, long long>> possible_h_and_w;
    vector<bool> tried_hw;

};

class Chip {
public:
    long long width, height;
    long long size;
    Chip(long long w, long long h);
};

#endif