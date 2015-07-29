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

const int n = 5, tw = 10;

// structure for synaptic crossbars
struct grid{
    double weight[n][n];
    vector <vector <bool> > prefire;
    vector <vector <bool> > postfire;
};

// function for filling our synaptic crossbar
grid fill_grid();

// function for hebbian learning / weight alteration
grid hebbian(grid data, double timestep, double tau);

// function for neuron charge collection
grid neurosum(grid data, vector <double> thresh, double tau,
                       double timestep);

/*----------------------------------------------------------------------------//
* MAIN
*-----------------------------------------------------------------------------*/

int main(void){

    srand(time(NULL));

    vector <double> thresh;

    double tau = 0.2, timestep = 0.1;

    grid data = fill_grid();

    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            cout << data.weight[i][j] << '\t';
        }
        cout << endl << endl;

        for (int k = 0; k < tw; k++){
            cout << data.prefire[i][k];
        }
        cout << endl;


    }

    for (int i = 0; i < n; i ++){
        thresh.push_back((rand() % 1000 * 0.001) * 10 * n);
    }

    data = neurosum(data, thresh, tau, timestep);


    cout << "Here are our sums: " << endl;
    for (int i = 0; i < n; i++){
        cout << data.postfire[0][i] << endl;
    }

    return 0;
}

/*----------------------------------------------------------------------------//
* SUBROUTINES
*-----------------------------------------------------------------------------*/

// function for filling our synaptic crossbar
grid fill_grid(){
    grid data;
    vector<bool> rn(n,false);

    // creating initial weights and firing pattern in time window
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            data.weight[i][j] = rand() % 1000 * 0.005;
        }
    }

    for (int i = 0; i < tw; i++){
        for (int j = 0; j < n; j++){
            rn[j] = round(rand() % 10 / 10.0);
        }
        data.prefire.push_back(rn);
    }

    return data;
}

// function for hebbian learning / weight alteration
// For now, we will assume that each output neuron is attached directly to 
// its corresponding row. Sure, this is not an ideal situation, but... meh.
//
// The initial firing pattern of everything will be random. 
// Because of the way hebbian learning works, we will update the weights twice:
//     Once after the postsynaptic neuron fires (+weight change)
//     Again after the presynaptic neuron fires (-weight change)

grid hebbian(grid data, double timestep, double tau){
/*
    double history, rn;

    // Following above, positive first, then negative

    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){

            history = 0;

            // positive
            for (int k = 0; k < tw; k++){
                if (data.prefire[k] == 1){
                    history += tw - k;
                }
            }

            // Here we are updating the prefire, we will need to change the 
            // weights negatively.
            rn = round(rand() % 10 / 10.0);

            data.prefire.erase(data.prefire.begin());
            data.prefire.push_back(rn);
        }
    }
*/
    return data;
}

// function for neuron charge collection
grid neurosum(grid data, vector <double> thresh, double tau,
                       double timestep){

    vector <bool> sum(tw, false);
    vector<double> cumulative(n, false);
    double pt_cumulative = 0, t;

    for (int i = 0; i < n; i++){

        cumulative[i] = 0;

        for (int j = 0; j < n; j++){

            pt_cumulative = 0;

            for (unsigned int k = 0; k < data.prefire.size(); k++){
                t = k * timestep;
                pt_cumulative += data.prefire[i][k] * exp(-(t * tau));
            }

            cumulative[i] += pt_cumulative;

        }
    }

    for (int i = 0; i < n; i++){
        if (cumulative[i] > thresh[i]) {
            sum[i] = 1;
        }
        else{
            sum[i] = 0;
        }

    }

    data.postfire.push_back(sum);

    return data;
}
