#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#include <sylvan.h>
#include <sylvan_obj.hpp>

using namespace sylvan;

enum class Field {EMPTY, WALL, MAN, BLOCK, GOAL, BLOCK_ON_GOAL, MAN_ON_GOAL};
static std::vector<std::string> readfile(std::istream& input);
static size_t max_line_length(std::vector<std::string>& lines);
static std::vector<std::vector<Field>> parse_screen(std::vector<std::string> lines, size_t rows, size_t cols);
template<typename T>
static std::ostream& operator<<(std::ostream& stream, std::vector<std::vector<T>>& matrix);
void BddGraphGenerate(Bdd a, std::string suffix);
void BddPrint(Bdd a);

using Screen = std::vector<std::vector<Field>>;
using ManVec = std::vector<Bdd>;
using BlockVec = std::vector<std::vector<Bdd>>;

#define block1_x0 0
#define block1_x1 1
#define block1_x2 2
#define block1_x3 3
#define block1_y0 4
#define block1_y1 5
#define block1_y2 6
#define man_x0 7
#define man_x1 8
#define man_x2 9
#define man_x3 10
#define man_y0 10
#define man_y1 11
#define man_y2 12

struct SokobanVars {
    BlockVec blockX;
    BlockVec blockXP; //P is for primed
    BlockVec blockY;
    BlockVec blockYP;
    ManVec manX;
    ManVec manXP;
    ManVec manY;
    ManVec manYP;
    Screen screen;
    int rows;
    int cols;
};

SokobanVars buildScreen(const Screen& screen, int rows, int cols) 
{
    LACE_ME;
    int maxX    = cols-1; //inclusive
    int maxY    = rows-1; //inclusive
    int numBlocks  = 1;

    int bddVarCounter = 0;
    //-- Initialize the variables
    //-- block{i}_x{j} and block{i}_y{j}
    std::vector<std::vector<Bdd>> blockX;
    std::vector<std::vector<Bdd>> blockXP;
    std::vector<std::vector<Bdd>> blockY;
    std::vector<std::vector<Bdd>> blockYP;
    for(int i=0; i<numBlocks; i++){
        std::vector<Bdd> rowX;
        std::vector<Bdd> rowXP;
        std::vector<Bdd> rowY;
        std::vector<Bdd> rowYP;
        for(int j=0; j<=maxX; j++) {
            rowX.push_back(Bdd::bddVar(bddVarCounter++));
            rowXP.push_back(Bdd::bddVar(bddVarCounter++));
        }
        for(int j=0; j<=maxY; j++) {
            rowY.push_back(Bdd::bddVar(bddVarCounter++));
            rowYP.push_back(Bdd::bddVar(bddVarCounter++));
        }
        blockX.push_back(rowX);
        blockXP.push_back(rowXP);
        blockY.push_back(rowY);
        blockYP.push_back(rowYP);
    }
    //--manX_{j} and manY_{j}
    std::vector<Bdd> manX;
    std::vector<Bdd> manXP;
    std::vector<Bdd> manY;
    std::vector<Bdd> manYP;
    for(int j=0; j<=maxX; j++) {
        manX.push_back(Bdd::bddVar(bddVarCounter++));
        manXP.push_back(Bdd::bddVar(bddVarCounter++));
    }
    for(int j=0; j<=maxY; j++) {
        manY.push_back(Bdd::bddVar(bddVarCounter++));
        manYP.push_back(Bdd::bddVar(bddVarCounter++));
    }
    return {blockX, blockXP, blockY, blockYP, manX, manXP, manY, manYP, screen, rows, cols};
}

Bdd propInit(const SokobanVars vars){
    LACE_ME;
    Screen screen = vars.screen;
    BlockVec blockX = vars.blockX;
    BlockVec blockY = vars.blockY;
    ManVec manX = vars.manX;
    ManVec manY = vars.manY;
    //--make the formula for blocks and man in position
    //use the formula from the report
    return staticInit(vars);
}

Bdd staticInit(const SokobanVars vars){
    Screen screen = vars.screen;
    BlockVec bX = vars.blockX;
    BlockVec bY = vars.blockY;
    ManVec mX = vars.manX;
    ManVec mY = vars.manY;
    return !bX[0][0] * !bX[0][1] * bX[0][2] * !bX[0][3]
            * !bY[0][0] * bY[0][1] * !bY[0][2]
	    * mX[0] * !mX[1] * !mX[2] * !mX[3]
            * !mY[0] * mY[1] * !mY[2];
}

Bdd propError(const SokobanVars vars){
    LACE_ME;
    Screen screen = vars.screen;
    BlockVec blockX = vars.blockX;
    BlockVec blockY = vars.blockY;
    ManVec manX = vars.manX;
    ManVec manY = vars.manY;
    //--make the formula for blocks and man in position
    return staticError(vars);
}

Bdd staticError(const SokobanVars vars){
    LACE_ME;
    //error is when two blocks overlap or when a block overlaps with a wall
    //or when the man overlaps with a wall
    //transitions for man overlapping with wall are not generated
    BlockVec bX = vars.blockX;
    BlockVec bY = vars.blockY;
    return bX[0][0] * bY[0][0];
}

struct TransCube {
    Bdd trans;
    BddSet cube;
};

TransCube propTrans(const SokobanVars vars){
    LACE_ME;
    BlockVec bX = vars.blockX;
    BlockVec bXP = vars.blockXP;
    BlockVec bY = vars.blockY;
    BlockVec bYP = vars.blockYP;
    ManVec mX = vars.manX;
    ManVec mXP = vars.manXP;
    ManVec mY = vars.manY;
    ManVec mYP = vars.manYP;
    Bdd result = Bdd::bddOne();
    BddSet cube;
    for(int x=0; x<vars.cols; x++){
        for (int y=0; y<vars.rows; y++){
            //--build the transition relation. It is of the form
            //(m_in_some_pos -> (m moves left OR right OR up OR down)) 
            //  AND m_in_some_other_pos -> (m moves left OR etc.) etc.
            //  the moving up/left/right/etc. is restricted to when the man is not already
            //  at the edge of the game
            //needs to be expended for moving blocks around and not pushing blocks of the
            //board
            Bdd next_man_up, next_man_down, next_man_left, next_man_right = Bdd::bddZero();
            if(y<vars.rows-1){ next_man_up = mXP[x] * !mYP[y] * mYP[y+1]; } 
            if(y>0){ next_man_down = mXP[x] * !mYP[y] * mYP[y-1]; }
            if(x<vars.cols-1){ next_man_right = mXP[x+1] * !mXP[x] * mYP[y]; }
            if(x>0){ next_man_left = mXP[x-1] * !mXP[x] * mYP[y]; }
            Bdd cur_man_in_pos = mX[x] * mY[y];
            Bdd trans_for_cur_pos = next_man_up + next_man_down + next_man_left + next_man_right;
            result = result * sylvan_imp(cur_man_in_pos.GetBDD(), trans_for_cur_pos.GetBDD());
        }
    }
    return {result, cube};
}

Bdd staticGoal(const SokobanVars vars){
    BlockVec bX = vars.blockX;
    BlockVec bY = vars.blockY;
    ManVec mY = vars.manY;
    ManVec mX = vars.manX;
    return !bX[0][0] * !bX[0][1] * !bX[0][2] * bX[0][3]
            * !bY[0][0] * !bY[0][1] * bY[0][2];
}

Bdd existsUntil(Bdd ex, Bdd un, TransCube tc, Bdd z){
    LACE_ME;
    std::cerr << "EU run" << std::endl;
    //result = un 'or' (ex 'and' PREV())
    Bdd prev = z.RelPrev(tc.trans, tc.cube);
    BddPrint(prev);
    Bdd result = ex + (un * prev);
    if(result == z) {
        std::cerr << "result == z" << std::endl;
        return z; 
    }
    else {
        std::cerr << "result != z; more recursion " << std::endl;
        return existsUntil(ex, un, tc, result);
    }
}

void setUpSylvan(){
    lace_init(0, 0); //auto #workers and task_deque
    lace_startup(0, NULL, NULL); //auto stack size
    LACE_ME;
    sylvan_init_package(1LL<<21, 1LL<<27, 1LL<<20, 1LL<<26);
    sylvan_init_bdd(6);
}

int main(int argc, char* argv[]){
    //read the screen from stdin or argv[1]
    std::vector<std::string> lines;
    if (argc > 1) {
        std::ifstream ifs;
        ifs.open(argv[1], std::ifstream::in);
        lines = readfile(ifs);
        ifs.close();
    } else {
        lines = readfile(std::cin);
    }
    auto rows = lines.size();
    auto cols = max_line_length(lines);

    std::vector<std::vector<Field>> screen = parse_screen(lines, rows, cols);

    //setup sylvan
    setUpSylvan();
    
    SokobanVars vars = buildScreen(screen, rows, cols);
    std::cerr << "VARS: " << std::endl
                <<"\t cols: "<< vars.cols <<", rows: "<<vars.rows<< std::endl
                <<"\t numBlocks: "<<vars.blockX.size()<<"("<<vars.blockY.size()<<")"<<std::endl
                <<"\t blockX[0].size: "<<vars.blockX[0].size()<<", blockY[0].size: "<<vars.blockY[0].size()<<std::endl
                <<"\t manX.size:"<<vars.manX.size()<<", manY.size: "<<vars.manY.size()<<std::endl;
    
    Bdd init = staticInit(vars);
    BddGraphGenerate(init, "init");

    Bdd error = staticError(vars);
    BddGraphGenerate(error, "error");

    TransCube tc = propTrans(vars);
    BddGraphGenerate(tc.trans, "trans");

    Bdd goal = staticGoal(vars);
    BddGraphGenerate(goal, "goal");    

    //Exists !error Until Goal
    Bdd lfp = existsUntil(!error, goal, tc, Bdd::bddZero());
    BddGraphGenerate(lfp, "lfp");

    Bdd result = goal * lfp;
    BddGraphGenerate(result, "result");        

    //TODO: figure out when this is actually succesfull... 

    std::cerr << screen;

    return 0;
}

void exampletrans(){
    LACE_ME;
    Bdd x = Bdd::bddVar(100); //current state needs even variable numbers
    Bdd y = Bdd::bddVar(102);
    Bdd cur = x * y; 
    Bdd xp = Bdd::bddVar(101); //next state needs odd variable numbers
    Bdd yp = Bdd::bddVar(103); //i.e. yp = y' (y-prime)
    Bdd trans = sylvan_imp(x.GetBDD(), (!xp).GetBDD());
    Bdd next = cur.RelPrev(trans, x*xp);
    BddGraphGenerate(cur, "cur");
    BddGraphGenerate(trans, "trans");
    BddGraphGenerate(next, "next");
}

void BddPrint(Bdd a){
    if(!a.isTerminal()){
        std::cerr << "NODE:  "<< a.TopVar()  << std::endl;    
        
        BddPrint(a.Then());
        BddPrint(a.Else());
    }else{
        std::cerr << "TERMINAL:  "<< (a==Bdd::bddOne()) << (a==Bdd::bddZero())  << std::endl;
    }
}

void BddGraphGenerate(Bdd a, std::string suffix){
    FILE * pFile;
    std::string path = std::string("testBDD_")+suffix+std::string(".dot");
    pFile = fopen (path.c_str(), "w");
    if (pFile == NULL) 
        perror ("Error opening file");
    else
    {
        sylvan_fprintdot_nc(pFile,a.GetBDD());
    }
    fclose (pFile);
}


// reading Sylvan screens
// read a file, result is a vector of strings for each line, without trailing whitespace
static std::vector<std::string> readfile(std::istream& input) 
{
    std::vector<std::string> result;
    for (std::string line; std::getline(input, line);) {
        // get rid of trailing whitespace
        line = line.substr(0, line.find_last_not_of(" \t")+1);
        if (line.length() > 0) result.push_back(line);
    }
    return result;
}

// given a vector of strings, calculate the length of the longest string
static size_t max_line_length(std::vector<std::string>& lines)
{
    size_t max = 0;
    for (auto it = std::begin(lines); it != std::end(lines); it++) {
        if (it->length() > max) max = it->length();
    }
    return max;
}

// create a 'matrix' (vector of vectors) of size <rows> x <cols>, filled with <initial_value>
template <typename T>
static std::vector<std::vector<T>> create_matrix(const size_t rows, const size_t cols, const T initial_value)
{
    std::vector<std::vector<T>> result;
    for (size_t i = 0; i < rows; i++) {
        std::vector<T> row;
        row.assign(cols, initial_value);
        result.push_back(row);
    }
    return result;
}

// given the vector of strings of a screen input file, with <rows> rows and <cols> columns, return the screen in a matrix
static std::vector<std::vector<Field>> parse_screen(std::vector<std::string> lines, size_t rows, size_t cols)
{
    std::vector<std::vector<Field>> screen = create_matrix(rows, cols, Field::EMPTY);

    for (size_t i = 0; i < rows; i++) {
        std::string s = lines[i];
        std::vector<Field> &row = screen[i];

        for (size_t j = 0; j < s.length(); j++) {
            switch (s[j]) {
            case '#':
                row[j] = Field::WALL;
                break;
            case '@':
                row[j] = Field::MAN;
                break;
            case '.':
                row[j] = Field::GOAL;
                break;
            case ' ':
                row[j] = Field::EMPTY;
                break;
            case '$':
                row[j] = Field::BLOCK;
                break;
            case '*':
                row[j] = Field::BLOCK_ON_GOAL;
                break;
            case '+':
                row[j] = Field::MAN_ON_GOAL;
                break;
            default:
                std::cerr << "Invalid character '" << s[j] << "' in screen!" << std::endl;
                abort();
            }
        }
    }
    return screen;
}

// helper function for writing the screen from matrix to stream
static std::ostream& operator<<(std::ostream& stream, const Field field)
{
    switch (field) {
        case Field::WALL:
            stream << "#";
            break;
        case Field::MAN:
            stream << "@";
            break;
        case Field::GOAL:
            stream << ".";
            break;
        case Field::EMPTY:
            stream << " ";
            break;
        case Field::BLOCK:
            stream << "$";
            break;
        case Field::BLOCK_ON_GOAL:
            stream << "*";
            break;
        case Field::MAN_ON_GOAL:
            stream << "+";
            break;
    }
    return stream;
}

// helper function for writing a matrix to a stream
template<typename T>
static std::ostream& operator<<(std::ostream& stream, std::vector<std::vector<T>>& matrix)
{
    for (size_t i = 0; i < matrix.size(); i++) {
        std::vector<T> row = matrix[i];
        for (size_t j = 0; j < row.size(); j++) {
            stream << row[j];
        }
        stream << std::endl;
    }
    return stream;
}