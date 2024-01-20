#include <bits/stdc++.h>

using namespace std;

class Tech;
class LibCell;
class Cell;
class Net;
class Die;

class Input {
public:
    unordered_map<string, Tech*> techs;
    // unordered_map<string, LibCell *> lib_cells;
    vector<Net*> nets;
    unordered_map<string, Die*> dies;
    unordered_map<string, Cell*> cells;

    int Pmax = INT_MIN;
    long long DieSize;
    double balance;

    Input();
    void loadInput(int argc, char* argv[]);
    void loadTechs(ifstream& inputfile);
    void loadDies(ifstream& inputfile);
    void loadCells(ifstream& inputfile);
    void loadNets(ifstream& inputfile);
    void checkInput(string filename);

    double input_time;
};

class Die {
public:
    Tech* tech;
    double max_util;
    int maxNumPins, numCells;
    long long maxSize, totalCellSize;
    unordered_map<int, list<Cell *>> bucketList;

    void setBucketSize(int mpn);
    void insertCelltoBucket(Cell* cell);
    void removeCellfromBucket(Cell* cell);
    void updateCellinBucket(Cell* cell);

    Die(Tech* tech, double max_util, long long maxSize);
};

class Tech {
public:
    string name;
    int num_LC;
    unordered_map<string, LibCell*> lib_cells;

    Tech(string name, int num_LC);
};

class LibCell {
public:
    string name;
    long long width;
    long long height;

    LibCell(string name, long long width, long long height);
};

class Cell {
public:
    string name;
    // LibCell *libCell;
    string lcName;
    vector<Net*> nets;

    bool isLocked;
    long long gain;
    string whichDie, theOtherDie;
    
    unordered_map<string, long long> sizes;
    list<Cell *>::iterator it_inBucketList;

    // update base cell's gain
    void updateCellGain();

    Cell(string name, string lcName);
};

class Net {
public:
    string name;
    int num_cell;
    vector<Cell*> cells;
    // cellNumInDie[die name] = number of cell in net in "die name" 
    unordered_map<string, int> cellNumInDie;

    Net(string name);
};
