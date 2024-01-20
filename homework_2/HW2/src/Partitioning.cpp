#include <bits/stdc++.h>
#include "Partitioning.hpp"

using namespace std;

void Cell::updateCellGain() {
    gain = 0;
    for (auto n : nets) {
        if (n->cellNumInDie[whichDie] == 1) gain++;
        if (n->cellNumInDie[theOtherDie] == 0) gain--;
    }
}

void Die::setBucketSize(int mpn) {
    maxNumPins = mpn;
    // for (auto d : dies) {
        // d.second->maxNumPins = mpn;
    for (int i = -mpn; i <= mpn; i++) {
        list<Cell*> cellList;
        bucketList[i] = cellList;
    }
    // }
}
void Die::insertCelltoBucket(Cell* cell) {
    bucketList[cell->gain].push_back(cell);
    cell->it_inBucketList = bucketList[cell->gain].end();
    --(cell->it_inBucketList); // point to .back()

    // totalCellSize += cell->sizes[tech->name];
    numCells++;
}
void Die::removeCellfromBucket(Cell* cell) {
    list<Cell*>::iterator rm = cell->it_inBucketList;
    bucketList[cell->gain].erase(rm);
    cell->it_inBucketList = bucketList[cell->gain].end();

    // totalCellSize -= cell->sizes[tech->name];
    numCells--;
}
void Die::updateCellinBucket(Cell* cell) {
    removeCellfromBucket(cell);
    insertCelltoBucket(cell);
}

FM_Algo::FM_Algo(Input* p_input) : input(p_input), check_filename("") {}

void FM_Algo::run() {
    cout << "Debug: Start to init()" << endl;
    init();

    // check initialization
    if (!check_filename.empty()) {
        cout << "Debug: Start to checkInitialization()" << endl;
        checkInitialization();
    }

    cout << "\nDebug: Start to partition()" << endl;
    set_max_fmTime(295.0);
    partition();

    if (!check_filename.empty()) {
        cout << "Debug: Start to checkPartition()" << endl;
        checkPartition();
    }
}

void FM_Algo::init() {
    //cout << "Debug: Start to initpartitioning()" << endl;
    initPartition();

    // initialize cell number of each net for dieA & dieB
    //cout << "Debug: Start to initNetDistribution()" << endl;
    initNetDistribution();

    // initialize cell gain
    //cout << "Debug: Start to initCellGains()" << endl;
    initCellGains();

    // initialize bucket list
    //cout << "Debug: Start to initBucketLists()" << endl;
    initBucketLists();

    //cout << "Debug: init() Ends." << endl;
}


void FM_Algo::initPartition() {
    // Variable initialize
    long long dieCellSizeA, dieCellSizeB;
    vector<Cell*> unsortedCells;

    // Push all cell into a vector
    // //cout << "Debug: In initPartition(), start to push all cell into vector<Cell*>unsortedCells" << endl;
    for (auto cp : input->cells) {
        unsortedCells.push_back(cp.second);
    }

    // //cout << "Debug: In initPartition(), start to random partition" << endl;

    // random partition
    int cnt = 0;
    do {
        cnt++;
        dieCellSizeA = 0, dieCellSizeB = 0;
        random_device rd;
        default_random_engine rng;
        int testSeed = rd();
        rng.seed(testSeed);
        mt19937 g(testSeed);

        shuffle(unsortedCells.begin(), unsortedCells.end(), g);
        //cout << "Debug: random times = " << cnt << endl;

        string assignDie = "DieA";
        for (auto c : unsortedCells) {
            if ((double)(dieCellSizeA + c->sizes[input->dies[assignDie]->tech->name]) > (double)input->DieSize * input->dies["DieA"]->max_util) {
                assignDie = "DieB";
            }
            c->whichDie = assignDie;
            if (assignDie.compare("DieA") == 0) {
                c->theOtherDie = "DieB";
                dieCellSizeA += c->sizes[input->dies[assignDie]->tech->name];
            }
            else if (assignDie.compare("DieB") == 0) {
                c->theOtherDie = "DieA";
                dieCellSizeB += c->sizes[input->dies[assignDie]->tech->name];
            }
        }
        //cout << "Debug: after random push cells, dieA_isbelow = " << dieA_isbelow << ", dieB_isbelow = " << dieB_isbelow << endl;
    } while (!isBalanced(dieCellSizeA, dieCellSizeB));
    input->dies["DieA"]->totalCellSize = dieCellSizeA;
    input->dies["DieB"]->totalCellSize = dieCellSizeB;
    //cout << "Debug: In initPartition(), dieCellSizeA = " << input->dies["DieA"]->totalCellSize << ", dieCellSizeB = " << input->dies["DieB"]->totalCellSize << endl;
    //cout << "Debug: In initPartition(), End all the function" << endl;
}

void FM_Algo::initNetDistribution() {
    for (auto np : input->nets) {
        np->cellNumInDie["DieA"] = np->cellNumInDie["DieB"] = 0;
        for (auto& cp : np->cells) {
            np->cellNumInDie[cp->whichDie]++;
        }
    }
}

void FM_Algo::initCellGains() {
    for (auto& c : input->cells) {
        c.second->gain = 0;
        for (auto n : c.second->nets) {
            if (n->cellNumInDie[c.second->whichDie] == 1) c.second->gain++;
            if (n->cellNumInDie[c.second->theOtherDie] == 0) c.second->gain--;
        }
    }
}

void FM_Algo::initBucketLists() {
    for (auto& d : input->dies) {
        d.second->setBucketSize(input->Pmax);
    }
    for (auto& cell : input->cells) {
        input->dies[cell.second->whichDie]->insertCelltoBucket(cell.second);
    }
}

void FM_Algo::partition() {
    maxPartialSum = 1;
    partition_times = 1;

    while (maxPartialSum > 0) {
        cout << "Debug: partition times = " << partition_times << endl;
        loopPartition();
        partition_times++;
    }
}

void FM_Algo::loopPartition() {
    int currentPartialSum = 0;
    int loop_times = 0;
    bool terminate = false;

    // Once we get a base cell, we have already done the balance condition check
    Cell* base_cell = getBaseCell();
    while (base_cell != nullptr) {
        //cout << "Debug: in loopPartition(), getBaseCell() success, base cell = " << base_cell->name <<" from " << base_cell->whichDie<< ", gain = " << base_cell->gain << endl;
        //cout << "Debug: in loopPartition(), push base cell to maxGains & selectedBaseCells" << endl;
        maxGains.push_back(base_cell->gain);
        selectedBaseCells.push_back(base_cell);

        //cout << "Debug: in loopPartition(), start to updateCellGains()" << endl;
        updateCellGains(base_cell);
        //cout << "Debug: in loopPartition(), updateCellGains() ends" << endl;

        if (partition_times == 1) {
            if (loop_times >= (int)(input->cells.size() * 0.5)) {
                break;
            }
        }
        else {
            if (loop_times >= (int)input->cells.size() * 0.1) {
                break;
            }
        }
        if (((clock() - main_start) / (double)CLOCKS_PER_SEC) > max_fm_time) {
            terminate = true;
            break;
        }

        //cout << "Debug: in loopPartition(), try to get next base cell" << endl;
        base_cell = getBaseCell();
        loop_times++;
        // cout << "Debug: in loopPartition(), 1 loop complete, loop_times = " << loop_times << endl;
    }
    //cout << "Debug: loop_times in loopPartition: " << loop_times << endl;

    calculateMaxPartialSum();
    cout << "Debug: Max partial sum: " << maxPartialSum << endl;
    cout << "Debug: Steps for max partial sum: " << steps_for_maxPartialSum << endl;
    cout << "Debug: Base cells size: " << selectedBaseCells.size() << endl;

    // unlock cells for canceling moving selectedBaseCell after steps_for_maxPartialSum
    unlock_all_cells();
    if (maxPartialSum <= 0) {
        cancel_moving_selectedBaseCell_after(0);
    }
    else if (steps_for_maxPartialSum < (selectedBaseCells.size() - 1)) {
        cancel_moving_selectedBaseCell_after(steps_for_maxPartialSum);
    }

    calculateCellGains();

    // check
    if (!check_filename.empty()) {
        check_after_loopPart();
    }
    
    // clear
    unlock_all_cells();
    maxGains.clear();
    selectedBaseCells.clear();

    if (terminate) {
        maxPartialSum = INT_MIN;
    }

    cout << "Debug: cut size: " << calculateCutSize() << endl << endl;
}

void FM_Algo::updateCellGains(Cell* bc) {
    string FromDie = bc->whichDie;
    string ToDie = bc->theOtherDie;

    // Lock the base cell and complement its block
    // //cout << "Debug: in updateCellGains(), Lock the base cell and complement its block" << endl;
    bc->isLocked = true;
    // move the base cell and update all information about it
    input->dies[FromDie]->removeCellfromBucket(bc);
    updateDieSizes(FromDie, ToDie, bc);
    bc->whichDie = ToDie;
    bc->theOtherDie = FromDie;
    bc->updateCellGain();
    input->dies[ToDie]->insertCelltoBucket(bc);

    // //cout << "Debug: in updateCellGains(), for loop iterate each net starts" << endl;
    for (auto bc_net : bc->nets) {
        // check critical nets before the move
        if (bc_net->cellNumInDie[ToDie] == 0) {
            for (auto freeCellinBCNet : bc_net->cells) {
                if (!freeCellinBCNet->isLocked) {
                    freeCellinBCNet->gain++;
                    // since free cell.gain has been changed, need to update its position in bucket list
                    input->dies[FromDie]->updateCellinBucket(freeCellinBCNet);
                }
            }
        }
        else if (bc_net->cellNumInDie[ToDie] == 1) {
            for (auto freeCellinBCNet_ToDie : bc_net->cells) {
                if (!freeCellinBCNet_ToDie->isLocked && freeCellinBCNet_ToDie->whichDie.compare(ToDie) == 0) {
                    freeCellinBCNet_ToDie->gain--;
                    // since free cell.gain in ToDie has been changed, need to update its position in bucket list
                    input->dies[ToDie]->updateCellinBucket(freeCellinBCNet_ToDie);
                }
            }
        }
        // cout << "Finish checking critical nets before move\n";

        // change net.F and net.T to reflect the move
        bc_net->cellNumInDie[FromDie]--;
        bc_net->cellNumInDie[ToDie]++;

        // check for critical nets after the move
        if (bc_net->cellNumInDie[FromDie] == 0) {
            for (auto freeCellinBCNet : bc_net->cells) {
                if (!freeCellinBCNet->isLocked) {
                    freeCellinBCNet->gain--;
                    // since free cell.gain has been changed, need to update its position in bucket list
                    input->dies[ToDie]->updateCellinBucket(freeCellinBCNet);
                }
            }
        }
        else if (bc_net->cellNumInDie[FromDie] == 1) {
            for (auto freeCellinBCNet_FromDie : bc_net->cells) {
                if (!freeCellinBCNet_FromDie->isLocked && freeCellinBCNet_FromDie->whichDie.compare(FromDie) == 0) {
                    freeCellinBCNet_FromDie->gain--;
                    // since free cell.gain in ToDie has been changed, need to update its position in bucket list
                    input->dies[FromDie]->updateCellinBucket(freeCellinBCNet_FromDie);
                }
            }
        }
        // cout << "Finish checking critical nets after move\n";
    }
    //cout << "Debug: in updateCellGains(), for loop iterate each net ends" << endl;
    //cout << "Debug: in updateCellGains(), dieA util = " << (double)(input->dies["DieA"]->totalCellSize / (double)input->DieSize) << endl;
    //cout << "Debug: in updateCellGains(), dieB util = " << (double)(input->dies["DieB"]->totalCellSize / (double)input->DieSize) << endl;
}

void FM_Algo::updateDieSizes(string FromDie, string ToDie, Cell* cell) {
    input->dies[FromDie]->totalCellSize -= cell->sizes[input->dies[FromDie]->tech->name];
    input->dies[ToDie]->totalCellSize += cell->sizes[input->dies[ToDie]->tech->name];
}

// in getBaseCell(), we have done the balance condition check
Cell* FM_Algo::getBaseCell() {
    Cell* bc;

    //cout << "Debug: in getBaseCell(), get 2 base cell from getBaseCellfromDie()" << endl;
    Cell* bc_dieA = getBaseCellfromDie("DieA");
    Cell* bc_dieB = getBaseCellfromDie("DieB");
    if (bc_dieA != nullptr && bc_dieB != nullptr) {
        //cout << "Debug: in getBaseCell(), already got 2 base cell from getBaseCellfromDie()" << endl;
        //cout << "Debug: in getBaseCell(), bc_dieA = " << bc_dieA->name << ", gain = " << bc_dieA->gain << ", cell size = " << bc_dieA->sizes[input->dies["DieA"]->tech->name] << endl;
        //cout << "Debug: in getBaseCell(), bc_dieB = " << bc_dieB->name << ", gain = " << bc_dieB->gain << ", cell size = " << bc_dieB->sizes[input->dies["DieA"]->tech->name] << endl;
        if (bc_dieA->gain > bc_dieB->gain) {
            bc = bc_dieA;
        }
        else {
            bc = bc_dieB;
        }
    }
    else if (bc_dieA != nullptr && bc_dieB == nullptr) {
        //cout << "Debug: in getBaseCell(), bc_dieA = " << bc_dieA->name << ", gain = " << bc_dieA->gain << ", cell size = " << bc_dieA->sizes[input->dies["DieA"]->tech->name] << endl;
        bc = bc_dieA;
    }
    else if (bc_dieA == nullptr && bc_dieB != nullptr) {
        //cout << "Debug: in getBaseCell(), bc_dieB = " << bc_dieB->name << ", gain = " << bc_dieB->gain << ", cell size = " << bc_dieB->sizes[input->dies["DieA"]->tech->name] << endl;
        bc = bc_dieB;
    }
    else {
        //cout << "Debug: both getBaseCellfromDie() get nullptr\n";
        bc = nullptr;
    }

    return bc;
}
// base cell need to be got randomly(follow balance condition)
Cell* FM_Algo::getBaseCellfromDie(string dieName) {
    Cell* baseCell;
    string from = dieName;
    string to = (dieName.compare("DieA") == 0) ? "DieB" : "DieA";
    long long A = 0, B = 0;
    for (int i = input->Pmax; i >= -(input->Pmax); i--) {
        // //cout << "Debug: in getBaseCellfromDie(), iterate bucketList in " << dieName << endl;
        if (input->dies[from]->bucketList[i].empty()) {
            continue;
        }
        // search base cell in bucketList[i]
        for (list<Cell*>::iterator it = input->dies[from]->bucketList[i].begin(); it != input->dies[from]->bucketList[i].end(); ++it) {
            baseCell = (*it);
            if (from.compare("DieA") == 0) {
                A = input->dies[from]->totalCellSize - baseCell->sizes[input->dies[from]->tech->name];
                B = input->dies[to]->totalCellSize + baseCell->sizes[input->dies[to]->tech->name];
            }
            else if (from.compare("DieB") == 0) {
                B = input->dies[from]->totalCellSize - baseCell->sizes[input->dies[from]->tech->name];
                A = input->dies[to]->totalCellSize + baseCell->sizes[input->dies[to]->tech->name];
            }
            if (baseCell->isLocked) continue;
            else {
                if (isBalanced(A, B)) {
                    //cout << "Debug: in getBaseCellfromDie(), if move this bc, " << from << ".totalCellSize = " << A << endl;
                    //cout << "Debug: in getBaseCellfromDie(), if move this bc, " << to << ".totalCellSize = " << B << endl;
                    return baseCell;
                }
                else {
                    break;
                }
            }
        }
    }
    //cout << "Debug: End getBaseCellfromDie(), return nullptr" << endl;
    return nullptr;
}

bool FM_Algo::isBalanced(long long A, long long B) {
    // double DieA_util = ((double)A / (double)input->dies["DieA"]->maxSize);
    // double DieB_util = ((double)B / (double)input->dies["DieB"]->maxSize);
    // bool cond_A = (DieA_util < input->dies["DieA"]->max_util);
    // bool cond_B = (DieB_util < input->dies["DieB"]->max_util);
    long long DieA_maxUtilSize = input->dies["DieA"]->maxSize * input->dies["DieA"]->max_util;
    long long DieB_maxUtilSize = input->dies["DieB"]->maxSize * input->dies["DieB"]->max_util;
    bool cond_A = A < DieA_maxUtilSize;
    bool cond_B = B < DieB_maxUtilSize;

    return (cond_A && cond_B);
}

void FM_Algo::calculateMaxPartialSum() {
    int max_partial_sum = INT_MIN, current_sum = 0, index = -1;
    for (int i = 0; i < maxGains.size(); i++) {
        current_sum += maxGains[i];
        //cout << "Debug: in calculateMaxPartialSum(), current_sum = " << current_sum << " while i = " << i << endl;
        if (max_partial_sum < current_sum) {
            max_partial_sum = current_sum;
            index = i;
        }
        // maybe can calculate for better balance condition?
    }
    maxPartialSum = max_partial_sum;
    steps_for_maxPartialSum = index + 1;
}

int FM_Algo::calculateCutSize() {
    int cut_size = 0;
    for (auto net : input->nets) {
        if (net->cellNumInDie["DieA"] > 0 && net->cellNumInDie["DieB"] > 0) {
            cut_size++;
        }
    }
    return cut_size;
}

void FM_Algo::calculateCellGains() {
    for (auto c : input->cells) {
        c.second->updateCellGain();
    }
}

void FM_Algo::unlock_all_cells() {
    for (auto cell : selectedBaseCells) {
        cell->isLocked = false;
    }
}

void FM_Algo::cancel_moving_selectedBaseCell_after(int steps) {
    for (int i = selectedBaseCells.size() - 1; i >= steps; i--) {
        Cell* basecell = selectedBaseCells[i];
        //cout << "Debug: in cancel_moving(), undo " << basecell->name << endl;
        updateCellGains(basecell);// undo the cell exchange
    }
}

void FM_Algo::check_after_loopPart() {
    string check_loopPart = "_check_loopPartition.out";
    ofstream forCheck(check_filename + check_loopPart);
    forCheck << "----loopPartition Check Start----\n";

    forCheck << "1. selectedBaseCells: ";
    for (auto c : selectedBaseCells) {
        forCheck << c->name << ", ";
    }forCheck << endl;

    forCheck << "2. maxGains: ";
    for (auto g : maxGains) {
        forCheck << g << ", ";
    }forCheck << endl;

    forCheck << "3. moved base cells: ";
    for (int i = 0; i < steps_for_maxPartialSum; i++) {
        forCheck << selectedBaseCells[i]->name << ", ";
    }forCheck << endl;

    forCheck << "4. after moving, new cell gain: ";
    for (auto c : input->cells) {
        forCheck << c.second->name << " gain = " << c.second->gain << ", ";
    }forCheck << endl;

    forCheck << "5. cut size: " << calculateCutSize() << endl << endl;

    forCheck << "----loopPartition Check End----\n";
}

void FM_Algo::checkPartition() {
    string check_partition = "_check_partition.out";
    ofstream forCheck(check_filename + check_partition);
    forCheck << "----Partition Check Start----\n";
    forCheck << "Die max size: " << input->DieSize << endl;
    forCheck << "DieA total cell size: " << input->dies["DieA"]->totalCellSize << ", max_util: " << input->dies["DieA"]->max_util << endl;
    forCheck << "DieB total cell size: " << input->dies["DieB"]->totalCellSize << ", max_util: " << input->dies["DieB"]->max_util << endl;
    forCheck << "DieA cell number: " << input->dies["DieA"]->numCells << endl;
    forCheck << "DieB cell number: " << input->dies["DieB"]->numCells << endl;
    forCheck << "Total cell number: " << input->cells.size() << endl;

    forCheck << "3. cut size: " << calculateCutSize() << endl;
    forCheck << "----Partition Chcek End----\n";
}

void FM_Algo::checkInitialization() {
    // Debug to check 1. cell->whichDie, 2. cell number of each net, 3. cell gain
        // 4. bucket list of each die
    string check_init = "_check_initialization.out";
    ofstream forCheck(check_filename + check_init);
    forCheck << "----Initialization Check----\n";
    forCheck << "Die max size: " << input->DieSize << endl;
    forCheck << "DieA total cell size: " << input->dies["DieA"]->totalCellSize << ", max_util: " << input->dies["DieA"]->max_util << endl;
    forCheck << "DieB total cell size: " << input->dies["DieB"]->totalCellSize << ", max_util: " << input->dies["DieB"]->max_util << endl;
    forCheck << "DieA cell number: " << input->dies["DieA"]->numCells << endl;
    forCheck << "DieB cell number: " << input->dies["DieB"]->numCells << endl;
    forCheck << "Total cell number: " << input->cells.size() << endl;

    int cellNumDieA = 0, cellNumDieB = 0;
    for (auto& c : input->cells) {
        if (c.second->whichDie.compare("DieA") == 0)
            cellNumDieA++;
        else if (c.second->whichDie.compare("DieB") == 0)
            cellNumDieB++;
    }
    forCheck << "1. check die cell number: DieA.numCell = " << cellNumDieA << ", DieB.numCell = " << cellNumDieB << endl;

    forCheck << "2. bucket list: \n";
    for (auto& d : input->dies) {
        forCheck << "  Bucket in " << d.first << endl;
        for (int i = -(input->Pmax); i <= input->Pmax; i++) {
            if (!d.second->bucketList[i].empty()) {
                forCheck << "    gain " << i << " size: " << d.second->bucketList[i].size() << endl;
            }
        }
    }
    forCheck << "3. cut size: " << calculateCutSize() << endl;
    forCheck << "----Initialization Check End----\n";
}

void FM_Algo::outputResult(string filename) {
    ofstream fout(filename);
    int cut_size = calculateCutSize();
    fout << "CutSize " << cut_size << endl;

    fout << "DieA " << input->dies["DieA"]->numCells << endl;
    for (auto cell : input->cells) {
        if (cell.second->whichDie.compare("DieA") == 0) {
            fout << cell.second->name << endl;
        }
    }
    fout << "DieB " << input->dies["DieB"]->numCells << endl;
    for (auto cell : input->cells) {
        if (cell.second->whichDie.compare("DieB") == 0) {
            fout << cell.second->name << endl;
        }
    }
    fout.close();
}

void FM_Algo::set_max_fmTime(double setting) {
    max_fm_time = setting;
}

void FM_Algo::runtime_report() {
    total_time = (double)(clock() - main_start) / CLOCKS_PER_SEC;
    fm_time = total_time - input->input_time;

    cout << "----runtime_report info----" << endl;
    cout << "[Total time]: " << total_time << " secs" << endl;
    cout << "[Input time]: " << input->input_time << " secs" << endl;
    cout << "[FM runtime_report]: " << fm_time << " secs" << endl;
}