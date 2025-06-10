#ifndef MDL_FRM_H
#define MDL_FRM_H

#include <cstdint>
#include <vector>
#include <complex>
#include <string>
#include <list>

#include "mdl_bc.h"
#include "mdl_msh.h"
#include "mdl_mtrl.h"
#include "mdl_src.h"

using namespace std;

class mdl_frm
{
public:
    vector<list<string> > bc_type = {
        {"None", "PerfectE", "PerfectH", "Radiation", "WavePort", "Impedance", "LumpedPort", "LumpedRLC"},
        {"None", "PerfectE", "PerfectH", "Radiation"},
        {"None", "Voltage"},
        {"None", "Current", "Voltage", "Insulation", "Skin"}
    };
    vector<list<string> > frm_type = {
        {"EM_E_FD"}, {"EM_E_EIG"}, {"E_V_STAT"}, {"H_A_STAT"}, {"EM_PO"}
    };
    string type = "EM_E_FD";
    mdl_frm();
    ~mdl_frm();
    void clear();
    void update_msh_info(mdl_msh &msh);
    vector<mdl_bc> bcs;
    vector<mdl_mtrl> mtrls;
    vector<mdl_src> srcs;
    void write_prj_file(string &name);
    void read_prj_file(string &name);
    struct freq_type
    {
        double nominal = 1e10;
        vector<double> range = {1e9, 1e10};
        unsigned int nbr = 1;
    } freq;
    unsigned int niter = 100; // max number of iterations of iterative solver
    double toll = 1e-6;       // tollerance for iterative solver
    double relax = 1.0;
    unsigned int h = 0; // homogeneous refinement
    unsigned int p = 1; // polynomial order
    /// post processing
    vector<vector<double>> sol_real;
    vector<vector<complex<double>>> sol_cmplx;
};

#endif // MDL_FRM_H