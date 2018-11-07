# bode2spice
The <b>bode2spice</b> tool reads data from Comma Separated Values file, exported by the Omicron Labs Bode Analyzer Suite or similar source, containing complex impedance or transfer function, plots it for inspection and generates LTSpice compatible component to ease embedding into simulation.

It uses Qt5 https://www.qt.io/ and QCustomplot https://www.qcustomplot.com/ libraries.

Qt must be installed on your system as a prerequisite, QCustomplot sources are included here.

To build it follow this steps:

    mkdir build_directory
    cd build_directory
    qmake bode2spice.pro
    make
    
Subdirectory data_example contains one input file example.csv, sample output files and a test schematic in LTSpice format which uses the components/models. You can see how the interaction works.

Enjoy your measurement and simulation.
