// #include <bits/stdc++.h>
#include "total.hpp"

// #include "input.hpp"

using namespace std;

Input::Input() {}

void Input::loadInput(int argc, char* argv[]) {
    ifstream inputfile(argv[1]);

    loadChipInfo(inputfile);

    loadSoftModules(inputfile);

    loadFixedModules(inputfile);

    loadNets(inputfile);
}

void Input::loadNets(ifstream& inputfile) {
    string Type;
    int NumOfNets;

    inputfile >> Type;
    if (Type.compare("NumNets") == 0) {
        inputfile >> NumOfNets;

        for (int i = 0; i < NumOfNets; i++) {
            string type_net, module_name1, module_name2;
            long long net_weight;
            inputfile >> type_net >> module_name1 >> module_name2 >> net_weight;

            if (type_net.compare("Net") == 0) {
                Net* n = new Net(2, net_weight);
                if (modules[module_name1]->module_type.compare("SoftModule") == 0) {
                    // n->module_info[module_name1] = "SoftModule";
                    n->connected_modules[module_name1] = soft_modules[module_name1];
                    soft_modules[module_name1]->nets.push_back(n);
                }
                else {
                    // n->module_info[module_name1] = "FixedModule";
                    n->connected_modules[module_name1] = fixed_modules[module_name1];
                    fixed_modules[module_name1]->nets.push_back(n);
                }
                if (soft_modules.count(module_name2)) {
                    // n->module_info[module_name2] = "SoftModule";
                    n->connected_modules[module_name2] = soft_modules[module_name2];
                    soft_modules[module_name2]->nets.push_back(n);
                }
                else {
                    // n->module_info[module_name2] = "FixedModule";
                    n->connected_modules[module_name2] = fixed_modules[module_name2];
                    fixed_modules[module_name2]->nets.push_back(n);
                }
                nets.push_back(n);
            }
        }
    }
}

void Input::loadFixedModules(ifstream& inputfile) {
    string Type;
    int NumOfFixedModules;

    inputfile >> Type;
    if (Type.compare("NumFixedModules") == 0) {
        inputfile >> NumOfFixedModules;

        for (int i = 0; i < NumOfFixedModules; i++) {
            string type_fm, fm_name;
            long long fm_x, fm_y, fm_w, fm_h;
            inputfile >> type_fm >> fm_name >> fm_x >> fm_y >> fm_w >> fm_h;

            if (type_fm.compare("FixedModule") == 0) {
                fixed_modules[fm_name] = new FixedModule(fm_name, fm_x, fm_y, fm_w, fm_h);
                module_type[fm_name] = "FixedModule";
                modules[fm_name] = fixed_modules[fm_name];
            }
        }
    }
    
}

void Input::loadSoftModules(ifstream& inputfile) {
    string Type;
    int NumOfSoftModules;

    inputfile >> Type;
    if (Type.compare("NumSoftModules") == 0) {
        inputfile >> NumOfSoftModules;

        for (int i = 0; i < NumOfSoftModules; i++) {
            string type_sm, sm_name;
            long long sm_min_area;
            inputfile >> type_sm >> sm_name >> sm_min_area;

            if (type_sm.compare("SoftModule") == 0) {
                soft_modules[sm_name] = new SoftModule(sm_name, sm_min_area);
                module_type[sm_name] = "SoftModule";
                modules[sm_name] = soft_modules[sm_name];
                soft_modules[sm_name]->findAllShape();
            }
            soft_modules[sm_name]->tried_hw.assign(soft_modules[sm_name]->possible_h_and_w.size(), false);
            // soft_modules[sm_name]->rotate_possible_hw.assign(soft_modules[sm_name]->possible_h_and_w.size(), false);
            // soft_modules[sm_name]->wasRotated_possible_hw.assign(soft_modules[sm_name]->possible_h_and_w.size(), false);
        }
    }
}

void Input::loadChipInfo(ifstream& inputfile) {
    string Type;
    long long width, height;

    inputfile >> Type;
    if (Type.compare("ChipSize") == 0) {
        inputfile >> width >> height;
        chip = new Chip(width, height);
    }
}

void Input::checkInput() {
    cout << "----Check ChipSize----" << endl;
    cout << "width = " << chip->width << ", height = " << chip->height << endl;

    cout << "----Check SoftModule----" << endl;
    cout << "NumSoftModules = " << soft_modules.size() << endl;
    for (auto sm : soft_modules) {
        cout << "SoftModule " << sm.second->name << " " << sm.second->min_area << endl;
        // sm.second->findAllShape();
    }

    cout << "----Check FixedModule----" << endl;
    for (auto fm : fixed_modules) {
        cout << "FixedModule " << fm.second->name << " " << fm.second->x_coor << ' ' << fm.second->y_coor
            << ' ' << fm.second->width << ' ' << fm.second->height << endl;
    }

    cout << "----Check Nets----" << endl;
    for (auto n : nets) {
        cout << "Net ";
        for (auto m : n->connected_modules) {
            cout << m.second->name << ' ';
        }
        // for (auto m: n->module_info) {
        //     cout << m.first << " ";
        // }
        cout << n->net_weight << endl;
    }

}

Net::Net(int num_modules, long long n_weight) : num_modules(num_modules), net_weight(n_weight) {}

Module::Module() {}

FixedModule::FixedModule(string fm_name, long long x, long long y, long long w, long long h) {
    this->name = fm_name;
    this->x_coor = x;
    this->y_coor = y;
    this->width = w;
    this->height = h;
    this->module_type = "FixedModule";
}

void SoftModule::findAllShape() {
    // double x;
    long long x;
    long long x_min;
    x = round(sqrt(this->min_area));
    x_min = x / sqrt(2);
    while (true) {
        for (long long i = x; i >= x_min; i--) {
            if (this->min_area % i == 0) {
                // cout << "TryShape: possible h = " << i << endl;
                long long poss_h = i, poss_w = min_area / i;
                this->possible_h_and_w.push_back(make_pair(poss_h, poss_w));
            }
        }
        if (this->possible_h_and_w.empty()) {
            // cout << "possible h is NOT Found" << endl;
            this->adjustMinArea();
        }
        else break;
    }

    int hw_pair_vectorSize = possible_h_and_w.size();
    for (int i = 0; i < hw_pair_vectorSize; i++) {
        possible_h_and_w.push_back(make_pair(possible_h_and_w[i].second, possible_h_and_w[i].first));
    }
    sort(possible_h_and_w.begin(), possible_h_and_w.end(), [](pair<long long, long long> a, pair<long long, long long> b){return a.first < b.first;});
}

void SoftModule::adjustMinArea() {
    int num_of_ten = 0;
    long long tmp_min_area = this->min_area;
    while (tmp_min_area % 10 == 0) {
        tmp_min_area /= 10;
        num_of_ten++;
    }
    tmp_min_area++;
    while (num_of_ten) {
        tmp_min_area *= 10;
        num_of_ten--;
    }
    this->min_area = tmp_min_area;
}

void SoftModule::tryShape() {
    for (int i = 0; i < tried_hw.size(); i++) {
        if (!tried_hw[i]) {
            height = possible_h_and_w[i].first;
            width = possible_h_and_w[i].second;
            admis_hw_idx = i;
            // cout << "Debug: in tryShape(), i = " << i << ", h = " << height << ", w = " << width << endl;
            break;
        }
    }
}

void SoftModule::retryShape() {
    // cout << "Debug: start retryShape(), size = " << tried_hw.size() << endl;
    tried_hw[admis_hw_idx] = true;
    for (int i = 0; i < tried_hw.size(); i++) {
        // cout << "Debug: in retryShape() for loop, i = " << i << ", tried_hw[i] = " << tried_hw[i] << endl;
        if (!tried_hw[i]) {
            height = possible_h_and_w[i].first;
            width = possible_h_and_w[i].second;
            admis_hw_idx = i;
            // cout << "Debug: in retryShape(), i = " << i << ", h = " << height << ", w = " << width << endl;
            break;
        }
    }
}

void SoftModule::resetPossibleShape() {
    tried_hw.assign(tried_hw.size(), false);
}

SoftModule::SoftModule(string sm_name, long long min_area) {
    this->name = sm_name;
    this->min_area = min_area;
    this->module_type = "SoftModule";

    this->min_w = round(sqrt(this->min_area / RATIO_Bi));
    this->max_w = round(sqrt(this->min_area / RATIO_Ai));
    this->min_h = round(sqrt(this->min_area * RATIO_Ai));
    this->max_h = round(sqrt(this->min_area * RATIO_Bi));

    this->x_coor = 0;
    this->y_coor = 0;
}

SoftModule::SoftModule(SoftModule* original_sm) {
    // cout << "Debug: soft module copy start" << endl;
    name = original_sm->name;
    min_area = original_sm->min_area;
    module_type = original_sm->module_type;

    min_w = original_sm->min_w;
    max_w = original_sm->max_w;
    min_h = original_sm->min_h;
    max_h = original_sm->max_h;

    x_coor = original_sm->x_coor;
    y_coor = original_sm->y_coor;

    width = original_sm->width;
    height = original_sm->height;

    possible_h_and_w = original_sm->possible_h_and_w;
    tried_hw = original_sm->tried_hw;
    // cout << "Debug: soft module copy is done" << endl;
}

Chip::Chip(long long w, long long h) : width(w), height(h) {
    size = width * height;
}