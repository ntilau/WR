#include "mdl_core.h"
#include <climits>
#include <algorithm>

void mdl_core::create_tri_mesh()
{
    msh.clear();
    msh.type = "TRIA";
    msh.n_nodes = sld.nodes.size();
    msh.nod_pos = sld.nodes;
    msh.n_faces = sld.faces.size();
    msh.fac_nodes.resize(msh.n_faces);
    msh.fac_edges.resize(msh.n_faces);
    msh.fac_lab.assign(msh.n_faces, 1);
    map<pair<size_t, size_t>, size_t> edg_map;
    size_t edg_cnt = 0;
    for (size_t i = 0; i < msh.n_faces; i++)
    {
        msh.fac_nodes[i] = sld.faces[i].polygons[0];
        sort(msh.fac_nodes[i].begin(), msh.fac_nodes[i].end());
        //		msh.fac_lab[i] = sld.faces_marker[i][0];
        if (edg_map.find(make_pair(msh.fac_nodes[i][0],
                                   msh.fac_nodes[i][1])) == edg_map.end())
            edg_map[make_pair(msh.fac_nodes[i][0], msh.fac_nodes[i][1])] =
                edg_cnt++;
        if (edg_map.find(make_pair(msh.fac_nodes[i][0],
                                   msh.fac_nodes[i][1])) == edg_map.end())
            edg_map[make_pair(msh.fac_nodes[i][0], msh.fac_nodes[i][2])] =
                edg_cnt++;
        if (edg_map.find(make_pair(msh.fac_nodes[i][0],
                                   msh.fac_nodes[i][1])) == edg_map.end())
            edg_map[make_pair(msh.fac_nodes[i][1], msh.fac_nodes[i][2])] =
                edg_cnt++;
    }
    cout << edg_cnt << "\n";
    msh.n_edges = edg_cnt;
    msh.edg_nodes.resize(edg_cnt);
    for (map<pair<size_t, size_t>, size_t>::iterator it =
             edg_map.begin();
         it != edg_map.end(); it++)
    {
        vector<size_t> edge(2);
        edge[0] = get<0>(it->first);
        edge[1] = get<1>(it->first);
        msh.edg_nodes[it->second] = edge;
    }
    msh.edg_lab.assign(edg_cnt, 1);
    msh.max_edg_marker = 1;
    msh.get_mesh_statistics();
}

void mdl_core::import(string path, string name, string ext)
{
    int ret = 0;
    if (strcmp(ext.c_str(), ".hfss") == 0) // convert HFSS project to CORE
    {
        cout << "HFSS project files:" << endl;
#ifdef __linux__
        ret = system(string("cp -f " + path + "/" + name + ".hfssresults/*.results/*.cmesh/current.* " + path).c_str());
#elif _WIN32
        system(string("cd " + path + "\\" + name + ".hfssresults\\*.results\\*.cmesh && copy /Y current.* ..\\..\\..\\*").c_str());
#endif
        import_hfss(path, string(path + "/" + name + ext));
#ifdef __linux__
        ret = system(string("rm -f " + path + "/current.*").c_str());
#elif _WIN32
        system(string("del /F /Q current.*").c_str());
#endif
        write_prj_file(string(path + "/" + name));
    }
    else if (strcmp(ext.c_str(), ".core") == 0) // import CORE project data
    {
        read_prj_file(string(path + "/" + name));
    }
    // PRE_PROCESSING
    msh.get_mesh_statistics();
    // POST_PROCESSING
    msh.save_vtk_mesh(string(path + "/" + name));
}

void mdl_core::import_hfss(string path, string full_path_name)
{
    bool debug = false;
    cout << full_path_name << endl;
    {
        int tmpInt;
        double tmpDbl;
        string tmpStr;
        string partName;
        bool solveInside;
        string materialName;
        string boundaryName;
        string line;

        ifstream fileName(full_path_name.c_str());
        if (fileName.is_open())
        {
            while (getline(fileName, line)) // fileName.good())
            {
                istringstream iss(line);
                iss >> tmpStr;
                if (tmpStr == "$begin")
                {
                    iss >> tmpStr;
                    if (tmpStr == "\'Materials\'")
                    {
                        // cout << line << endl;
                        while (getline(fileName, line))
                        {
                            // cout << line << endl;
                            istringstream iss(line);
                            iss >> tmpStr;
                            if (tmpStr == "$begin")
                            {
                                HFSSMtrl hfssMaterial;
                                string tmpMtrl;
                                while (iss.good())
                                {
                                    iss >> tmpStr;
                                    tmpMtrl.append(tmpStr);
                                }
                                materialName = tmpMtrl.substr(1, tmpMtrl.size() - 2);
                                while (getline(fileName, line))
                                {
                                    istringstream iss(line);
                                    iss >> tmpStr; // $begin
                                    if (tmpStr.substr(0, 12) == "permittivity")
                                    {
                                        tmpDbl = atof(string(tmpStr.substr(14, tmpStr.size() - 15)).data());
                                        hfssMaterial.permittivity = tmpDbl;
                                    }
                                    else if (tmpStr.substr(0, 12) == "permeability")
                                    {
                                        tmpDbl = atof(string(tmpStr.substr(14, tmpStr.size() - 15)).data());
                                        hfssMaterial.permeability = tmpDbl;
                                    }
                                    else if (tmpStr.substr(0, 12) == "conductivity")
                                    {
                                        tmpDbl = atof(string(tmpStr.substr(14, tmpStr.size() - 15)).data());
                                        hfssMaterial.conductivity = tmpDbl;
                                    }
                                    else if (tmpStr.substr(0, 23) == "dielectric_loss_tangent")
                                    {
                                        tmpDbl = atof(string(tmpStr.substr(25, tmpStr.size() - 26)).data());
                                        hfssMaterial.dielectric_loss_tangent = tmpDbl;
                                    }
                                    else if (tmpStr == "$end")
                                    {
                                        string tmpMtrl;
                                        while (iss.good())
                                        {
                                            iss >> tmpStr;
                                            tmpMtrl.append(tmpStr);
                                        }
                                        tmpMtrl = tmpMtrl.substr(1, tmpMtrl.size() - 2);
                                        if (tmpMtrl == materialName)
                                        {
                                            hfssMaterial.name = materialName;
                                            mtrls[materialName] = hfssMaterial;
                                            // mtrls.insert(std::pair(materialName, hfssMaterial));
                                            break;
                                        }
                                    }
                                }
                            }
                            else if (tmpStr == "$end")
                            {
                                iss >> tmpStr;
                                if (tmpStr == "\'Materials\'")
                                {
                                    break;
                                }
                            }
                        }
                    }
                    else if (tmpStr == "\'ToplevelParts\'")
                    {
                        while (getline(fileName, line))
                        {
                            // cout << line << endl;
                            istringstream iss(line);
                            iss >> tmpStr;
                            if (tmpStr == "$begin")
                            {
                                iss >> tmpStr;
                                if (tmpStr == "\'GeometryPart\'")
                                {
                                    HFSSPart hfssPart;
                                    while (getline(fileName, line))
                                    {
                                        istringstream iss(line);
                                        iss >> tmpStr;
                                        if (tmpStr == "$begin")
                                        {
                                            iss >> tmpStr;
                                            if (tmpStr == "\'Attributes\'")
                                            {
                                                while (getline(fileName, line))
                                                {
                                                    istringstream iss(line);
                                                    iss >> tmpStr;
                                                    if (tmpStr.substr(0, 4) == "Name")
                                                    {
                                                        string tmpString = tmpStr;
                                                        while (iss.good())
                                                        {
                                                            iss >> tmpStr;
                                                            tmpString.append(tmpStr);
                                                        }
                                                        hfssPart.name = string(tmpString.substr(6, tmpString.size() - 7));
                                                    }
                                                    else if (tmpStr.substr(0, 13) == "MaterialValue") // for hfss v13
                                                    {
                                                        string tmpString = tmpStr;
                                                        while (iss.good())
                                                        {
                                                            iss >> tmpStr;
                                                            tmpString.append(tmpStr);
                                                        }
                                                        hfssPart.material = string(tmpString.substr(16, tmpString.size() - 18).data());
                                                    }
                                                    else if (tmpStr.substr(0, 12) == "MaterialName") // for hfss v11
                                                    {
                                                        string tmpString = tmpStr;
                                                        while (iss.good())
                                                        {
                                                            iss >> tmpStr;
                                                            tmpString.append(tmpStr);
                                                        }
                                                        hfssPart.material = string(tmpString.substr(14, tmpString.size() - 15));
                                                    }
                                                    else if (tmpStr.substr(0, 11) == "SolveInside")
                                                    {
                                                        hfssPart.solveInside = string(tmpStr.substr(12, tmpStr.size() - 12)) == "true";
                                                    }
                                                    else if (tmpStr == "$end")
                                                    {
                                                        iss >> tmpStr;
                                                        if (tmpStr == "\'Attributes\'")
                                                        {
                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                            else if (tmpStr == "\'Operation\'")
                                            {
                                                while (getline(fileName, line))
                                                {
                                                    istringstream iss(line);
                                                    iss >> tmpStr;
                                                    if (tmpStr.substr(0, 12) == "ParentPartID")
                                                    {
                                                        hfssPart.id = atoi(string(tmpStr.substr(13, tmpStr.size() - 13)).data());
                                                    }
                                                    else if (tmpStr == "$end")
                                                    {
                                                        iss >> tmpStr;
                                                        if (tmpStr == "\'Operation\'")
                                                        {
                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                        else if (tmpStr == "$end")
                                        {
                                            iss >> tmpStr;
                                            if (tmpStr == "\'GeometryPart\'")
                                            {
                                                parts.push_back(hfssPart);
                                                // cout << hfssPart.material << endl;
                                                // cout << parts.size() << endl;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            else if (tmpStr == "$end")
                            {
                                iss >> tmpStr;
                                if (tmpStr == "\'ToplevelParts\'")
                                {
                                    break;
                                }
                            }
                        }
                    }
                    else if (tmpStr == "\'Boundaries\'")
                    {
                        while (getline(fileName, line))
                        {
                            // cout << line << endl;
                            istringstream iss(line);
                            iss >> tmpStr; // $begin
                            if (tmpStr == "$begin")
                            {
                                iss >> tmpStr;
                                HFSSBnd hfssBoundary;
                                hfssBoundary.name = tmpStr.substr(1, tmpStr.size() - 2);
                                while (getline(fileName, line))
                                {
                                    istringstream iss(line);
                                    iss >> tmpStr; // $begin
                                    if (tmpStr.substr(0, 9) == "BoundType")
                                    {
                                        hfssBoundary.type = string(tmpStr.substr(11, tmpStr.size() - 11));
                                        while (iss.good())
                                        {
                                            iss >> tmpStr;
                                            hfssBoundary.type += string(tmpStr.substr(0, tmpStr.size() - 1));
                                        }
                                        if (hfssBoundary.type.substr(0, 9) == "Radiation")
                                        {
                                            hfssBoundary.type = "Radiation";
                                        }
                                        if (hfssBoundary.type.substr(0, 11) == "Lumped Port")
                                        {
                                            hfssBoundary.type = "LumpedPort";
                                        }
                                    }
                                    else if (tmpStr.substr(0, 8) == "NumModes")
                                    {
                                        hfssBoundary.numModes = atoi(string(tmpStr.substr(9, tmpStr.size() - 1)).data());
                                    }
                                    else if (tmpStr.substr(0, 5) == "Faces")
                                    {
                                        hfssBoundary.faces.push_back(atoi(string(tmpStr.substr(6, tmpStr.size() - 7)).data()));
                                        while (iss.good())
                                        {
                                            iss >> tmpStr;
                                            hfssBoundary.faces.push_back(atoi(string(tmpStr.substr(0, tmpStr.size() - 1)).data()));
                                        }
                                    }
                                    else if (tmpStr.substr(0, 7) == "Objects")
                                    {
                                        // if(hfssBoundary.type != "LumpedPort") {
                                        hfssBoundary.solids.push_back(atoi(string(tmpStr.substr(8, tmpStr.size() - 9)).data()));
                                        while (iss.good())
                                        {
                                            iss >> tmpStr;
                                            hfssBoundary.solids.push_back(atoi(string(tmpStr.substr(0, tmpStr.size() - 1)).data()));
                                        }
                                        //}
                                        // else
                                        // hfssBoundary.faces.push_back(atoi(string(tmpStr.substr(8, tmpStr.size() - 9)).data()));
                                    }
                                    else if (tmpStr == "$end")
                                    {
                                        iss >> tmpStr;
                                        if (tmpStr == "\'" + hfssBoundary.name + "\'")
                                        {
                                            bnds.push_back(hfssBoundary);
                                            break;
                                        }
                                    }
                                }
                            }
                            else if (tmpStr == "$end")
                            {
                                iss >> tmpStr;
                                if (tmpStr == "\'Boundaries\'")
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            throw string("Cannot find " + full_path_name);
        }
        fileName.close();
    }

    cout << "current.pnt," << endl;
    {
        size_t numPnts;
        int tmpInt;
        double tmpDbl, x, y, z;
        std::string tmpStr;
        std::string line;
        std::ifstream fileName(std::string(path + "/current.pnt").c_str());
        if (fileName.is_open())
        {
            std::getline(fileName, line);
            std::istringstream iss(line);
            iss >> tmpStr;
            if (tmpStr == "points")
            {
                iss >> numPnts;
            }
            else
            {
                throw std::string("current.pnt is not of points type");
            }
            msh.n_nodes = numPnts;
            msh.nod_pos.resize(numPnts);
            for (size_t i = 0; i < numPnts; i++)
            {
                std::getline(fileName, line);
                std::istringstream iss(line);
                iss >> tmpStr;
                iss >> tmpInt;
                msh.nod_pos[i].resize(3);
                iss >> msh.nod_pos[i][0];
                iss >> msh.nod_pos[i][1];
                iss >> msh.nod_pos[i][2];
            }
            fileName.close();
        }
        else
        {
            throw std::string(path + " project \"current.pnt\" not available");
        }
    }

    cout << "current.fac, " << endl;
    {
        size_t numFaces;
        int tmpInt, n0, n1, n2, t0, t1;
        std::vector<size_t> bndTag;
        size_t bndTagNum;
        double tmpDbl;
        std::string tmpStr;
        std::string line;
        std::ifstream fileName(std::string(path + "/current.fac").c_str());
        if (fileName.is_open())
        {
            getline(fileName, line);
            std::istringstream iss(line);
            iss >> tmpStr;
            if (tmpStr == "faces_v2")
            {
                iss >> numFaces;
            }
            else
            {
                throw std::string("current.fac is not of faces_v2 type");
            }
            msh.n_faces = numFaces;
            msh.fac_nodes.resize(numFaces);
            facHFSStag.resize(numFaces);
            msh.fac_lab.resize(numFaces);
            msh.fac_adj_tet.resize(numFaces);
            for (size_t i = 0; i < numFaces; i++)
            {
                getline(fileName, line);
                // cout << line << endl;
                std::istringstream iss(line);
                iss >> tmpStr; // f
                iss >> tmpInt; // id
                msh.fac_nodes[i].resize(3);
                iss >> msh.fac_nodes[i][0];
                iss >> msh.fac_nodes[i][1];
                iss >> msh.fac_nodes[i][2];
                msh.fac_nodes[i][0] -= 1;
                msh.fac_nodes[i][1] -= 1;
                msh.fac_nodes[i][2] -= 1;
                iss >> tmpStr; // h
                iss >> t0;
                iss >> t1;
                iss >> bndTagNum;
                if (bndTagNum > 0)
                {
                    bndTag.resize(bndTagNum);
                    for (size_t ibnd = 0; ibnd < bndTagNum; ibnd++)
                    {
                        iss >> bndTag[ibnd];
                    }
                    facHFSStag[i] = bndTag;
                }
            }
            {
                getline(fileName, line);
                std::istringstream iss(line);
                iss >> tmpStr;
                if (tmpStr != "end_face")
                {
                    throw std::string("end_face not found");
                }
            }
            {
                getline(fileName, line);
                std::istringstream iss(line);
                iss >> tmpStr;
                if (tmpStr == "NumFaces")
                {
                    size_t tag, label;
                    iss >> bndTagNum;
                    for (size_t i = 0; i < bndTagNum; i++)
                    {
                        getline(fileName, line);
                        std::istringstream iss(line);
                        iss >> tag;
                        iss >> label;
                        bndMap[label].push_back(tag);
                    }
                }
            }
            fileName.close();
        }
        else
        {
            throw std::string(path + " project file \"current.fac\" not available");
        }
    }

    cout << "current.hyd, " << endl;
    {
        size_t numHydras;
        int tmpInt, n0, n1, n2, n3, f0, f1, f2, f3, b0, b1, b2, b3;
        size_t l0, l1, l2, l3, l4, l5, s0;
        double tmpDbl;
        std::string tmpStr;
        std::string line;
        std::ifstream fileName(std::string(path + "/current.hyd").c_str());
        if (fileName.is_open())
        {
            getline(fileName, line);
            std::istringstream iss(line);
            iss >> tmpStr;
            if (tmpStr == "hydras")
            {
                iss >> numHydras;
            }
            else
            {
                throw std::string("current.fac is not of faces_v2 type");
            }
            msh.n_tetras = numHydras;
            msh.tet_nodes.resize(numHydras);
            msh.tet_edges.resize(numHydras);
            msh.tet_faces.resize(numHydras);
            msh.tet_lab.resize(numHydras);
            hfssid.resize(numHydras);
            for (size_t i = 0; i < numHydras; i++)
            {
                getline(fileName, line);
                {
                    std::istringstream iss(line);
                    iss >> tmpStr; // h
                    iss >> tmpInt; // id
                    msh.tet_nodes[i].resize(4);
                    iss >> msh.tet_nodes[i][0];
                    iss >> msh.tet_nodes[i][1];
                    iss >> msh.tet_nodes[i][2];
                    iss >> msh.tet_nodes[i][3];
                    msh.tet_nodes[i][0] -= 1;
                    msh.tet_nodes[i][1] -= 1;
                    msh.tet_nodes[i][2] -= 1;
                    msh.tet_nodes[i][3] -= 1;
                }
                getline(fileName, line);
                {
                    std::istringstream iss(line);
                    iss >> tmpStr; // f
                    msh.tet_faces[i].resize(4);
                    iss >> msh.tet_faces[i][0];
                    iss >> msh.tet_faces[i][1];
                    iss >> msh.tet_faces[i][2];
                    iss >> msh.tet_faces[i][3];
                    msh.tet_faces[i][0] -= 1;
                    msh.tet_faces[i][1] -= 1;
                    msh.tet_faces[i][2] -= 1;
                    msh.tet_faces[i][3] -= 1;
                }
                getline(fileName, line);
                {
                    std::istringstream iss(line);
                    iss >> tmpStr; // b
                    iss >> tmpInt;
                    iss >> b0;
                    iss >> b1;
                    iss >> b2;
                    iss >> b3;
                }
                getline(fileName, line);
                {
                    std::istringstream iss(line);
                    iss >> tmpStr; // l
                    iss >> l0;
                    iss >> l1;
                    iss >> l2;
                    iss >> l3;
                    iss >> l4;
                    iss >> l5;
                }
                getline(fileName, line);
                {
                    std::istringstream iss(line);
                    iss >> tmpStr; // s
                    iss >> hfssid[i];
                }
            }
            {
                getline(fileName, line);
                std::istringstream iss(line);
                iss >> tmpStr;
                if (tmpStr != "end_hydra")
                {
                    throw std::string("end_hydra not found");
                }
            }
            fileName.close();
        }
        else
        {
            throw std::string(path + " project file \"current.hyd\" not available");
        }
    }
    // msh.get_mesh_statistics();
    cout << "Finalizing ..." << endl;
    {
        tetFlag = std::vector<bool>(msh.n_tetras, false);
        facFlag = std::vector<bool>(msh.n_faces, false);
        nodFlag = std::vector<bool>(msh.n_nodes, false);
        size_t idx = 0, nidx = 0, eidx = 0, fidx = 0, tidx = 0;
        std::map<std::string, std::vector<size_t>> gSolidTets;
        for (std::vector<HFSSPart>::iterator it = parts.begin(); it != parts.end(); it++)
        {
            if (it->solveInside)
            {
                mdl_mtrl mtr;
                mtr.name = it->material;
                mtr.epsr = mtrls[it->material].permittivity;
                mtr.mur = mtrls[it->material].permeability;
                mtr.sigma = mtrls[it->material].conductivity;
                mtr.tand = mtrls[it->material].dielectric_loss_tangent;
                for (size_t tid = 0; tid < msh.n_tetras; tid++)
                {
                    if (it->id == hfssid[tid])
                    {
                        tetFlag[tid] = true;
                        mtr.label = frm.mtrls.size();
                        gSolidTets[it->name].push_back(tid);
                    }
                }
                if (gSolidTets[it->name].size() == 0)
                {
                    gSolidTets.erase(it->name);
                }
                else
                {
                    // mtr.Tetras.resize(gSolidTets[it->name].size());
                    mtr.tetras.resize(gSolidTets[it->name].size());
                    size_t idx = 0;
                    for (std::vector<size_t>::iterator iter = gSolidTets[it->name].begin();
                         iter != gSolidTets[it->name].end(); iter++)
                    {
                        // mtr.Tetras(idx++) = *iter;
                        mtr.tetras[idx++] = *iter;
                        msh.tet_lab[*iter] = mtr.label;
                    }
                    std::cout << it->name << " " << mtr.name << "\n";
                    frm.mtrls.push_back(mtr);
                }
            }
        }

        for (std::vector<HFSSBnd>::iterator it = bnds.begin(); it != bnds.end(); it++)
        {
            mdl_bc bc;
            bc.type = it->type;
            bc.name = it->name;
            bc.label = frm.bcs.size();
            std::cout << bc.name << " " << bc.type;
            if (bc.type == "WavePort")
            {
                bc.num_modes = it->numModes;
                std::cout << " " << bc.num_modes;
            }
            std::cout << "\n";
            frm.bcs.push_back(bc);
        }
        if (debug)
        {
            std::cout << "TetMap\n";
        }
        std::vector<size_t> tetMap(msh.n_tetras, SIZE_MAX);
        tidx = 0;
        for (size_t tid = 0; tid < msh.n_tetras; tid++)
        {
            if (tetFlag[tid])
            {
                tetMap[tid] = tidx++;
            }
        }
        if (debug)
        {
            std::cout << "newTetNodes newTetFaces newTetLab " << tidx << "\n";
        }

        vector<vector<size_t>> newTetNodes(tidx);
        vector<vector<size_t>> newTetFaces(tidx);
        vector<int> newTetLab(tidx);
        for (size_t tid = 0; tid < msh.n_tetras; tid++)
        {
            if (tetFlag[tid])
            {
                newTetNodes[tetMap[tid]] = msh.tet_nodes[tid];
                newTetFaces[tetMap[tid]] = msh.tet_faces[tid];
                newTetLab[tetMap[tid]] = msh.tet_lab[tid];
            }
        }
        msh.n_tetras = tidx;
        msh.tet_nodes = newTetNodes;
        msh.tet_faces = newTetFaces;
        msh.tet_lab = newTetLab;
        if (debug)
        {
            std::cout << "tetMtrl newTetras\n";
        }

        for (size_t mtrid = 0; mtrid < mtrls.size(); mtrid++)
        {
            std::vector<size_t> newTetras;
            for (size_t tid = 0; tid < frm.mtrls[mtrid].tetras.size(); tid++)
            {
                if (tetMap[frm.mtrls[mtrid].tetras[tid]] < SIZE_MAX)
                {
                    newTetras.push_back(tetMap[frm.mtrls[mtrid].tetras[tid]]);
                }
            }
            frm.mtrls[mtrid].tetras = newTetras;
            // msh->tetMtrl[mtrid].Tetras.clear();
            // msh->tetMtrl[mtrid].Tetras.resize(newTetras.size());
            // for (size_t tid = 0; tid < newTetras.size(); tid++)
            // {
            //     msh->tetMtrl[mtrid].Tetras(tid) = newTetras[tid];
            // }
        }

        // reorder nodes and faces
        if (debug)
        {
            std::cout << "nodMap " << msh.n_nodes << " facMap " << msh.n_faces << "\n";
        }
        std::vector<size_t> nodMap(msh.n_nodes, -SIZE_MAX);
        std::vector<size_t> facMap(msh.n_faces, -SIZE_MAX);
        for (size_t tid = 0; tid < msh.n_tetras; tid++)
        {
            for (size_t i = 0; i < 4; i++)
            {
                if (nodFlag[msh.tet_nodes[tid][i]] == false)
                {
                    nodFlag[msh.tet_nodes[tid][i]] = true;
                    nodMap[msh.tet_nodes[tid][i]] = nidx++;
                }
                if (facFlag[msh.tet_faces[tid][i]] == false)
                {
                    facFlag[msh.tet_faces[tid][i]] = true;
                    facMap[msh.tet_faces[tid][i]] = fidx++;
                }
            }
        }
        // finishing with nodes
        if (debug)
        {
            std::cout << "tetNodes tetFaces\n";
        }
        for (size_t tid = 0; tid < msh.n_tetras; tid++)
        {
            for (size_t i = 0; i < 4; i++)
            {
                msh.tet_nodes[tid][i] = nodMap[msh.tet_nodes[tid][i]];
                msh.tet_faces[tid][i] = facMap[msh.tet_faces[tid][i]];
            }
        std:
            sort(msh.tet_nodes[tid].begin(), msh.tet_nodes[tid].end());
        }
        if (debug)
        {
            std::cout << "facNodes\n";
        }
        for (size_t fid = 0; fid < msh.n_faces; fid++)
        {
            for (size_t i = 0; i < 3; i++)
            {
                msh.fac_nodes[fid][i] = nodMap[msh.fac_nodes[fid][i]];
            }
            sort(msh.fac_nodes[fid].begin(), msh.fac_nodes[fid].end());
        }
        if (debug)
        {
            std::cout << "newNodPos " << nidx << "\n";
        }
        // arma::mat newNodPos(nidx, 3);
        vector<vector<double>> newNodPos(msh.n_nodes);
        for (size_t nid = 0; nid < msh.n_nodes; nid++)
        {
            if (nodFlag[nid])
            {
                newNodPos[nodMap[nid]] = msh.nod_pos[nid];
            }
        }
        if (debug)
        {
            std::cout << "newFacNodes " << fidx << "\n";
        }
        // arma::umat newFacNodes(fidx, 3);
        vector<vector<size_t>> newFacNodes(msh.n_faces);
        for (size_t fid = 0; fid < msh.n_faces; fid++)
        {
            if (facFlag[fid])
            {
                sort(msh.fac_nodes[fid].begin(), msh.fac_nodes[fid].end());
                newFacNodes[facMap[fid]] = msh.fac_nodes[fid];
            }
        }
        msh.nod_pos = newNodPos;
        msh.fac_nodes = newFacNodes;
        msh.n_nodes = nidx;
        msh.n_faces = fidx;
        // remain to assign face labels
        // reorder faces in tetrahedron
        // create edges
        if (debug)
        {
            std::cout << "facLab\n";
        }
        msh.fac_lab.assign(msh.n_faces, -1);
        // msh->facLab.fill(msh->maxLab); // non boundary flag
        for (size_t fid = 0; fid < facHFSStag.size(); fid++)
        {
            for (std::vector<HFSSBnd>::iterator it = bnds.begin(); it != bnds.end(); it++)
            {
                if (it->faces.size())
                {
                    for (std::vector<size_t>::iterator itids = it->faces.begin(); itids != it->faces.end(); itids++)
                    {
                        if (std::find(facHFSStag[fid].begin(), facHFSStag[fid].end(), *itids) != facHFSStag[fid].end())
                        {
                            // std::cout << *itids << " ";
                            for (std::vector<mdl_bc>::iterator bndit = frm.bcs.begin();
                                 bndit != frm.bcs.end(); bndit++)
                            {
                                if (bndit->name == it->name)
                                {
                                    if (debug)
                                    {
                                        std::cout << " " << it->name << " " << facMap[fid] << " ";
                                    }
                                    msh.fac_lab[facMap[fid]] = bndit->label;
                                    bndit->faces.push_back(facMap[fid]);
                                    // arma::uvec nface(1);
                                    // nface(0) = facMap[fid];
                                    // bndit->Faces = arma::join_cols(bndit->Faces, nface);
                                }
                            }
                            // std::cout << " \n";
                        }
                    }
                }
                // solid based boundaries
                if (it->solids.size())
                {
                    for (std::vector<size_t>::iterator itids = it->solids.begin(); itids != it->solids.end(); itids++)
                    {
                        std::vector<size_t> cid = bndMap[*itids];
                        for (size_t idd = 0; idd < cid.size(); idd++)
                        {
                            if (std::find(facHFSStag[fid].begin(), facHFSStag[fid].end(), cid[idd]) != facHFSStag[fid].end())
                            {
                                for (std::vector<mdl_bc>::iterator bndit = frm.bcs.begin();
                                     bndit != frm.bcs.end(); bndit++)
                                {
                                    if (bndit->name == it->name)
                                    {
                                        if (debug)
                                        {
                                            std::cout << " " << it->name << " " << facMap[fid] << " ";
                                        }
                                        msh.fac_lab[facMap[fid]] = bndit->label;
                                        bndit->faces.push_back(facMap[fid]);
                                        // arma::uvec nface(1);
                                        // nface(0) = facMap[fid];
                                        // bndit->Faces = arma::join_cols(bndit->Faces, nface);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //msh.fac_adj_tet.clear();
    //msh.fac_nodes.clear();
    //msh.regularize_mesh();
    //frm.update_msh_info(msh);
    //msh.complete_mesh();
}

void mdl_core::read_prj_file(string name)
{
    frm.read_prj_file(name);
    sld.read_prj_file(name);
    msh.read_prj_file(name);
}

void mdl_core::write_prj_file(string name)
{
    frm.write_prj_file(name);
    sld.write_prj_file(name);
    msh.write_prj_file(name);
}
