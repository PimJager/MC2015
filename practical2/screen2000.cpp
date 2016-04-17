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
    BlockVec blockY;
    ManVec manX;
    ManVec manY;
    Screen screen;
    int rows;
    int cols;
};

SokobanVars buildScreen(const Screen& screen, int rows, int cols) 
{
    LACE_ME;
    int maxX    = cols-1; //inclusive
    int maxY    = rows-1; //inclusive
    int numBlocks  = 0+1;

    int bddVarCounter = 0;
    //-- Initialize the variables
    //-- block{i}_x{j} and block{i}_y{j}
    std::vector<std::vector<Bdd>> blockX;
    std::vector<std::vector<Bdd>> blockY;
    for(int i=0; i<numBlocks; i++){
        std::vector<Bdd> rowX;
        std::vector<Bdd> rowY;
        for(int j=0; j<=maxX; j++) {
            rowX.push_back(Bdd::bddVar(bddVarCounter++));
        }
        for(int j=0; j<=maxY; j++) {
            rowY.push_back(Bdd::bddVar(bddVarCounter++));
        }
        blockX.push_back(rowX);
        blockY.push_back(rowY);
    }
    //--manX_{j} and manY_{j}
    std::vector<Bdd> manX;
    std::vector<Bdd> manY;
    for(int j=0; j<=maxX; j++) {
        manX.push_back(Bdd::bddVar(bddVarCounter++));
    }
    for(int j=0; j<=maxY; j++) {
        manY.push_back(Bdd::bddVar(bddVarCounter++));
    }
    return {blockX, blockY, manX, manY, screen, rows, cols};
}

Bdd propInit(SokobanVars vars){
    LACE_ME;
    Screen screen = vars.screen;
    BlockVec blockX = vars.blockX;
    BlockVec blockY = vars.blockY;
    ManVec manX = vars.manX;
    ManVec manY = vars.manY;
    //-- result= (∀x,y.Ɐ(b∈blocks). !block{b}_{X/Y}{}) 
    //              ⋁ (Ɐ(b∈blocks). block{b}_{X/Y}{bx / by})
    //   similar for man_{X/Y}
    //--make the formula for everything is empty
    Bdd empty = Bdd::bddOne();
    for(int i=0; i<blockX.size(); i++){
        for(int j=0; j<blockX[i].size(); j++){ //block i on position x=j
            empty = empty * !blockX[i][j];
        }
        for(int j=0; j<blockY[i].size(); j++){ //block i on position y=j
            empty = empty * !blockY[i][j];
        }
    }
    for(int j=0; j<vars.cols; j++){
        empty = empty * !manX[j];
    }
    for(int j=0; j<vars.rows; j++){
        empty = empty * !manY[j];
    }
    //--make the formula for blocks and man in position
    Bdd contents = Bdd::bddOne();
    int i=0;
    for(int x=0; x<vars.cols; x++){
        for(int y=0; y<vars.rows; y++){
            if(screen[y][x] == Field::BLOCK 
                || screen[y][x] == Field::BLOCK_ON_GOAL) {
                std::cout << "Block on: ("<< x <<","<< y <<")"<< std::endl;
                contents = contents * blockX[i][x];
                contents = contents * blockY[i][y];
                i++;
            } else if(screen[y][x] == Field::MAN 
                || screen[y][x] == Field::MAN_ON_GOAL) {
                std::cout << "Man on: ("<< x <<","<< y <<")"<< std::endl;
                contents = contents * manX[x];
                contents = contents * manY[y];
            }
        }
    }
    Bdd result = sylvan_or(empty.GetBDD(), contents.GetBDD());
    return result;
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
    Bdd init = propInit(vars);
    BddGraphGenerate(init, "init");

    std::cerr << screen;

    return 0;
}

void BddPrint(Bdd a){
    if(!a.isTerminal()){
        std::cout << "NODE:  "<< a.TopVar()  << std::endl;    
        
        BddPrint(a.Then());
        BddPrint(a.Else());
    }else{
        std::cout << "TERMINAL:  "<< (a==Bdd::bddOne()) << (a==Bdd::bddZero())  << std::endl;
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
        sylvan_fprintdot(pFile,a.GetBDD());
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