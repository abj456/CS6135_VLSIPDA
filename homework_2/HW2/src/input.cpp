// #include <iostream>
// #include <fstream>
#include <bits/stdc++.h>
#include "input.hpp"

using namespace std;

Input::Input() : DieSize(0) {}

void Input::loadInput(int argc, char* argv[]) {
    ifstream inputfile(argv[1]);
    string Type;
    /*---------Load Techs---------*/
    loadTechs(inputfile);

    /*---------Load Die---------*/
    loadDies(inputfile);

    /*---------Load Cells---------*/
    loadCells(inputfile);

    /*---------Load Nets--------*/
    loadNets(inputfile);

    /* -d: debug for testcase input*/
    if (argc > 3) {
        if (!strcmp(argv[3], "-d")) {
            checkInput(argv[4]);
        }
    }
}

void Input::loadTechs(ifstream& inputfile) {
    string Type;
    int techNum;
    inputfile >> Type;

    if (Type.compare("NumTechs") == 0) { // NumTechs 2
        inputfile >> techNum;
        for (int i = 0; i < techNum; i++) {
            string type_class, techName;
            int libCellNum;
            inputfile >> type_class >> techName >> libCellNum;

            if (type_class.compare("Tech") == 0) { // Tech TA 3
                Tech* newTech = new Tech(techName, libCellNum);
                techs[techName] = newTech;
                for (int j = 0; j < newTech->num_LC; j++) {
                    string type_class;
                    string lcName;
                    long long lcWidth, lcHeight;
                    inputfile >> type_class >> lcName >> lcWidth >> lcHeight;
                    if (type_class.compare("LibCell") == 0) { // LibCell MC1 7 10
                        LibCell* newLC = new LibCell(lcName, lcWidth, lcHeight);
                        // lib_cells[lcName] = newLC;
                        newTech->lib_cells[lcName] = newLC;
                    }
                }
            }
        }
    }//end loading tech
}

void Input::loadDies(ifstream& inputfile) {
    string Type;
    inputfile >> Type;

    if (Type.compare("DieSize") == 0) {
        long long dieWidth, dieHeight;
        inputfile >> dieWidth >> dieHeight;
        DieSize = dieWidth * dieHeight;

        for (int i = 0; i < 2; i++) {
            string dieName, dieTechName;
            double maxUtil;
            inputfile >> dieName >> dieTechName >> maxUtil;

            Tech* dieTech = techs[dieTechName];
            Die* newDie = new Die(dieTech, maxUtil / 100, DieSize);
            dies[dieName] = newDie;
        }
    }//end loading die
}

void Input::loadCells(ifstream& inputfile) {
    string Type;
    inputfile >> Type;

    if (Type.compare("NumCells") == 0) {
        int cellNum;
        inputfile >> cellNum;
        for (int i = 0; i < cellNum; i++) {
            string classCell, cellName, lcName;
            inputfile >> classCell >> cellName >> lcName;

            /* find out libCell with same lcName */
            // LibCell *cellLC = lib_cells[lcName];
            Cell* newCell = new Cell(cellName, lcName);
            cells[cellName] = newCell;

            /* input cell sizes of diff tech */
            for (auto& tech: techs) {
                // cout << "debug: " << tech.second->lib_cells[lcName]->width << endl;
                long long w = tech.second->lib_cells[lcName]->width;
                long long h = tech.second->lib_cells[lcName]->height;
                newCell->sizes[tech.first] = w * h;
            }
        }
    }// end loading cells
}

void Input::loadNets(ifstream& inputfile) {
    string Type;
    inputfile >> Type;
    if (Type.compare("NumNets") == 0) { // NumNets 6
        int netNum;
        inputfile >> netNum;
        for (int i = 0; i < netNum; i++) {
            string classNet, netName;
            int netCellNum;
            inputfile >> classNet >> netName >> netCellNum;
            Net* newNet = new Net(netName);// new net
            newNet->num_cell = netCellNum;

            // push cell into net
            for (int j = 0; j < newNet->num_cell; j++) {// Net N1 2
                string classCell, cellName;
                inputfile >> classCell >> cellName;
                Cell* netCell = cells[cellName];
                newNet->cells.push_back(netCell);
                netCell->nets.push_back(newNet);
                
                // cout << "debug Input::loadNets(): " << netCell->nets.size() << endl;
                // cout << "debug Input::Pmax: " << Pmax << endl;
                if ((int)netCell->nets.size() > Pmax) {
                    Pmax = netCell->nets.size();
                }
            }
            nets.push_back(newNet);
        }
    }
}

void Input::checkInput(string filename) {
    string check_input = "_check_input.out";
    ofstream forCheck(filename + check_input);
    forCheck << "----Input Check----\n";
    for (auto tech : techs) {
        forCheck << tech.first << ", " << tech.second->num_LC << endl;
        for (auto libcell : tech.second->lib_cells) {
            forCheck << libcell.first << ", " << libcell.second->height * libcell.second->width << endl;
        }
    }
    forCheck << "DieSize: " << DieSize << endl;
    for (auto die : dies) {
        forCheck << die.second->tech->name << ", " << die.second->max_util << endl;
    }
    for (auto cell : cells) {
        // forCheck << cell.first << ", " << cell.second->libCell->name << endl;
        forCheck << cell.first << ", " << cell.second->lcName << ": ";
        for (auto cellTechSize: cell.second->sizes) {
            forCheck << cellTechSize.first << " & " << cellTechSize.second << "; ";
        }
        forCheck << endl;
    }
    for (auto net : nets) {
        forCheck << net->name << ", " << net->num_cell << endl;
        for (auto cell : net->cells) {
            forCheck << "  " << cell->name << ", " << cell->lcName << endl;
        }
    }
    forCheck << "----Input Check End----\n";
}

Die::Die(Tech* tech, double max_util, long long size) :
    tech(tech), max_util(max_util), maxSize(size), 
    totalCellSize(0), numCells(0) {}

Tech::Tech(string name, int num_LC) :
    name(name), num_LC(num_LC) {}

LibCell::LibCell(string name, long long width, long long height) :
    name(name), width(width), height(height) {}

Cell::Cell(string name, string lcName) :
    name(name), lcName(lcName), isLocked(false) {}

Net::Net(string name) : name(name), num_cell(0) {}

