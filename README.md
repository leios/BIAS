BIAS
====
Biologically-Inspired Architecture Simulator

This project follows from the developments of IBM's TrueNorth chip and Compass
simulator for the TrueNorth chip. The idea is relatively simple: to build an
analog cognitive computer, with some (if not all) the bells and whistles of 
TrueNorth. There are plenty of benefits for analog computation when compared to
digital; however, I will not go into those here. 

I would like to describe the development and concepts of this project:

1.  We intend to theoretically develop an analog circuit that is both scalable
    and can perform similar functions to TrueNorth.

2.  We intend to write a script that translates our circuit into a netlist to be
    read by SPICE or a SPICE-like program.

3.  We intend to work with SPICE code to run on our local computing cluster, 
    Sango. 

----

This project has been performed as a rotation project for OIST under the 
direction of Jeff Wickens. It should be noted that due to the time constraint
inherent in rotations and my own lack of understanding of neural systems, this
project may be continued in the future under the direction of another OIST
student. In such a case, please contact me (James Schloss) at one of the 
following e-mail addresses:

    james.schloss@oist.jp
    jrs.schloss@gmail.com

Thank you for reading, and I hope this project benefits you in some way.

----

The following is a list of files and their intended use:

    neuralnet.cpp
        This is a computational model of the neural network, written to better
        understand our system.

    netlist_gen.cpp
        This is a generator for a SPICE netlist. By running this, you will 
        generate the netlist for a single neurosynaptic core. Note that this is 
        a relatively simple netlist and under active development.
