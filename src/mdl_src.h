#ifndef MDL_SRC_H
#define MDL_SRC_H

#include <vector>
#include <complex>
#include <string>

using namespace std;

class mdl_src
{
public:
    mdl_src();
    mdl_src(mdl_src *); // copy constructor
    ~mdl_src();
    struct dipole
    {
        vector<complex<double>> amplitude;
        vector<double> position;
        vector<double> direction;
        double length;
    };
    vector<dipole> currents;
    string name = "None";
    string type = "None";
};


#endif // MDL_SRC_H
