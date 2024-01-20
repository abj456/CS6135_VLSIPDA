#include "total.hpp"
// #include "Floorplan.hpp"

using namespace std;

Floorplan::Floorplan(Input* p_input) {
    input_info = p_input;
    set_max_SA_time(595.0);
}

void Floorplan::run(int argc, char* argv[]) {
    // cout << "Debug: ready to start initFloorplan()" << endl;
    output_file = argv[2];

    initFloorplan();
    SA(50000.0, 15.0, 40);
    calWireLength(input_info->modules);
    outputFloorplan(input_info->modules);
}


bool Floorplan::is_overlap(SoftModule* sm, Module* fm) {
    bool Mi_to_left_of_Mj = sm->x_coor + sm->width <= fm->x_coor;
    bool Mi_below_Mj = sm->y_coor + sm->height <= fm->y_coor;
    bool Mi_to_right_of_Mj = sm->x_coor >= fm->x_coor + fm->width;
    bool Mi_above_Mj = sm->y_coor >= fm->y_coor + fm->height;

    if (Mi_to_left_of_Mj || Mi_below_Mj || Mi_to_right_of_Mj || Mi_above_Mj) {
        return false;
    }
    return true;
}

bool Floorplan::is_overBound_x(Module* m) {
    long long x = m->x_coor, w = m->width;
    return x + w >= input_info->chip->width || x < 0;
}

bool Floorplan::is_overBound_y(Module* m) {
    long long y = m->y_coor, h = m->height;
    return y + h >= input_info->chip->height || y < 0;
}

void Floorplan::init_deque() {
    planned_modules.clear();
    fm_deque.clear();
    sm_deque.clear();
    for (auto fm : input_info->fixed_modules) {
        planned_modules.push_back(fm.second);
        fm_deque.push_back(fm.second);
    }
    for (auto sm : input_info->soft_modules) {
        sm_deque.push_back(sm.second);
    }
    sort(sm_deque.begin(), sm_deque.end(), [](const SoftModule* a, const SoftModule* b) {
        return a->min_area > b->min_area;
        });
    sort(fm_deque.begin(), fm_deque.end(), [](const FixedModule* a, const FixedModule* b) {
        return a->x_coor < b->x_coor;
        });
}

void Floorplan::initFloorplan() {
    initFloorplan_start = clock();

    init_deque();
    // cout << "Debug: init_deque() has NO bug" << endl;

    bool incomplete = false;
    do {
        incomplete = false;
        for (auto sm : sm_deque) {
            // SoftModule* sm = (SoftModule*)sm_m->_module;
            // cout << "Debug: cur sm is " << sm->name << endl;
            sm->tryShape();
            sm->height = sm->possible_h_and_w[sm->admis_hw_idx].first;
            sm->width = sm->possible_h_and_w[sm->admis_hw_idx].second;
            bool sm_overlap_or_overbound = false;
            do {
                incomplete = sm_overlap_or_overbound = false;
                for (auto fm : planned_modules) {
                    // FixedModule* fm = (FixedModule*)fm_m->_module;
                    if (is_overlap(sm, fm)) {
                        sm_overlap_or_overbound = true;
                        sm->x_coor++;
                        break;
                    }
                    else if (is_overBound_x((Module*)sm)) {
                        sm_overlap_or_overbound = true;
                        sm->y_coor++;
                        sm->x_coor = 0;
                        break;
                    }
                    else if (is_overBound_y((Module*)sm)) {
                        sm_overlap_or_overbound = true;
                        sm->x_coor = sm->y_coor = 0;
                        sm->retryShape();
                        sm->height = sm->possible_h_and_w[sm->admis_hw_idx].first;
                        sm->width = sm->possible_h_and_w[sm->admis_hw_idx].second;
                        // exit(0);
                        break;
                    }
                }
                // double apsect_ratio = ((double)sm->height / sm->width);
                // if (apsect_ratio <= RATIO_Ai || apsect_ratio >= RATIO_Bi) {
                //     incomplete = true;
                // }
                if (sm->tried_hw.back() == true) {
                    incomplete = true;
                }
            } while (!incomplete && sm_overlap_or_overbound);

            if (!incomplete) {
                planned_modules.push_back(sm);
            }
            else {
                sm->resetPossibleShape();
                init_deque();
                break;
            }
        }

        // wirelength = calWireLength(input_info->modules);
        // outputFloorplan(input_info->modules);
        // cout << "Debug: wirelength = " << wirelength << endl;

    } while (incomplete);

    initFloorplan_time = cal_time(initFloorplan_start);
}

long long Floorplan::calWireLength(unordered_map<string, Module*>& all_modules) {
    long long total_wirelength = 0;
    for (auto n : input_info->nets) {
        long long min_x{ INT_MAX }, max_x{ 0 }, min_y{ INT_MAX }, max_y{ 0 };
        long long x{ 0 }, y{ 0 };
        for (auto m : n->connected_modules) {
            // x = bstree->getNode(m.first)->_module->x_coor + bstree->getNode(m.first)->_module->width / 2;
            // y = bstree->getNode(m.first)->_module->y_coor + bstree->getNode(m.first)->_module->height / 2;
            // cout << "Debug: module = " << m.first << ", x_coor = " << bstree->getNode(m.first)->_module->x_coor << ", y = " << bstree->getNode(m.first)->_module->y_coor << endl;
            x = all_modules[m.first]->x_coor + all_modules[m.first]->width / 2;
            y = all_modules[m.first]->y_coor + all_modules[m.first]->height / 2;

            min_x = min(min_x, x);
            max_x = max(max_x, x);
            min_y = min(min_y, y);
            max_y = max(max_y, y);
            // cout << "Debug: max_x = " << max_x << ", min_x = " << min_x << ", max_y = " << max_y << ", min_y = " << min_y << endl;
        }
        total_wirelength += ((max_x - min_x) + (max_y - min_y)) * n->net_weight;
    }
    this->wirelength = total_wirelength;
    return total_wirelength;
}

void Floorplan::outputFloorplan(unordered_map<string, Module*>& all_modules) {
    ofstream fout(output_file);
    fout << "Wirelength " << wirelength << endl << endl;

    fout << "NumSoftModules " << input_info->soft_modules.size() << endl;

    for (auto sm : all_modules) {
        if (sm.second->module_type.compare("SoftModule") == 0) {
            fout << sm.second->name << ' ' << sm.second->x_coor << ' ' << sm.second->y_coor << ' ' << sm.second->width << ' ' << sm.second->height << endl;
        }
    }

}

void Floorplan::SA(double initialT, double frozenT, int k) {
    SA_start = clock();

    unordered_map<string, Module*> cur_sm, next_sm, best_sm;
    cur_sm = best_sm = input_info->modules;

    double T = initialT;
    long long MT = 0, up_hill = 0, reject = 0, delta_cost = 0;
    int N = k * input_info->soft_modules.size();
    do {
        MT = 0, up_hill = 0, reject = 0, delta_cost = 0;
        // cout << "Debug: a new perturb start, T = " << T << ", frozenT = " << frozenT << endl;
        do {
            next_sm = perturb(cur_sm);

            MT++;
            delta_cost = calWireLength(next_sm) - calWireLength(cur_sm);

            if (delta_cost <= 0 || isAccept(T, delta_cost)) {
                // cout << "Debug: DO find a better cost" << endl;
                if (delta_cost > 0) up_hill++;
                cur_sm = next_sm;
                if (calWireLength(cur_sm) < calWireLength(best_sm)) {
                    best_sm = cur_sm;
                }
            }
            else {
                reject++;
                // cur_sm = next_sm;
            }
        } while (up_hill < N && MT < 2 * N);
        cout << "Debug: N = " << N << ", up_hill = " << up_hill << ", MT = " << MT << ", reject = " << reject << ", best WL = " << calWireLength(best_sm) << ", temperature = " << T << endl;
        // reduce(T);
        if (T > frozenT) reduce(T);
    } 
    while ((cal_time(SA_start) + initFloorplan_time < max_SA_time));
    // while (T > frozenT && (cal_time(SA_start) + initFloorplan_time < max_SA_time) && ((double)reject / MT) < 0.95);

    input_info->modules = best_sm;
    // calWireLength(input_info->modules);
    // outputFloorplan(input_info->modules);

    SA_time = cal_time(SA_start);
}

unordered_map<string, Module*> Floorplan::perturb(unordered_map<string, Module*> cur_sm) {
    unordered_map<string, Module*> return_sm = cur_sm;

    // int op_num = (rand() % 4) + 1, random_sm_idx = rand() % return_sm.size();

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> int_distribution(1, 4);

    // pick a operation
    int op_num = int_distribution(gen);
    // pick a module to do the picked operation
    uniform_int_distribution<int> rand_module(0, return_sm.size() - 1);
    unordered_map<string, Module*>::iterator it;
    do {
        it = return_sm.begin();
        int random_sm_idx = rand_module(gen);
        advance(it, random_sm_idx);
    } while (it->second->module_type.compare("SoftModule") != 0);

    // move
    if (op_num == 1) {
        // cout << "Debug: this round of perturb is move, sm is " << it->second->name << endl;
        // int x_possible_steps = input_info->chip->width * 0.005
        int x_possible_steps = 10;
        int x_step = rand() % x_possible_steps;
        int y_step = rand() % x_possible_steps;
        int direction = rand() % 8;

        auto sm = it->second;
        int old_x = sm->x_coor, new_x = sm->x_coor;
        int old_y = sm->y_coor, new_y = sm->y_coor;

        // move left
        if (direction == 0) {
            new_x -= x_step;
        }
        // move right
        else if (direction == 1) {
            new_x += x_step;
        }
        // move down
        else if (direction == 2) {
            new_y -= y_step;
        }
        // move up
        else if (direction == 3) {
            new_y += y_step;
        }
        // move left-up
        else if (direction == 4) {
            new_x -= x_step;
            new_y += y_step;
        }
        // move left-down
        else if (direction == 5) {
            new_x -= x_step;
            new_y -= y_step;
        }
        // move right-up
        else if (direction == 6) {
            new_x += x_step;
            new_y += y_step;
        }
        //move right-down
        else if (direction == 7) {
            new_x += x_step;
            new_y -= y_step;
        }

        // cout << "Debug: checkpoint of moving perturbation" << endl;
        SoftModule* new_sm = new SoftModule((SoftModule*)sm);
        new_sm->x_coor = new_x;
        new_sm->y_coor = new_y;


        bool overlap_or_overbound = false;
        // sm->x_coor = new_x, sm->y_coor = new_y;
        for (auto& fm : return_sm) {
            if (new_sm->name != fm.second->name && (is_overlap(new_sm, fm.second) || is_overBound_x(new_sm) || is_overBound_y(new_sm))) {
                overlap_or_overbound = true;
                break;
            }
        }
        if (!overlap_or_overbound) {
            return_sm[sm->name] = new_sm;
        }
    }
    // change position
    else if (op_num == 2) {
        int new_x = rand() % input_info->chip->width;
        int new_y = rand() % input_info->chip->height;

        auto sm = it->second;
        int old_x = sm->x_coor;
        int old_y = sm->y_coor;

        SoftModule* new_sm = new SoftModule((SoftModule*)sm);
        new_sm->x_coor = new_x, new_sm->y_coor = new_y;

        bool overlap_or_overbound = false;
        for (auto& fm : return_sm) {
            if (new_sm->name != fm.second->name && (is_overlap(new_sm, fm.second) || is_overBound_x(new_sm) || is_overBound_y(new_sm))) {
                overlap_or_overbound = true;
                break;
            }
        }
        if (!overlap_or_overbound) {
            return_sm[sm->name] = new_sm;
        }
    }
    // swap
    else if (op_num == 3) {
        // cout << "Debug: this round of perturb is swap" << endl;
        // first pick another sm to swap
        unordered_map<string, Module*>::iterator it2;
        do {
            it2 = return_sm.begin();
            int random_sm_idx2 = rand_module(gen);
            advance(it2, random_sm_idx2);
        } while (it2->second->module_type.compare("SoftModule") != 0 || it2->second->name == it->second->name);

        auto sm1 = it->second;
        auto sm2 = it2->second;
        // SoftModule* new_sm = new SoftModule((SoftModule*)sm);
        SoftModule* new_sm1 = new SoftModule((SoftModule*)sm1);
        SoftModule* new_sm2 = new SoftModule((SoftModule*)sm2);
        new_sm1->x_coor = sm2->x_coor, new_sm1->y_coor = sm2->y_coor;
        new_sm2->x_coor = sm1->x_coor, new_sm2->y_coor = sm1->y_coor;

        bool overlap_or_overbound = false;
        for (auto& fm : return_sm) {
            if (new_sm1->name.compare(fm.second->name) != 0 && new_sm2->name.compare(fm.second->name) != 0) {
                if (is_overlap(new_sm1, fm.second) || is_overlap(new_sm2, fm.second)) {
                    overlap_or_overbound = true;
                    break;
                }
                else if (is_overBound_x((Module*)new_sm1) || is_overBound_x((Module*)new_sm2)) {
                    overlap_or_overbound = true;
                    break;
                }
                else if (is_overBound_y((Module*)new_sm1) || is_overBound_y((Module*)new_sm2)) {
                    overlap_or_overbound = true;
                    break;
                }
            }
        }
        if (is_overlap(new_sm1, new_sm2)) {
            overlap_or_overbound = true;
        }

        if (!overlap_or_overbound) {
            return_sm[sm1->name] = new_sm1;
            return_sm[sm2->name] = new_sm2;
        }
    }
    // reshape
    else if (op_num == 4) {
        SoftModule* new_sm = new SoftModule((SoftModule*)it->second);
        int hw_pair_random_idx = rand() % new_sm->tried_hw.size();
        new_sm->admis_hw_idx = hw_pair_random_idx;
        new_sm->tried_hw[hw_pair_random_idx] = false;
        new_sm->height = new_sm->possible_h_and_w[hw_pair_random_idx].first;
        new_sm->width = new_sm->possible_h_and_w[hw_pair_random_idx].second;

        bool overlap_or_overbound = false;
        for (auto& fm : return_sm) {
            if (new_sm->name != fm.second->name && (is_overlap(new_sm, fm.second) || is_overBound_x(new_sm) || is_overBound_y(new_sm))) {
                overlap_or_overbound = true;
                break;
            }
        }
        if (!overlap_or_overbound) {
            return_sm[it->second->name] = new_sm;
        }
    }

    return return_sm;
}

bool Floorplan::isAccept(double T, long long delta_c) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> uni_distribution(0.0, 1.0);
    double random_num = uni_distribution(gen);

    return (random_num < exp(-delta_c / T)) ? true : false;
}

void Floorplan::reduce(double& T) {
    if (T < 2000.0) {
        T *= 0.9;
    }
    else {
        // T--;
        T *= 0.998;
    }
    // T *= 0.9999;
}

double Floorplan::cal_time(clock_t start) {
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

void Floorplan::set_max_SA_time(double set_time) {
    max_SA_time = set_time;
}

void Floorplan::runtime_report() {
    total_time = cal_time(main_start);
    cout << "----runtime_report info----" << endl;
    cout << "[Total time]: " << total_time << " secs" << endl;
    cout << "[Input time]: " << input_time << " secs" << endl;
    cout << "[Initial Floorplan time]: " << initFloorplan_time << " secs" << endl;
    cout << "[SA time]: " << SA_time << " secs" << endl;
}