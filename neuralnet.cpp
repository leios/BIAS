/*-------------neuralnet.cpp--------------------------------------------------//
*
*              Neural Net -- A computational model of a neural network
*
* Purpose: Before building a purely analog circuit with the BIAS project, we
*          decided to first develop a computational model of how we expect the
*          circuit to work in a very algorithmic way. 
*
*   Notes: Even though this logic will later be used by the primary BIAS code, 
*          very little analog logic was used in the following script. Rather, 
*          this script was meant to strengthen our own understanding of our 
*          neural system.
*
*          Please let me know if you need any further information!
*              James Schloss
*
*-----------------------------------------------------------------------------*/

#include <iostream>
#include <vector>
#include <time.h>
#include <cstdlib>
#include <cmath>

/*----------------------------------------------------------------------------//
* STRUCTURES / FUNCTIONS
*-----------------------------------------------------------------------------*/

using namespace std;

const int n = 5;

// structure for the synaptic points
struct synapse{
    double weight;
    vector <bool> prefire;
};

// structure for synaptic crossbars
struct grid{
    synapse array[n][n];
};

// function for filling our synaptic crossbar
grid fill_grid(int time_window);

// function for hebbian learning
grid hebbian(grid data);

/*----------------------------------------------------------------------------//
* MAIN
*-----------------------------------------------------------------------------*/

int main(void){

    srand(time(NULL));

    int time_window = 10;

    grid data = fill_grid(time_window);

    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            cout << data.array[i][j].weight << '\t';
        }
        cout << endl << endl;

       for (int j = 0; j < n; j++){
            for (int k = 0; k < time_window; k++){
                cout << data.array[i][j].prefire[k];
            }
 
            cout << endl;
        } 

        cout << endl;

    }
    return 0;
}

/*----------------------------------------------------------------------------//
* SUBROUTINES
*-----------------------------------------------------------------------------*/

// function for filling our synaptic crossbar
grid fill_grid(int time_window){
    grid data;

    // creating initial weights and firing pattern in time window
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            data.array[i][j].weight = 1.0;
            for (int k = 0; k < time_window; k++){
                data.array[i][j].prefire.push_back(round(rand() % 10 / 10.0));
            }
        }
    }

    return data;
}

// function for hebbian learning
grid hebbian(grid data){
    grid hebb_data;

    return hebb_data;
}
