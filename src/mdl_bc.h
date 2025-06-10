#ifndef MDL_BC_H
#define MDL_BC_H

#include <string>
#include <vector>
#include <complex>

using namespace std;

class mdl_bc
{
public:
    mdl_bc();
    ~mdl_bc();
    int label;
    string name = "None";
    string type = "None";
    double power = 1.0;
    unsigned int num_modes = 0;
    bool tfe = true;
    vector<vector<size_t>> mode_dof_map = vector<vector<size_t>>(3); // HGRAD, HCURL, HDIV
    vector<complex<double>> mode_beta;
    vector<vector<complex<double>>> mode_eig_vec;
    vector<vector<complex<double>>> mode_eig_vec_f;
    complex<double> lumped_impedance = complex<double>(50.0, 0.0);
    double R = 50.0, L = 0.0, C = 0.0;
    complex<double> surf_impedance = complex<double>(0.0, 0.0);
    bool inc = false;
    vector<double> inc_E = {1.0, 0.0, 0.0};
    vector<double> inc_k = {0.0, 0.0, 1.0};
    double n_theta = 101, n_phi = 201;
    double voltage = 1.0;
    double current = 1.0;
    vector<size_t> faces;
    vector<size_t> edges;
};

#endif // MDL_BC_H