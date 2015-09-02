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
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

/*----------------------------------------------------------------------------//
* STRUCTURES AND FUNCTIONS
*-----------------------------------------------------------------------------*/

// precision settings
template <typename T>
std::string to_string_with_precision(const T a_value, const int p)
{
    std::ostringstream out;
    out << std::setprecision(p) << a_value;
    return out.str();
}

//Grid Size
const int n = 5, p = 4;

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
    int rcount, ccount, dcount, ocount;
};

// struct to hold the connection data in junctions
// synapse[i][j1...jn] is the vector of ints for summing amp
// We can connect everything up to the axon[n]. Just re-use those values
struct connectome{
    int axon[n], synapse[n][n], hillock[n];
};

// struct to pass connectome and netlist from functions
struct connet{
    connectome conn;
    netlist nl;
};

// These functions will all take the current netlist and count and append the
// appropriate variables to it. These will be used in the larger f(x)'s below
netlist inv_amp(netlist net, double rval);
netlist sum_amp(netlist net, vector<resistor> connections);
netlist diff_amp(netlist net, resistor inrp, resistor inrn);
netlist samhold(netlist net, double cval);
netlist multiplier(netlist net, int in1, int in2, double rval);

// These functions append the appropriate variables to the netlist string
netlist wres(netlist net, resistor r);
netlist wcap(netlist net, capacitor c);
netlist wop(netlist net, opamp oa);
netlist wdi(netlist net, diode d);

// These functions will take care of connections and such within the core
connet neuron(connet all, voltage thresh, double rval, double cval, int hill);
connet junction(connet all, int axn, int hill, double rval, double cval);

// This will generate connectome and write final netlist to file
void write_netlist(netlist net, ofstream &output, double rval, double cval);

/*----------------------------------------------------------------------------//
* MAIN
*-----------------------------------------------------------------------------*/

int main(void){

    // creating all the necessary parameters
    double rval = 1000, cval = 1000;

    std::ofstream output("out.net", std::ofstream::out);

    netlist net = {};

    write_netlist(net, output, rval, cval);

    output.close();

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
    net = wop(net, oa);

    // R1
    net = wres(net, res);

    // R2
    net = wres(net, res_2);

    net.index += 2;
    
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
        net = wres(net, r);
        //net.str.append("SUM CHECK");
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
    net = wop(net, oa);

    // Resistor
    net = wres(net, res);

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
    net = wres(net, inrp);
    net = wres(net, inrn);

    // Now we need to create the rest of the differential amplifier circuit

    // r2 clone to ground
    inrp.forw = 0;
    net = wres(net, inrp);

    // r1 clone to out
    inrn.back = inrn.forw;
    inrn.forw = net.index + 2;
    net = wres(net, inrn);

    // opamp definitions and writing
    oa.inp = inrp.forw;
    oa.inn = inrn.forw;

    // Note +2 instead of +1 due to two inputs
    oa.out = net.index + 2;

    net = wop(net, oa);

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
    net = wop(net, oa1);
    net = wop(net, oa2);

    // capacitor
    net = wcap(net, cap);

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
    r4.value = rval;

    // resistor 5
    r5.back = r4.forw;
    r5.forw = r5.back + 1;
    r5.value = rval;

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
    r6.value = rval;

    //opamp 4
    oa4.inp = 0;
    oa4.inn = r6.back;
    oa4.out = r6.forw;

    // resistor 7
    r7.back = oa4.out;
    r7.forw = r7.back + 1;
    r7.value = rval;

    // resistor 8
    r8.back = r7.forw;
    r8.forw = r8.back + 1;
    r8.value = rval;

    // resistor 9
    r9.back = v1;
    r9.forw = r8.back;
    r9.value = rval;

    // resistor 10
    r10.back = v2;
    r10.forw = r8.back;
    r10.value = rval;

    // opamp 5
    oa5.inp = 0;
    oa5.inn = r8.back;
    oa5.out = r8.forw;

    // Let's assume everything before now is fine. Now we need to put it all in
    // the netlist

    // opamps
    net = wop(net, oa1);
    net = wop(net, oa2);
    net = wop(net, oa3);
    net = wop(net, oa4);
    net = wop(net, oa5);

    // Resistors
    net = wres(net, r1);
    net = wres(net, r2);
    net = wres(net, r3);
    net = wres(net, r4);
    net = wres(net, r5);
    net = wres(net, r6);
    net = wres(net, r7);
    net = wres(net, r8);
    net = wres(net, r9);
    net = wres(net, r10);

    // diodes
    net = wdi(net, d1);
    net = wdi(net, d2);
    net = wdi(net, d3);

    // I think it's 7 from v1
    net.index += 7;

    return net;
}

// Now for just a connecting wire
// technically a resistor with an incredibly small resistor value... 
// we are technically trying 0 first...
// I DON'T KNOW IF 0 WORKS FOR RESISTORS!
netlist wres(netlist net, resistor r){

    net.str.append("r" + to_string_with_precision(net.rcount, p) + " " 
                  + to_string_with_precision(r.back, p)+ " "
                  + to_string_with_precision(r.forw, p) + " " 
                  + to_string_with_precision(r.value, p) + "k" + "\n");

    net.rcount++;

    return net;
}

netlist wcap(netlist net, capacitor c){

    net.str.append("c" + to_string_with_precision(net.ccount, p) + " " 
                  + to_string_with_precision(c.back, p) +  " "
                  + to_string_with_precision(c.forw, p) + " " 
                  + to_string_with_precision(c.value, p) + "u" + "\n");

    net.ccount++;

    return net;
}

// opamps are defined by their negative input
netlist wop(netlist net, opamp oa){

    net.str.append("e"+to_string_with_precision(net.ocount, p) + " "
                  + to_string_with_precision(oa.out, p) + " 0 "
                  + to_string_with_precision(oa.inp, p) + " " 
                  + to_string_with_precision(oa.inn, p) + " 999k" + "\n");

    net.ocount++;

    return net;
}

netlist wdi(netlist net, diode d){

    net.str.append("d" + to_string(net.dcount) + " " + to_string(d.back) + 
                  + " " + to_string(d.forw) + " mod1" + "\n");

    net.dcount++;

    return net;
}

// These functions will take care of connections and such within the core
// PEMDAS: sum_amp-> diff_amp -> sum_amp -> hillock
connet neuron(connet all, voltage thresh, double rval, double cval, int hill){

    netlist net = all.nl;
    connectome grid = all.conn;
    resistor dr1, dr2, sr1, sr2, w;
    voltage v6;
    v6.forw = 2;
    v6.value = 6;

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

    // Now we need to multiply this value by 6-ish
    net = multiplier(net, net.index, v6.forw, rval);

    // This is the first output to the axon.
    // not sure about output!
    w.back = net.index;
    w.forw = grid.axon[hill];
    w.value = 0;

    net = wres(net, w);

    // The summing amplifier will sum with the output of the samhold like in the
    // junction

    // SR1
    sr1.back = net.index;
    sr1.forw = net.index + 1;
    sr1.value = rval;

    // SR2
    sr2.back = net.index;
    sr2.back = net.index + 4;
    sr2.value = rval;

    scharge[0] = sr1;
    scharge[1] = sr2;

    net = sum_amp(net, scharge);

    // sample and hold
    net = samhold(net, cval);

    // This is the second output
    // redefine wire
    w.back = net.index;
    w.forw = grid.axon[hill];
    w.value = 0;


    net = wres(net, w);

    all.nl = net;
    all.conn = grid;

    return all;
}

// PEMDAS: sum_amp -> samhold -> diff_amp -> multiplier -> neuron
// update connectome
// UNTESTED
connet junction(connet all, int axn, int hill, double rval, double cval){

    netlist net = all.nl;
    connectome grid = all.conn;
    vector <resistor> set_1(2);
    resistor r1, r2, dr1, dr2;

    // setting up set_1 of resistors for first summing amp
    // R1
    r1.back = grid.axon[axn];
    r1.forw = r1.back + 1;
    r1.value = rval;

    // R2
    r2.forw = r1.forw + 4; // after the sample and hold
    r2.back = r1.forw;
    r2.value = rval;

    set_1[0] = r1;
    set_1[1] = r2;
    
    // resetting index
    //net.index = r1.forw;

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

    net = multiplier(net, grid.axon[axn], net.index, rval);

    grid.synapse[hill][axn] = net.index;
    cout << grid.synapse[hill][axn] << '\n';

    all.nl = net;
    all.conn = grid;
    return all;
}


// This will generate connectome and write fial netlist to file
void write_netlist(netlist net, ofstream &output, double rval, double cval){

    // generate connectome
    connet all;
    all.nl = net;
    connectome grid = all.conn;

    all.nl.str.append("BIAS Circuit \n");

    // starting with determining the numbers for hillocks and axons
    for (int i = 0; i < n; i++){
        all.conn.axon[i] = 3 + i;
        all.conn.hillock[i] = n + i + 3;
    }

    all.nl.index += 2 * n + 2;

    // let's create our voltage 
    voltage thresh;

    thresh.forw = 1;
    thresh.value = 10;

    // now we need to go through and define each j(x)
    for (int hill = 0; hill < n; hill++){
        for (int axn = 0; axn < n; axn++){
            all = junction(all, axn, hill, rval, cval);
            cout << all.conn.synapse[hill][axn] << '\n';

        }

        all = neuron(all, thresh, rval, cval, hill);
    }

    // now we need to append the voltages and such
    // thresh
    all.nl.str.append("v1 " + to_string_with_precision(thresh.forw, p) + " dc "
                       + to_string_with_precision(thresh.value, p) + "\n");

    // Adding the model for the diodes
    all.nl.str.append(".model mod1 d \n");
    all.nl.str.append(" .end");

    // Actual writing to a file is easy
    output << all.nl.str << '\n';
}

