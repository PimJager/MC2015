#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

#include <sylvan.h>
#include <sylvan_obj.hpp>

enum class Field {EMPTY, WALL, MAN, BLOCK, GOAL, BLOCK_ON_GOAL, MAN_ON_GOAL};
static std::vector<std::string> readfile(std::istream& input);
static size_t max_line_length(std::vector<std::string>& lines);
static std::vector<std::vector<Field>> parse_screen(std::vector<std::string> lines, size_t rows, size_t cols);
template<typename T>
static std::ostream& operator<<(std::ostream& stream, std::vector<std::vector<T>>& matrix);

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