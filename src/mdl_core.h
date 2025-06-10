#ifndef MDL_CORE_H
#define MDL_CORE_H

#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>


#include "mdl_frm.h"
#include "mdl_sld.h"
#include "mdl_msh.h"

using namespace std;

class mdl_core
{
public:
    struct HFSSPart
    {
        std::string name = "";
        std::string material = "";
        bool solveInside = false;
        int id = 0;
    };

    struct HFSSBnd
    {
        std::string name = "";
        std::string type = "";
        std::vector<size_t> faces;
        std::vector<size_t> solids;
        std::vector<size_t> faceIds;
        int numModes = 1;
    };

    struct HFSSMtrl
    {
        double permittivity = 1;
        double permeability = 1;
        double conductivity = 0;
        double dielectric_loss_tangent = 0;
        std::string name = "";
    };
    mdl_frm frm;
    mdl_sld sld;
    mdl_msh msh;
    void create_tri_mesh();
    void import(string path, string name, string ext);
    void import_hfss(string path, string name_ext);
    void read_prj_file(string path_name);
    void write_prj_file(string path_name);
    void clear()
    {
        sld.clear();
        frm.clear();
        msh.clear();
    }

protected:
    std::map<std::string, std::string> extractContent(const std::string &fileName, const std::string &dataType);
    void removeCharsFromString(string &str, const char *charsToRemove)
    {
        for (unsigned int i = 0; i < strlen(charsToRemove); ++i)
        {
            str.erase(remove(str.begin(), str.end(), charsToRemove[i]), str.end());
        }
    }
    char find_SI_factor(string &str)
    {
        for (unsigned int i = 0; i < strlen(SI_chars); ++i)
        {
            size_t found = str.find(SI_chars[i]);
            if (found <= str.size())
                return str[found];
        }
        return 0;
    }
    double set_factor(char fact)
    {
        if (fact == 'm')
            return 1e-3;
        else if (fact == 'u')
            return 1e-6;
        else if (fact == 'n')
            return 1e-9;
        else if (fact == 'p')
            return 1e-12;
        else if (fact == 'f')
            return 1e-15;
        else if (fact == 'a')
            return 1e-18;
        else
            return 1;
    }
    const char *SI_chars = "munpfa";
private:
    std::map<string, HFSSMtrl > mtrls;
    std::vector<HFSSBnd > bnds;
    std::vector<HFSSPart > parts;
    std::vector<size_t> mtrlTag;
    std::vector<size_t> hfssid;
    std::vector<bool> tetFlag;
    std::vector<bool> facFlag;
    std::vector<bool> nodFlag;
    std::vector< std::vector<size_t> > facHFSStag;
    std::map<size_t, std::vector<size_t> > bndMap;
    std::vector<std::vector<size_t> > adjTetra;
};

#endif // MDL_CORE_H
