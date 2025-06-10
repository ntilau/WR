#ifndef MDL_MTRL_H
#define MDL_MTRL_H

#include <string>
#include <vector>

using namespace std;

class mdl_mtrl
{
public:
    mdl_mtrl();
    ~mdl_mtrl();
    void upd_mtrl(double &freq);
    double calc_epsr2(double &freq);
    void upd_mtrl();
    double epsr = 1.0;
    double epsr2 = 0.0;
    double mur = 1.0;
    double kr = 0.0;
    double sigma = 0.0;
    double etaSigma = 0.0;
    double tand = 0.0;
    double kerr = 0.0;
    string name = "None";
    string type = "Vacuum";
    int label = 0;
    vector<size_t> tetras;
    vector<size_t> faces;
};

#endif // MDL_MTRL_H