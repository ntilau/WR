#include "mdl_frm.h"


#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>

using namespace std;

mdl_frm::mdl_frm() {}

mdl_frm::~mdl_frm()
{
    vector<mdl_bc>().swap(bcs);
    vector<mdl_mtrl>().swap(mtrls);
}

void mdl_frm::clear()
{
    bcs.clear();
    mtrls.clear();
}

void mdl_frm::update_msh_info(mdl_msh &msh)
{
    if (strcmp(type.data(), "TETRA") == 0)
    {
        for (size_t i = 0; i < bcs.size(); i++)
        {
            bcs[i].faces.clear();
            size_t cLab = bcs[i].label;
            for (size_t fid = 0; fid < msh.n_faces; fid++)
            {
                if (cLab == msh.fac_lab[fid])
                {
                    bcs[i].faces.push_back(fid);
                }
            }
        }
        vector<size_t> mtrl_tets(mtrls.size(), 0), LabMap(mtrls.size(), 0);
        for (size_t tid = 0; tid < msh.n_tetras; tid++)
        {
            mtrl_tets[msh.tet_lab[tid]]++;
        }
        for (size_t i = 0; i < mtrls.size(); i++)
        {
            mtrls[i].tetras.resize(mtrl_tets[i]);
            mtrl_tets[i] = 0;
        }
        for (size_t tid = 0; tid < msh.n_tetras; tid++)
        {
            size_t cLab = msh.tet_lab[tid];
            mtrls[cLab].tetras[mtrl_tets[cLab]++] = tid;
        }
    }
}

void mdl_frm::write_prj_file(string &name)
{
    ofstream prj_out_file(string(name + ".core").c_str(),
                               ios::out | ios::ate);
    prj_out_file << "#Formulation " << type << "\n";
    prj_out_file << "#Materials " << mtrls.size() << "\n";
    for (size_t i = 0; i < mtrls.size(); i++)
    {
        prj_out_file << mtrls[i].label << " " << mtrls[i].type << " "
                     << mtrls[i].epsr << " " << mtrls[i].mur << " "
                     << mtrls[i].sigma << " " << mtrls[i].tand << " "
                     << mtrls[i].name << "\n";
    }
    prj_out_file << "#Boundaries " << bcs.size() << "\n";
    for (size_t i = 0; i < bcs.size(); i++)
    {
        prj_out_file << bcs[i].label << " " << bcs[i].name << " " << bcs[i].type;
        if (strcmp(bcs[i].type.data(), "WavePort") == 0)
        {
            prj_out_file << " " << bcs[i].num_modes;
        }
        else if (strcmp(bcs[i].type.data(), "Impedance") == 0)
        {
            prj_out_file << " " << bcs[i].surf_impedance.real() << " "
                         << bcs[i].surf_impedance.imag();
        }
        else if (strcmp(bcs[i].type.data(), "LumpedPort") == 0)
        {
            prj_out_file << " " << bcs[i].lumped_impedance.real() << " "
                         << bcs[i].lumped_impedance.imag();
        }
        else if (strcmp(bcs[i].type.data(), "LumpedRLC") == 0)
        {
            prj_out_file << " " << bcs[i].R << " " << bcs[i].L << " " << bcs[i].C;
        }
        else if (strcmp(bcs[i].type.data(), "Voltage") == 0)
        {
            prj_out_file << " " << bcs[i].voltage;
        }
        else if (strcmp(bcs[i].type.data(), "Current") == 0)
        {
            prj_out_file << " " << bcs[i].current;
        }
        prj_out_file << "\n";
    }
    prj_out_file.close();
}

void mdl_frm::read_prj_file(string &name)
{
    clear();
    ifstream frm_in_file(string(name + ".core").c_str(), ios::in);
    string line;
    istringstream iss;
    unsigned int tmp_uint;
    double tmp_dbl;
    string tmp_str;
    if (frm_in_file.is_open())
    {
        while (getline(frm_in_file, line))
        {
            iss.clear();
            iss.str(line);
            iss >> tmp_str;
            if (strcmp(tmp_str.data(), "#Formulation") == 0)
            {
                iss >> type;
            }
            if (strcmp(tmp_str.data(), "#Materials") == 0)
            {
                iss >> tmp_uint;
                mtrls.resize(tmp_uint);
                for (size_t i = 0; i < mtrls.size(); i++)
                {
                    getline(frm_in_file, line);
                    iss.clear();
                    iss.str(line);
                    iss >> mtrls[i].label;
                    iss >> mtrls[i].type;
                    iss >> mtrls[i].epsr;
                    iss >> mtrls[i].mur;
                    iss >> mtrls[i].sigma;
                    iss >> mtrls[i].tand;
                    iss >> mtrls[i].name;
                }
            }
            if (strcmp(tmp_str.data(), "#Boundaries") == 0)
            {
                iss >> tmp_uint;
                bcs.resize(tmp_uint);
                for (size_t i = 0; i < bcs.size(); i++)
                {
                    getline(frm_in_file, line);
                    iss.clear();
                    iss.str(line);
                    iss >> bcs[i].label;
                    iss >> bcs[i].name;
                    iss >> bcs[i].type;
                    if (strcmp(bcs[i].type.data(), "WavePort") == 0)
                    {
                        iss >> bcs[i].num_modes;
                    }
                    else if (strcmp(bcs[i].type.data(), "Impedance") == 0)
                    {
                        double real;
                        double imag;
                        iss >> real;
                        iss >> imag;
                        bcs[i].surf_impedance = complex<double>(real, imag);
                    }
                    else if (strcmp(bcs[i].type.data(), "LumpedPort") == 0)
                    {
                        double real;
                        double imag;
                        iss >> real;
                        iss >> imag;
                        bcs[i].lumped_impedance = complex<double>(real, imag);
                    }
                    else if (strcmp(bcs[i].type.data(), "LumpedRLC") == 0)
                    {
                        iss >> bcs[i].R;
                        iss >> bcs[i].L;
                        iss >> bcs[i].C;
                    }
                    else if (strcmp(bcs[i].type.data(), "Voltage") == 0)
                    {
                        iss >> bcs[i].voltage;
                    }
                    else if (strcmp(bcs[i].type.data(), "Current") == 0)
                    {
                        iss >> bcs[i].current;
                    }
                }
            }
        }
    }
    frm_in_file.close();
}