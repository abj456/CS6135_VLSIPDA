#include <bits/stdc++.h>
// #include <fstream>
// #include "input.hpp"
#include "Partitioning.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    // prevent insufficient args
    if (argc < 3) {
        cout << "The number of arg is not enough!!" << endl;
        return 0;
    }

    int times = 1;

    for (int i = 0; i < times; i++) {
        clock_t start = clock();

        Input* p_input = new Input();
        p_input->loadInput(argc, argv);
        p_input->input_time = (double)(clock() - start) / CLOCKS_PER_SEC;

        FM_Algo fm(p_input);
        fm.main_start = start;
        if (argc > 3) {
            fm.check_filename = argv[4];
        }
        fm.run();
        fm.runtime_report();

        fm.outputResult(argv[2]);
    }

    return 0;
}