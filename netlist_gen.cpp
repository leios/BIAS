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

//Grid Size
const int n = 5;

struct resistor{

    double value;
    int forw, back;
};

struct capacitor{

    double value;
    int forw, back;

};

struct diode{
    int forw, back;
};

// dc voltage
struct voltage{
    double value;
    int forw;
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

// struct to hold the connection data in junctions
// synapse[i][j1...jn] is the vector of ints for summing amp
// We can connect everything up to the axon[n]. Just re-use those values
struct connectome{
    int axon[n], synapse[n][n], hillock[n];
};

// These functions will all take the current netlist and count and append the
// appropriate variables to it. These will be used in the larger f(x)'s below
netlist inv_amp(netlist net, double rval);
netlist sum_amp(netlist net, vector<resistor> connections);
netlist diff_amp(netlist net, resistor inrp, resistor inrn);
netlist samhold(netlist net, double cval);
netlist multiplier(netlist net, resistor in1, resistor in2, double rval);
netlist wire(netlist net, int p1, int p2);

// These functions will take care of connections and such within the core
netlist neuron(netlist net, connectome grid, voltage threash, double rval,
               double cval, int hill);
netlist junction(netlist net, connectome &grid, int axn, int hill, double rval, 
                 double cval);

// This will generate connectome and write final netlist to file
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

netlist inv_amp(netlist net, double rval){
    resistor res, res_2;
    opamp oa;

    // Setting initial values
    res.value = res_2.value = rval;

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
netlist samhold(netlist net, double cval){

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
    cap.value = cval;

    // appending to netlist
    // opamp
    net.str.append(" e"+to_string(net.index)+" "+to_string(oa1.out)+" 0 "
                   + to_string(oa1.inp) + " " + to_string(oa1.inn) + " 999k");

    net.str.append(" e"+to_string(net.index+1)+" "+to_string(oa2.out)+" 0 "
                   + to_string(oa2.inp) + " " + to_string(oa2.inn) + " 999k");

    // capacitor
    net.str.append(" c" + to_string(cap.back) + " " + to_string(cap.forw) + " "
                   + to_string(cap.value) + "k");



    net.index += 2;

    return net;
}

// This function requires 5 oas, 3 diodes, and 10 resistors
// We are reading in ints insead of resistors because of how this circuit works
// Note: We may need to figre out a way to get the inputs positive and such
netlist multiplier(netlist net, int v1, int v2, double rval){

    opamp oa1, oa2, oa3, oa4, oa5;
    diode d1, d2, d3;
    resistor r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;

    // resistor 1
    r1.back = v1;
    r1.forw = v1+1;
    r1.value = rval;

    // resistor 2
    r2.back = v2;
    r2.forw = v2+1;
    r2.value = rval;

    // diode 1
    d1.back = r2.forw;
    d1.forw = d1.back + 1;

    // opamp 1
    oa1.inp = 0;
    oa1.inn = r1.forw;
    oa1.out = d1.forw;

    // resistor 3
    r3.back = oa1.out;
    r3.forw = r3.back + 1;
    r3.value = rval;

    // diode 2
    d2.back = r2.forw;
    d2.forw = d2.back + 1;

    // opamp 2
    oa2.inp = 0;
    oa2.inn = r2.forw;
    oa2.out = d2.forw;

    // resistor 4
    r4.back = oa2.out;
    r4.forw = r3.forw;

    // resistor 5
    r5.back = r4.forw;
    r5.forw = r5.back + 1;

    // opamp 3
    oa3.inp = 0;
    oa3.inn = r4.forw;
    oa3.out = r5.forw;

    // diode 3
    d3.back = oa3.out;
    d3.forw = d3.back + 1;

    // resistor 6
    r6.back = d3.forw;
    r6.forw = r6.back + 1;

    //opamp 4
    oa4.inp = 0;
    oa4.inn = r6.back;
    oa4.out = r6.forw;

    // resistor 7
    r7.back = oa4.out;
    r7.forw = r7.back + 1;

    // resistor 8
    r8.back = r7.forw;
    r8.forw = r8.back + 1;

    // resistor 9
    r9.back = v1;
    r9.forw = r8.back;

    // resistor 10
    r10.back = v2;
    r10.forw = r8.back;

    // opamp 5
    oa5.inp = 0;
    oa5.inn = r8.back;
    oa5.out = r8.forw;

    // Let's assume everything before now is fine. Now we need to put it all in
    // the netlist

    // opamps
    net.str.append(" e"+to_string(oa1.inn)+" "+to_string(oa1.out)+" 0 "
                   + to_string(oa1.inp) + " " + to_string(oa1.inn) + " 999k");

    net.str.append(" e"+to_string(oa2.inn)+" "+to_string(oa2.out)+" 0 "
                   + to_string(oa2.inp) + " " + to_string(oa2.inn) + " 999k");

    net.str.append(" e"+to_string(oa3.inn)+" "+to_string(oa3.out)+" 0 "
                   + to_string(oa3.inp) + " " + to_string(oa3.inn) + " 999k");

    net.str.append(" e"+to_string(oa4.inn)+" "+to_string(oa4.out)+" 0 "
                   + to_string(oa4.inp) + " " + to_string(oa4.inn) + " 999k");

    net.str.append(" e"+to_string(oa5.inn)+" "+to_string(oa5.out)+" 0 "
                   + to_string(oa5.inp) + " " + to_string(oa5.inn) + " 999k");


    // Resistors
    net.str.append(" r" + to_string(r1.back) + " " + to_string(r1.forw) + " "
                   + to_string(r1.value) + "k");

    net.str.append(" r" + to_string(r2.back) + " " + to_string(r2.forw) + " "
                   + to_string(r2.value) + "k");

    net.str.append(" r" + to_string(r3.back) + " " + to_string(r3.forw) + " "
                   + to_string(r3.value) + "k");

    net.str.append(" r" + to_string(r4.back) + " " + to_string(r4.forw) + " "
                   + to_string(r4.value) + "k");

    net.str.append(" r" + to_string(r5.back) + " " + to_string(r5.forw) + " "
                   + to_string(r5.value) + "k");

    net.str.append(" r" + to_string(r6.back) + " " + to_string(r6.forw) + " "
                   + to_string(r6.value) + "k");

    net.str.append(" r" + to_string(r7.back) + " " + to_string(r7.forw) + " "
                   + to_string(r7.value) + "k");

    net.str.append(" r" + to_string(r8.back) + " " + to_string(r8.forw) + " "
                   + to_string(r8.value) + "k");

    net.str.append(" r" + to_string(r9.back) + " " + to_string(r9.forw) + " "
                   + to_string(r9.value) + "k");

    net.str.append(" r" + to_string(r10.back) + " " + to_string(r10.forw) + " "
                   + to_string(r10.value) + "k");


    // diodes
    net.str.append(" d" + to_string(d1.back) + " " + to_string(d1.forw));

    net.str.append(" d" + to_string(d2.back) + " " + to_string(d2.forw));

    net.str.append(" d" + to_string(d3.back) + " " + to_string(d3.forw));

    // I think it's 7 from v1
    net.index += 7;

    return net;
}

// Now for just a connecting wire
// technically a resistor with an incredibly small resistor value... 
// we are technically trying 0 first...
// I DON'T KNOW IF 0 WORKS FOR RESISTORS!
netlist wire(netlist net, int p1, int p2){

    resistor test;
    test.forw = p2;
    test.back = p1;
    test.value = 0;

    net.str.append(" r" + to_string(test.back) + " " + to_string(test.forw)+" "
                   + to_string(test.value) + "k");


    return net;
}



// These functions will take care of connections and such within the core
// PEMDAS: sum_amp-> diff_amp -> sum_amp -> hillock
netlist neuron(netlist net, connectome grid, voltage thresh, double rval,
               double cval, int hill){

    resistor dr1, dr2, sr1, sr2;

    // first we need to create our vector of resistors
    vector <resistor> charge(n), scharge(2);

    // index is fixed in sum_amp
    for (int i = 0; i < n; i++){
        charge[i].value = rval;
        charge[i].back = grid.synapse[hill][i];
        charge[i].forw = net.index;
    }

    net = sum_amp(net, charge);

    // Now we need to throw the output of the sum_amp into a diff_amp with 
    // the threshhold voltage

    // DR1
    dr1.back = thresh.forw;
    dr1.forw = thresh.forw + 1;
    dr1.value = rval;

    // DR2
    dr2.back = net.index;
    dr2.back = net.index + 1;
    dr2.value = rval;
    
    net = diff_amp(net, dr1, dr2); 

    // This is the first output to the axon.
    // not sure about output!
    net = wire(net, net.index, grid.axon[hill]);

    // The summing amplifier will sum with the output of the samhold like in the
    // junction

    // SR1
    sr1.back = net.index;
    sr1.forw = net.index + 1;
    sr1.value = rval;

    // DR2
    sr2.back = net.index;
    sr2.back = net.index + 4;
    sr2.value = rval;

    scharge[0] = sr1;
    scharge[1] = sr2;

    net = sum_amp(net, scharge);

    // sample and hold
    net = samhold(net, cval);

    // This is the second output
    net = wire(net, net.index, grid.hillock[hill]);

    return net;
}

// PEMDAS: sum_amp -> samhold -> diff_amp -> multiplier -> neuron
// update connectome
// UNTESTED
netlist junction(netlist net, connectome &grid, int axn, int hill, double rval, 
                 double cval){

    vector <resistor> set_1(2);
    resistor r1, r2, dr1, dr2, mr1, mr2;

    // setting up set_1 of resistors for first summing amp
    // R1
    r1.back = axn;
    r1.forw = axn + 1;
    r1.value = rval;

    // R2
    r2.back = r1.forw + 4; // after the sample and hold
    r2.forw = r1.forw;
    r2.value = rval;

    set_1[0] = r1;
    set_1[1] = r2;
    
    // resetting index
    net.index = r1.forw;

    // summing amp
    net = sum_amp(net, set_1);

    // sample and hold
    net = samhold(net, cval);

    // Set up resistors for differential ampl with hillock
    // DR1
    dr1.back = hill;
    dr1.forw = hill + 1;
    dr1.value = rval;

    // DR2
    dr2.back = net.index; // after the sample and hold
    dr2.forw = net.index + 1;
    dr2.value = rval;

    net.index++;

    // differential amplifier
    net = diff_amp(net, dr1,dr2);

    net = multiplier(net, axn, net.index, rval);

    return net;
}


// This will generate connectome and write fial netlist to file
void write_netlist(netlist net, ofstream &output){
}

