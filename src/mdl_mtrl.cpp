#include "mdl_mtrl.h"
#include "phys_const.h"


mdl_mtrl::mdl_mtrl() {}

mdl_mtrl::~mdl_mtrl() {}

void mdl_mtrl::upd_mtrl()
{
    epsr2 = -tand * epsr;
}

void mdl_mtrl::upd_mtrl(double &freq)
{
    epsr2 = -sigma / (2.0 * phys_const::pi * freq * phys_const::eps0) - tand * epsr;
}

double mdl_mtrl::calc_epsr2(double &freq)
{
    return (-sigma / (2.0 * phys_const::pi * freq * phys_const::eps0) - tand * epsr);
}