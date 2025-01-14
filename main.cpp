/* input/output */
#include <iostream>

/* math */
#include <cmath>

/* srand */
#include <stdlib.h>

/* STL types */
#include <vector>
#include <array>

/* MPI library */
#include <mpi.h>

/* Custom MPI structs */
#include "misc/mpi_types.h"

/* Body class */
#include "misc/body.h"

/* Reading and writing */
#include "misc/readwrite.h"

/* Tree building */
#include "tree/orb.h"
#include "tree/tree.h"
#include "tree/build_tree.h"

/* Input parsing */
#include "misc/inputparser.h"

/* Body generator */
#include "misc/gen_bodies.h"

#include <string>
#include <fstream>
#include <sstream>


int main(int argc, char * argv[]){
    
    int size, rank, tmax, N, nbodies;
    std::vector<Body> bodies;
    std::vector<pair<double, int> > splits;
    vector<pair<array<double, 2>, array<double, 2> > > bounds, other_bounds; 
    vector<pair<int, bool> > partners;
    vector<double> comp_time;
    double dt, min[2], max[2];
    double start_time, stop_time;
    InputParser ip;
    bool overwrite;
    
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Number of processes must be a power of 2 */
    if(fmod(log2(size), 1) != 0){
        if(rank == 0){
            std::cerr << "Error: Number of processes must be a power of 2!" << std::endl;
        }
        MPI_Finalize();
        return -1;
    }


    if(!ip.parse(argc, argv)){
        if(rank == 0){
            std::cout << "Error: Invalid command line input" << std::endl;
        }
        return -1; 
    }

    /* Initialize custom MPI structures */
    init_mpi_types();

    if(ip.read_bodies()){
        /* Read bodies from file */
        auto p = read_bodies(ip.in_file().c_str(), MPI_COMM_WORLD);
        bodies = p.first;
        N = p.second;
    }
    else{
        N = ip.n_bodies();
        nbodies = ip.n_bodies() / size;
        if (rank <= ip.n_bodies() % size - 1){
            nbodies++;
        }
        double lim = (double) 10 * N;
        bodies = generate_bodies(nbodies, {{-lim, -lim}}, {{lim, lim}}, rank);
    }

    if (rank == 0) {
        // Get the size of N
        int N;
        std::ifstream in_file;
        in_file.open(ip.in_file());
        std::string line;
        std::getline(in_file, line);
        std::istringstream iss(line);
        iss >> N;
        in_file.close();

        // Write the size of N
        std::ofstream out_file;
        out_file.open(ip.out_file());
        out_file << N << std::endl;
        out_file.close();
    }

    // /* Write initial positions to file */
    // overwrite = true;
    // if(ip.write_positions()){
    //     write_bodies(ip.out_file().c_str(), bodies, MPI_COMM_WORLD, overwrite);
    // }

    
    tmax = ip.n_steps(); // number of time steps
    dt = ip.time_step(); // time step
    
    if(ip.clock_run()){
        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
    }

    for(int t = 0; t < tmax; t++){

        /* Reset variables */
        bounds.clear();
        other_bounds.clear();
        partners.clear();
        
        /* Domain composition and transfer of bodies */
        global_minmax(bodies, min, max);
        orb(bodies, bounds, other_bounds, partners, min, max, rank, size);

        
        /* Build the local tree */
        Tree tree(min, max, ip.bh_approx_constant());
        build_tree(bodies, bounds, other_bounds, partners, tree, rank);

        /* Compute forces */
        std::vector<array<double, 2> > forces;
        for(Body & b : bodies){
            /* time the computation */
            double start_time = MPI_Wtime();
            array<double, 2> f = tree.compute_force(&b);
            /* update the workload for the body */
            b.work = MPI_Wtime() - start_time;
            forces.push_back(f);
        }
        
        /* Update positions */
        for (int i = 0; i < bodies.size(); i++) {
            Body & b = bodies[i];
            for(int c = 0; c < 2; c++){
                b.vel[c] = b.vel[c] + forces[i][c] * dt / b.m;
                b.pos[c] = b.pos[c] + b.vel[c] * dt;
            }
        }
    }


    /* Stop the time */
    if(ip.clock_run()){
        MPI_Barrier(MPI_COMM_WORLD);
        stop_time = MPI_Wtime(); 
    }

    /* Write running time */
    if(ip.clock_run()){
        if(rank == 0){
            // write_to_file(ip.out_time_file().c_str(), stop_time - start_time, overwrite);
            printf("%f\n", stop_time - start_time);
        }
    }

    /* Write positions */
    if(ip.write_positions()){
        write_bodies(ip.out_file().c_str(), bodies, MPI_COMM_WORLD, false);
    }

    /* Finalize */
    free_mpi_types();
    MPI_Finalize();
   
    if(ip.write_summary()){
        if(rank == 0){
            write_summary(ip, N, size);
        }
    }
    

}

