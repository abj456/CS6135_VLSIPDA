#include <bits/stdc++.h>
// #include "input.hpp"
// #include "Floorplan.hpp"
#include "total.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    clock_t start = clock();

    Input* p_input = new Input();
    p_input->loadInput(argc, argv);
    p_input->input_time = (double)(clock() - start) / CLOCKS_PER_SEC;
    // p_input->checkInput();

    Floorplan floorplan(p_input);
    floorplan.main_start = start, floorplan.input_time = p_input->input_time;
    floorplan.run(argc, argv);
    floorplan.runtime_report();
    

    return 0;
}