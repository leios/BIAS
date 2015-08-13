/*-------------netlist_gen.cpp------------------------------------------------//
*
*              netlist generator
*
* Purpose: The BIAS project intends to re-implement the TrueNorth architechture
*          in analog electronics. For this purpose, we will need to create a 
*          simple script that will create a netlist to be read into a SPICE 
*          simulator (or similar software that is compatable with the netlist 
*          format. 
*
*   Notes: This could be written a little cleaner in another language, but...
*          We need to determine the appropriate analog inputs for this file
*
*          To compile, use the following command:
*              g++ netlist_gen.cpp -std=c++11 -o genet
*
*  PEMDAS: We will place everything on a grid. The components we need are:
*              1. inverting amp
*              2. summing amp
*              3. differential amp
*              4. sample and hold
*
*          The grid will then be enumerated appropriately by filling the grid
*              points with integers and counting.
* 
*          Finally, this will be written out to a file. 
*
*-----------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

/*----------------------------------------------------------------------------//
* STRUCTURES AND FUNCTIONS
*-----------------------------------------------------------------------------*/

struct resistor{

    double value;
    int forw, back;
};

struct capacitor{

    double value;
    int forw, back;

};

// opamps are weird in spice. They have 4 extremities:
// out, ground, in+, in-
struct opamp{
    int inp, inn, out; 
};

// struct to pass output and index between functions
struct netlist{
    string str;
    int index;
};

// These functions will all take the current netlist and count and append the
// appropriate variables to it. These will be used in the larger f(x)'s below
netlist inv_amp(netlist net);
netlist sum_amp(netlist net, vector<resistor> connections);
netlist diff_amp(netlist net, int inp, int inn);
netlist samhold(netlist net);

// These functions will take care of connections and such within the core
netlist neuron(netlist net);
netlist junction(netlist net);
netlist connectome(netlist net);

// Quick function to write the netlist to a file
void write_netlist(netlist net, ofstream &output);

/*----------------------------------------------------------------------------//
* MAIN
*-----------------------------------------------------------------------------*/

int main(void){

    return 0;
}

/*----------------------------------------------------------------------------//
* SUBROUTINES
*-----------------------------------------------------------------------------*/

// These functions will all take the current netlist and count and append the
// appropriate variables to it. These will be used in the larger f(x)'s below

netlist inv_amp(netlist net){
    resistor res, res_2;
    opamp oa;

    // Setting initial values
    res.value = res_2.value = 1000;

    // Setting resistors in place
    res.back = net.index;
    res.forw = net.index + 1;
    res_2.back = net.index + 1;
    res_2.forw = net.index + 2;

    // Setting opamp up
    oa.inp = 0;
    oa.inn = net.index + 1;
    oa.out = net.index + 2;

    // appending to netlist

    // opamp
    net.str.append(" e" + to_string(net.index) + " " + to_string(oa.out) + " 0 "
                   + to_string(oa.inp) + " " + to_string(oa.inn) + " 999k");

    // R1
    net.str.append(" r" + to_string(res.back) + " " + to_string(res.forw) + " "
                   + to_string(res.value) + "k");

    // R2
    net.str.append(" r" + to_string(res_2.back) + " " + to_string(res_2.forw) +
                   " " + to_string(res.value) + "k");
    
    return net;
}

// Write out summing amp, including preceding resistors here.
// This function is simple because we are assuming we are reading in the R vals
netlist sum_amp(netlist net, vector<resistor> connections){

    opamp oa;
    resistor res;

    // For this function, it is assumed that all the incoming resistors are
    // of the same value

    for (auto &r : connections){
        r.forw = net.index;
        net.str.append(" r" + to_string(r.back) + " " + to_string(r.forw) +
                       " " + to_string(res.value) + "k");
    }

    //Setting up op amp
    oa.inp = 0;
    oa.inn = net.index;
    oa.out = net.index + 1;

    // setting up resistor
    res.back = net.index;
    res.forw = net.index + 1;
    res.value = connections[0].value;

    // appending to netlist

    // opamp
    net.str.append(" e" + to_string(net.index) + " " + to_string(oa.out) + " 0 "
                   + to_string(oa.inp) + " " + to_string(oa.inn) + " 999k");

    // Resistor
    net.str.append(" r" + to_string(res.back) + " " + to_string(res.forw) + " "
                   + to_string(res.value) + "k");


    return net;
}

netlist diff_amp(netlist net, int inp, int inn){
    return net;
}

netlist samhold(netlist net){
    return net;
}


// These functions will take care of connections and such within the core
netlist neuron(netlist net){
    return net;
}

netlist junction(netlist net){
    return net;
}

netlist connectome(netlist net){
    return net;
}


// Quick function to write the netlist to a file
void write_netlist(netlist net, ofstream &output){
}

