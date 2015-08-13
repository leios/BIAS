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
netlist diff_amp(netlist net, resistor inrp, resistor inrn);
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

    net.index++;
    
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


    net.index++;

    return net;
}

// Because we only want the differences between the voltages, we do not need
// to worry about different resistor values.
// Like the summing amplifier, its easier to read in resistors than ints and
// write them here.
netlist diff_amp(netlist net, resistor inrp, resistor inrn){

    opamp oa;
    // Note: No resistor def necessary since all resistor values are the same

    // First, let's write out the resistors we have coming in
    inrp.forw = net.index + 1;
    inrn.forw = net.index;
    net.str.append(" r" + to_string(inrp.back) + " " + to_string(inrp.forw) +" "
                   + to_string(inrp.value) + "k");
    net.str.append(" r" + to_string(inrn.back) + " " + to_string(inrn.forw) +" "
                   + to_string(inrn.value) + "k");

    // Now we need to create the rest of the differential amplifier circuit

    // r2 clone to ground
    net.str.append(" r" + to_string(inrp.forw) + " " + to_string(0) +" "
                   + to_string(inrp.value) + "k");

    // r1 clone to out
    net.str.append(" r" + to_string(inrn.forw) + " " + to_string(net.index + 2)
                   + " " + to_string(inrn.value) + "k");

    // opamp definitions and writing

    oa.inp = inrp.forw;
    oa.inn = inrn.forw;
    // Note +2 instead of +1 due to two inputs
    oa.out = net.index + 2;

    net.str.append(" e" + to_string(net.index) + " " + to_string(oa.out) + " 0 "
                   + to_string(oa.inp) + " " + to_string(oa.inn) + " 999k");

    net.index += 2;

    return net;
}

// This iteration of a sample and hold circuit will not have FET switches.
// To implement FET switches, just put on the output of oa1 for charging, and
// another parallel to the capacitor for discarging
netlist samhold(netlist net){

    opamp oa1, oa2;
    capacitor cap;

    // opamp1
    oa1.inp = 0;
    oa1.inn = net.index;
    oa1.out = net.index + 1;

    // opamp2
    oa2.inp = 0;
    oa2.inn = net.index + 1;
    oa2.out = net.index + 2;

    // capacitor
    cap.back = net.index + 1;
    cap.forw = 0;

    //appending to netlist
    // opamp
    net.str.append(" e"+to_string(net.index)+" "+to_string(oa1.out)+" 0 "
                   + to_string(oa1.inp) + " " + to_string(oa1.inn) + " 999k");

    net.str.append(" e"+to_string(net.index+1)+" "+to_string(oa2.out)+" 0 "
                   + to_string(oa2.inp) + " " + to_string(oa2.inn) + " 999k");

    // Resistor
    net.str.append(" c" + to_string(cap.back) + " " + to_string(cap.forw) + " "
                   + to_string(cap.value) + "k");



    net.index += 2;

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

