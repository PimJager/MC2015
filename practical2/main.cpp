#include <stdio.h>

#include <sylvan.h>
#include <sylvan_obj.hpp>

void setUpSylvan(){
    lace_init(0, 0); //auto #workers and task_deque
    lace_startup(0, NULL, NULL); //auto stack size
    LACE_ME;
    sylvan_init_package(1LL<<21, 1LL<<27, 1LL<<20, 1LL<<26);
    sylvan_init_bdd(6);
}

int main(){
    setUpSylvan();
    return 0;
}