#include "inputparser.h"

#include <iostream>


bool InputParser::parse(int argc, char** argv){

    static const char * opt_string = "i:o:s:t:d:N:G:vI:T:r:R:q:";
    int in;

    opterr = 0;
    try{
        while((in = getopt(argc, argv, opt_string)) != -1){
            switch(in){
                // input file
                case 'i':
                    readfile = true;
                    in_fn = optarg;
                    break;
                // output file
                case 'o':
                    save_positions = true;
                    out_fn = optarg;
                    break;
                // steps
                case 's':
                    steps = std::stoi(optarg);
                    break;
                // threshold for MAC
                case 't':
                    theta = std::stod(optarg);
                    break;
                // timestep
                case 'd':
                    dt = std::stod(optarg);
                    break;

                /////////
                case 'N':
                    n = std::stoi(optarg);
                    break;
                case 'G':
                    g = std::stod(optarg);
                    break;
                case 'v':
                    verb = true;
                    break;
                case 'I':
                    si = std::stoi(optarg);
                    break;
                case 'T':
                    out_time_fn = optarg;
                    clock = true;
                    break;
                case 'r':
                    save_tree = true;
                    out_tree_fn = optarg;
                    break;
                case 'R':
                    save_tree_size = true;
                    out_tree_size_fn = optarg;
                    break;
                case 'q':
                    summarize = true;
                    summary_fn = optarg;
                    break;
                default:
                    return false;
            }
        }
    }
    catch(std::exception const & e){
        return false;
    }
    return true;
}

double InputParser::grav_constant(){
    return g;
}

double InputParser::bh_approx_constant(){
    return theta;
}

bool InputParser::read_bodies(){
    return readfile;
}

std::string InputParser::in_file(){
    return in_fn;
}


int InputParser::n_bodies(){
    return n;
}

int InputParser::n_steps(){
    return steps;
}


double InputParser::time_step(){
    return dt;
}

bool InputParser::verbose(){
    return verb;
}

bool InputParser::write_positions(){
    return save_positions; 
}

std::string InputParser::out_file(){
    return out_fn;
}

int InputParser::sampling_interval(){
    return si;
}

bool InputParser::clock_run(){
    return clock;
}

std::string InputParser::out_time_file(){
    return out_time_fn;
}

bool InputParser::write_tree(){
    return save_tree; 
}

std::string InputParser::out_tree_file(){
    return out_tree_fn;
}

bool InputParser::write_tree_size(){
    return save_tree_size;
}

std::string InputParser::out_tree_size_file(){
    return out_tree_size_fn;
}

bool InputParser::write_summary(){
    return summarize;
}

std::string InputParser::out_sum_file(){
    return summary_fn;
}
