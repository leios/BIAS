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
*-----------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

using namespace std;

/*----------------------------------------------------------------------------//
* STRUCTURES AND FUNCTIONS
*-----------------------------------------------------------------------------*/


// Function to create an initial netlist grid
vector< vector<string> > create_grid(); 

// Function that transforms a 2d array (here a vector of a vector)
// into our netlist string
string write_netlist(vector< vector<string> > grid);

/*----------------------------------------------------------------------------//
* MAIN
*-----------------------------------------------------------------------------*/

int main(void){

    return 0;
}

/*----------------------------------------------------------------------------//
* SUBROUTINES
*-----------------------------------------------------------------------------*/

// Function to create an initial netlist grid 
// The grid will connect to other points every other vector position.
// This way, we can hold values in-between each connection point.
vector< vector<string> > create_grid(){
    vector< vector<string> > grid;

    return grid;
}

// Function that transforms a 2d array (here a vector of a vector)
// into our netlist string
string write_netlist(vector< vector<string> > grid){
    string netlist;

    return netlist;
}

