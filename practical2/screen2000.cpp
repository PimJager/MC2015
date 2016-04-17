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

void buildScreen(const Screen& screen) {
    LACE_ME;
    int maxX    = 1+2; //fake calcualtion at runtime //inclusive
    int maxY    = 1+1; //fake calcualtion at runtime //inclusive
    int numBlocks  = 0+1;

    int bddVarCounter = 0;
    //-- Initialize the variables
    //-- block{i}_x{j} and block{i}_y{j}
    std::vector<std::vector<Bdd>> blockX;
    std::vector<std::vector<Bdd>> blockY;
    for(int i=0; i<numBlocks; i++){
        for(int j=0; j<=maxX; j++) {
            std::vector<Bdd> row;
            row.push_back(Bdd::bddVar(bddVarCounter++));
            blockX.push_back(row);
        }
        for(int j=0; j<=maxY; j++) {
            std::vector<Bdd> row;
            row.push_back(Bdd::bddVar(bddVarCounter++));
            blockY.push_back(row);
        }
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
}

// Bdd propInit(Screen& screen, BlockVec& blockX, BlockVec& blockY,
//          ManVec manX, ManVec manY){
//     LACE_ME;
//     //-- result= (∀x,y.Ɐ(b∈blocks). !block{b}_{X/Y}{}) 
//     //              ⋁ (Ɐ(b∈blocks). block{b}_{X/Y}{bx / by})
//     //   similar for man_{X/Y}
//     Bdd notBlocks = Bdd::bddOne();
//     for(int i=0; i<blockX.size(); i++){
//         for(int j=0; j<blockX[i].size(); j++){ //block i on position x=j
//             notBlocks = notBlocks * !blockX[i][j];
//         }
//         for(int j=0; j<blockY[i].size(); i++){ //block i on position y=j
//             notBlocks = notBlocks * !blockY[i][j];
//         }
//     }
//     Bdd hasBlocks = Bdd::bddOne();
//     int i=0;
//     for(int x=0; x<blockX.size(); x++){
//         for(int y=0; y<blockY.size(); y++){
//             if(screen[x][y] == Field::BLOCK 
//                 || screen[x][y] == Field::BLOCK_ON_GOAL) {
//                 std::cout << "Block on: ("<< x <<","<< y <<")"<< std::endl;
//                 hasBlocks = hasBlocks * blockX[i][x];
//                 hasBlocks = hasBlocks * blockY[i][y];
//             } 
//         }
//     }
//     Bdd blocks = sylvan_or(notBlocks.GetBDD(), hasBlocks.GetBDD());
//     Bdd result = blocks;
//     return result;
// }

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
    buildScreen(screen);

    std::cerr << screen;

    return 0;
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