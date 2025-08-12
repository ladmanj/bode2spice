# bode2spice
The <b>bode2spice</b> tool reads data from Comma Separated Values file, exported by the Omicron Labs Bode Analyzer Suite or similar source, containing complex impedance or transfer function, plots it for inspection and generates LTSpice compatible component to ease embedding into simulation.

Warning:
It doesn't anything smart with the data. It reads vector of complex numbers which corresponds to the frequency response / impedance points from the measurement data and spits out vector of magnitude + phase pairs embedded in LTSpice format (voltage dependent voltage source for TF, voltage dependent current source for Z). It copies everything inside templates that can then be used in LTSpice schematic editor as components.

It's vulgarly overkill for such a task, considering the size of the installed files. There was a small Octave script that did the same thing before, but nobody wanted to use it (octave is hundreds of megabytes, so at least some savings for those who don't use it otherwise).

Installation:
For a Windows binary go to the releases to the right here on github. Everyone else must build it from sources.

It uses Qt (5 or 6) https://www.qt.io/ and QCustomplot https://www.qcustomplot.com/ libraries.


Qt must be installed on your system as a prerequisite, QCustomplot sources are included here.

To build it follow this steps:

    mkdir build_directory
    cd build_directory
    qmake ../bode2spice.pro
    make
    
Subdirectory data_example contains one input file example.csv, sample output files and a test schematic in LTSpice format which uses the components/models. You can see how the interaction works.

Enjoy your measurement and simulation.
