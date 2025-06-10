#include "mdl_bc.h"

mdl_bc::mdl_bc()
{
    label = 0;
}

mdl_bc::~mdl_bc()
{
    std::vector<std::complex<double>>().swap(mode_beta);
    std::vector<std::vector<std::complex<double>>>().swap(mode_eig_vec);
    std::vector<std::vector<std::complex<double>>>().swap(mode_eig_vec_f);
    std::vector<std::vector<size_t>>().swap(mode_dof_map);
}