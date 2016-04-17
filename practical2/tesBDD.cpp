#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include <sylvan.h>
#include <sylvan_obj.hpp>

#include <iostream>

using namespace sylvan;

void BddPrint(Bdd b);
void BddGraphGenerate(Bdd b);

Bdd BDDEU(Bdd B1, Bdd B2, Bdd oldLfp);

VOID_TASK_0(simple_cxx)
{
    Bdd one = Bdd::bddOne(); // the True terminal
    Bdd zero = Bdd::bddZero(); // the False terminal

    Bdd x = Bdd::bddVar(0); // create a BDD variable x_0
    Bdd xP = Bdd::bddVar(1);// create a BDD variable x_1
    Bdd y = Bdd::bddVar(2); 
    Bdd yP = Bdd::bddVar(3);
    Bdd z = Bdd::bddVar(4); 
    Bdd zP = Bdd::bddVar(5);

    // transition
    //x and y = x * y
    Bdd R1 = !x * xP * y *= yP * z *= zP;
    Bdd R2 = x * xP * !y * yP * z *= zP;
    Bdd R3 = x *= xP * y *= yP * z *= !zP;
    Bdd R = R1+R2+R3;

    BddPrint(R1);
    BddGraphGenerate(R1);

    //properties
    Bdd init = !x * !y * !z;
    Bdd error = !x * y * !z;
    Bdd pay = y *= !z;
    Bdd goal = x * y * z;

    /*CTL formula
	AG(!error) = !EF (!(!error)) = !E(True U error)    
    	Lfp(Z) = error + (True * EX Z)
    */

    //calculate the formula
    Bdd Z0 = Bdd::bddZero(); // False
    Bdd Prev_Z0_R = Z0 * R;
    Bdd Z1 = error + Prev_Z0_R;
    Bdd Prev_Z1_R = Z1 * R;
    Bdd Z2 = error + Prev_Z1_R;
    Bdd Prev_Z2_R = Z2 * R;
    Bdd Z3 = error + Prev_Z2_R;
	
    printf("%d\n", Z2 == Z3);	// True -> the formula holds
	
    std::cout << "Start MD" << std::endl;
    Bdd result = BDDEU(R, error, Bdd::bddZero());
    std::cout << "END MD" << std::endl;
	
    std::cout << "Printing" << std::endl;
    BddPrint(result);
    BddGraphGenerate(result);
    std::cout << "END" << std::endl;
}

Bdd BDDEU(Bdd B1, Bdd B2, Bdd oldLfp){	
    Bdd lfp = B2 + B1 * oldLfp;

    if(lfp == oldLfp){
        return lfp;
    }
    
    return BDDEU(B1, B2, lfp);
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

void BddGraphGenerate(Bdd a){
	FILE * pFile;
	pFile = fopen ("testBDD.dot" , "w");
	if (pFile == NULL) 
		perror ("Error opening file");
	else
	{
		sylvan_fprintdot(pFile,a.GetBDD());
	}
	fclose (pFile);
}

VOID_TASK_1(_main, void*, arg)
{
    // Initialize Sylvan
    // With starting size of the nodes table 1 << 21, and maximum size 1 << 27.
    // With starting size of the cache table 1 << 20, and maximum size 1 << 20.
    // Memory usage: 24 bytes per node, and 36 bytes per cache bucket
    // - 1<<24 nodes: 384 MB
    // - 1<<25 nodes: 768 MB
    // - 1<<26 nodes: 1536 MB
    // - 1<<27 nodes: 3072 MB
    // - 1<<24 cache: 576 MB
    // - 1<<25 cache: 1152 MB
    // - 1<<26 cache: 2304 MB
    // - 1<<27 cache: 4608 MB
    sylvan_init_package(1LL<<22, 1LL<<26, 1LL<<22, 1LL<<26);

    // Initialize the BDD module with granularity 1 (cache every operation)
    // A higher granularity (e.g. 6) often results in better performance in practice
    sylvan_init_bdd(1);

    // Now we can do some simple stuff using the C++ objects.
    CALL(simple_cxx);

    // Report statistics (if SYLVAN_STATS is 1 in the configuration)
    sylvan_stats_report(stdout, 1);

    // And quit, freeing memory
    sylvan_quit();

    // We didn't use arg
    (void)arg;
}

int
main (int argc, char *argv[])
{
    int n_workers = 0; // automatically detect number of workers
    size_t deque_size = 0; // default value for the size of task deques for the workers
    size_t program_stack_size = 0; // default value for the program stack of each pthread

    // Initialize the Lace framework for <n_workers> workers.
    lace_init(n_workers, deque_size);

    // Spawn and start all worker pthreads; suspends current thread until done.
    lace_startup(program_stack_size, TASK(_main), NULL);

    // The lace_startup command also exits Lace after _main is completed.
    return 0;
    (void)argc; // unused variable
    (void)argv; // unused variable

}
