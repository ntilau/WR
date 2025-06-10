#include "mdl_msh.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <climits>
#include <cfloat>
#include <map>
#include <algorithm>
#include <math.h>
#include <sstream>

mdl_msh::mdl_msh()
{
    n_domains = 0;
    n_edges = 0;
    n_faces = 0;
    n_nodes = 0;
    n_tetras = 0;
}

void mdl_msh::write_prj_file(string &name)
{
    ofstream msh_out_file(string(name + ".core").c_str(),
                          ios::out | ios::ate | ios::app);
    msh_out_file << "#Mesh " << type << "\n";
    msh_out_file << "#Nodes " << n_nodes << "\n";
    for (size_t i = 0; i < n_nodes; i++)
    {
        msh_out_file << scientific << setprecision(15) << nod_pos[i][0]
                     << " " << setprecision(15) << nod_pos[i][1] << " "
                     << setprecision(15) << nod_pos[i][2] << "\n";
    }
    msh_out_file << "#Edges " << n_edges << "\n";
    for (size_t i = 0; i < n_edges; i++)
    {
        msh_out_file << edg_nodes[i][0] << " " << edg_nodes[i][1] << " "
                     << edg_lab[i] << "\n";
    }
    msh_out_file << "#Faces " << n_faces << "\n";
    for (size_t i = 0; i < n_faces; i++)
    {
        msh_out_file << fac_nodes[i][0] << " " << fac_nodes[i][1] << " "
                     << fac_nodes[i][2] << " " << fac_lab[i] << "\n";
    }
    msh_out_file << "#Tetras " << n_tetras << "\n";
    for (size_t i = 0; i < n_tetras; i++)
    {
        msh_out_file << tet_nodes[i][0] << " " << tet_nodes[i][1] << " "
                     << tet_nodes[i][2] << " " << tet_nodes[i][3] << " "
                     << tet_lab[i] << "\n";
    }
    msh_out_file.close();
}

void mdl_msh::read_prj_file(string &name)
{
    clear();
    ifstream msh_in_file(string(name + ".core").c_str(), ios::in);
    string line;
    istringstream iss;
    unsigned int tmp_uint;
    double tmp_dbl;
    string tmp_str;
    if (msh_in_file.is_open())
    {
        while (getline(msh_in_file, line))
        {
            iss.clear();
            iss.str(line);
            iss >> tmp_str;
            if (strcmp(tmp_str.data(), "#Mesh") == 0)
            {
                iss >> type;
            }
            if (strcmp(tmp_str.data(), "#Nodes") == 0)
            {
                iss >> n_nodes;
                nod_pos.resize(n_nodes);
                for (size_t i = 0; i < n_nodes; i++)
                {
                    getline(msh_in_file, line);
                    iss.clear();
                    iss.str(line);
                    nod_pos[i].resize(3);
                    iss >> nod_pos[i][0];
                    iss >> nod_pos[i][1];
                    iss >> nod_pos[i][2];
                }
            }
            if (strcmp(tmp_str.data(), "#Edges") == 0)
            {
                iss >> n_edges;
                edg_nodes.resize(n_edges);
                edg_lab.resize(n_edges);
                for (size_t i = 0; i < n_edges; i++)
                {
                    getline(msh_in_file, line);
                    iss.clear();
                    iss.str(line);
                    edg_nodes[i].resize(2);
                    iss >> edg_nodes[i][0];
                    iss >> edg_nodes[i][1];
                    iss >> edg_lab[i];
                }
            }
            if (strcmp(tmp_str.data(), "#Faces") == 0)
            {
                iss >> n_faces;
                fac_nodes.resize(n_faces);
                fac_lab.resize(n_faces);
                for (size_t i = 0; i < n_faces; i++)
                {
                    getline(msh_in_file, line);
                    iss.clear();
                    iss.str(line);
                    fac_nodes[i].resize(3);
                    iss >> fac_nodes[i][0];
                    iss >> fac_nodes[i][1];
                    iss >> fac_nodes[i][2];
                    iss >> fac_lab[i];
                }
            }
            if (strcmp(tmp_str.data(), "#Tetras") == 0)
            {
                iss >> n_tetras;
                tet_nodes.resize(n_tetras);
                tet_lab.resize(n_tetras);
                for (size_t i = 0; i < n_tetras; i++)
                {
                    getline(msh_in_file, line);
                    iss.clear();
                    iss.str(line);
                    tet_nodes[i].resize(4);
                    iss >> tet_nodes[i][0];
                    iss >> tet_nodes[i][1];
                    iss >> tet_nodes[i][2];
                    iss >> tet_nodes[i][3];
                    iss >> tet_lab[i];
                }
            }
        }
    }
    msh_in_file.close();
    // regularize_mesh(); // ensures ordering is OK
    complete_mesh();
}

void mdl_msh::read_tetgen_files(string &name)
{
    clear();
    type = "TETRA";
    unsigned int lvl = 1;
    ostringstream str_lvl;
    str_lvl << lvl;
    string line;
    istringstream iss;
    double tmp_dbl;
    int tmp_int;
    size_t tmp_uint;
    string tmp_str;
    ifstream tet_node_file(
        string(name + "." + str_lvl.str() + ".node").c_str());
    if (tet_node_file.is_open())
    {
        cout << "Loading " << string(name + "." + str_lvl.str() + ".node") << "\n";
        iss.clear();
        do
        {
            getline(tet_node_file, line);
        } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
        iss.str(line);
        iss >> n_nodes;
        iss >> tmp_int;
        nod_pos.resize(n_nodes);
        for (size_t i = 0; i < n_nodes; i++)
        {
            iss.clear();
            do
            {
                getline(tet_node_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            iss.str(line);
            iss >> tmp_int;
            for (unsigned int j = 0; j < 3; j++)
            {
                iss >> tmp_dbl;
                nod_pos[i].push_back(tmp_dbl);
            }
        }
    }
    tet_node_file.close();
    ifstream tet_edge_file(
        string(name + "." + str_lvl.str() + ".edge").c_str());
    if (tet_edge_file.is_open())
    {
        cout << "Loading " << string(name + "." + str_lvl.str() + ".edge")
             << "\n";
        unsigned int dim;
        iss.clear();
        do
        {
            getline(tet_edge_file, line);
        } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
        iss.str(line);
        iss >> n_edges;
        iss >> tmp_int;
        edg_nodes.resize(n_edges);
        edg_lab.assign(n_edges, 0);
        for (size_t i = 0; i < n_edges; i++)
        {
            iss.clear();
            do
            {
                getline(tet_edge_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            iss.str(line);
            iss >> tmp_int;
            for (unsigned int j = 0; j < 2; j++)
            {
                iss >> tmp_uint;
                edg_nodes[i].push_back(tmp_uint - 1);
            }
            iss >> tmp_int;
            edg_lab[i] = tmp_int;
        }
    }
    tet_edge_file.close();
    ifstream tet_face_file(
        string(name + "." + str_lvl.str() + ".face").c_str());
    if (tet_face_file.is_open())
    {
        cout << "Loading " << string(name + "." + str_lvl.str() + ".face")
             << "\n";
        unsigned int dim;
        iss.clear();
        do
        {
            getline(tet_face_file, line);
        } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
        iss.str(line);
        iss >> n_faces;
        iss >> tmp_int;
        fac_nodes.resize(n_faces);
        fac_lab.assign(n_faces, 0);
        for (size_t i = 0; i < n_faces; i++)
        {
            iss.clear();
            do
            {
                getline(tet_face_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            iss.str(line);
            iss >> tmp_int;
            for (unsigned int j = 0; j < 3; j++)
            {
                iss >> tmp_uint;
                fac_nodes[i].push_back(tmp_uint - 1);
            }
            iss >> fac_lab[i];
        }
    }
    tet_face_file.close();
    ifstream tet_ele_file(
        string(name + "." + str_lvl.str() + ".ele").c_str());
    if (tet_ele_file.is_open())
    {
        cout << "Loading " << string(name + "." + str_lvl.str() + ".ele")
             << "\n";
        unsigned int dim;
        iss.clear();
        do
        {
            getline(tet_ele_file, line);
        } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
        iss.str(line);
        iss >> n_tetras;
        iss >> tmp_int;
        tet_nodes.resize(n_tetras);
        tet_lab.assign(n_tetras, 0);
        for (size_t i = 0; i < n_tetras; i++)
        {
            iss.clear();
            do
            {
                getline(tet_ele_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            iss.str(line);
            iss >> tmp_int;
            for (unsigned int j = 0; j < 4; j++)
            {
                iss >> tmp_uint;
                tet_nodes[i].push_back(tmp_uint - 1);
            }
            iss >> tmp_int;
            tet_lab[i] = tmp_int;
        }
    }
    tet_ele_file.close();
    regularize_mesh();
}

void mdl_msh::read_triangle_files(string &name)
{
    clear();
    type = "TRIA";
    unsigned int lvl = 1;
    ostringstream str_lvl;
    str_lvl << lvl;
    string line;
    istringstream iss;
    double tmp_dbl;
    int tmp_int;
    size_t tmp_uint;
    string tmp_str;
    ifstream tria_node_file(
        string(name + "." + str_lvl.str() + ".node").c_str());
    if (tria_node_file.is_open())
    {
        cout << "Loading " << string(name + "." + str_lvl.str() + ".node")
             << "\n";
        iss.clear();
        do
        {
            getline(tria_node_file, line);
        } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
        iss.str(line);
        iss >> n_nodes;
        iss >> tmp_int;
        nod_pos.resize(n_nodes);
        for (size_t i = 0; i < n_nodes; i++)
        {
            iss.clear();
            do
            {
                getline(tria_node_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            iss.str(line);
            iss >> tmp_int;
            for (unsigned int j = 0; j < 3; j++)
            {
                iss >> tmp_dbl;
                nod_pos[i].push_back(tmp_dbl);
            }
        }
    }
    tria_node_file.close();
    ifstream tria_edge_file(
        string(name + "." + str_lvl.str() + ".edge").c_str());
    if (tria_edge_file.is_open())
    {
        cout << "Loading " << string(name + "." + str_lvl.str() + ".edge")
             << "\n";
        unsigned int dim;
        iss.clear();
        do
        {
            getline(tria_edge_file, line);
        } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
        iss.str(line);
        iss >> n_edges;
        iss >> tmp_int;
        edg_nodes.resize(n_edges);
        edg_lab.assign(n_edges, 0);
        for (size_t i = 0; i < n_edges; i++)
        {
            iss.clear();
            do
            {
                getline(tria_edge_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            iss.str(line);
            iss >> tmp_int;
            for (unsigned int j = 0; j < 2; j++)
            {
                iss >> tmp_uint;
                edg_nodes[i].push_back(tmp_uint - 1);
            }
            iss >> tmp_int;
            edg_lab[i] = tmp_int;
        }
    }
    tria_edge_file.close();
    ifstream tria_ele_file(
        string(name + "." + str_lvl.str() + ".ele").c_str());
    if (tria_ele_file.is_open())
    {
        cout << "Loading " << string(name + "." + str_lvl.str() + ".ele")
             << "\n";
        unsigned int dim;
        iss.clear();
        do
        {
            getline(tria_ele_file, line);
        } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
        iss.str(line);
        iss >> n_faces;
        iss >> tmp_int;
        fac_nodes.resize(n_faces);
        fac_lab.assign(n_faces, 0);
        for (size_t i = 0; i < n_faces; i++)
        {
            iss.clear();
            do
            {
                getline(tria_ele_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            iss.str(line);
            iss >> tmp_int;
            for (unsigned int j = 0; j < 3; j++)
            {
                iss >> tmp_uint;
                fac_nodes[i].push_back(tmp_uint - 1);
            }
            iss >> tmp_int;
            fac_lab[i] = tmp_int;
        }
    }
    tria_ele_file.close();
    regularize_mesh();
}

/// Refinement mapping
static unsigned int tet_tet_nodes_loc_map_id[] = {
    0, 4, 5, 6, 1, 4, 7, 8, 2, 5, 7, 9, 3, 6, 8, 9,
    4, 5, 6, 7, 4, 6, 7, 8, 5, 6, 7, 9, 6, 7, 8, 9};

static unsigned int tet_fac_nodes_loc_map_id[] = {
    1, 7, 8, 2, 7, 9, 3, 8, 9, 7, 8, 9, 0, 5, 6, 2, 5, 9, 3, 6, 9, 5, 6, 9,
    0, 4, 6, 1, 4, 8, 3, 6, 8, 4, 6, 8, 0, 4, 5, 1, 4, 7, 2, 5, 7, 4, 5, 7,
    4, 5, 6, 4, 6, 7, 4, 7, 8, 5, 6, 7, 5, 7, 9, 6, 7, 8, 6, 7, 9, 6, 8, 9};

static unsigned int fac_fac_nodes_loc_map_id[] = {0, 4, 5, 1, 3, 5,
                                                  2, 3, 4, 3, 4, 5};

static unsigned int tet_edg_nodes_loc_map_id[] = {
    0, 4, 0, 5, 0, 6, 1, 4, 1, 7, 1, 8, 2, 5, 2, 7, 2, 9, 3, 6, 3, 8, 3, 9, 4,
    5, 4, 6, 4, 7, 4, 8, 5, 6, 5, 7, 5, 9, 6, 7, 6, 8, 6, 9, 7, 8, 7, 9, 8, 9};

static unsigned int fac_edg_nodes_loc_map_id[] = {1, 3, 2, 3, 0, 4, 2, 4, 0,
                                                  5, 1, 5, 3, 4, 3, 5, 4, 5};

static int tet_fac_lab_loc_map_id[] = {0, 0, 0, 0, 1, 1, 1, 1,
                                       2, 2, 2, 2, 3, 3, 3, 3,
                                       -1, -1, -1, -1, -1, -1, -1, -1};

static int fac_edg_lab_loc_map_id[] = {0, 0, 1, 1, 2, 2, -1, -1, -1};

mdl_msh::mdl_msh(mdl_msh *msh)
{
    msh->tet_nodes = tet_nodes;
    msh->tet_edges = tet_edges;
    msh->tet_faces = tet_faces;
    msh->fac_nodes = fac_nodes;
    msh->fac_edges = fac_edges;
    msh->edg_nodes = edg_nodes;
    msh->nod_pos = nod_pos;
    msh->edg_lab = edg_lab;
    msh->fac_lab = fac_lab;
    msh->tet_lab = tet_lab;
    msh->fac_adj_tet = fac_adj_tet;
    msh->edg_adj_fac = edg_adj_fac;
    msh->dom_tetras = dom_tetras;
    msh->dom_faces = dom_faces;
    msh->n_nodes = n_nodes;
    msh->n_edges = n_edges;
    msh->n_faces = n_faces;
    msh->n_tetras = n_tetras;
    msh->n_domains = n_domains;
}

mdl_msh::~mdl_msh()
{
    clear();
}

void mdl_msh::get_mesh_statistics()
{
    cout << "type = " << type << "\n";
    cout << "tet_nodes = " << tet_nodes.size() << "\n";
    cout << "tet_edges = " << tet_edges.size() << "\n";
    cout << "tet_faces = " << tet_faces.size() << "\n";
    cout << "fac_nodes = " << fac_nodes.size() << "\n";
    cout << "fac_edges = " << fac_edges.size() << "\n";
    cout << "edg_nodes = " << edg_nodes.size() << "\n";
    cout << "nod_pos = " << nod_pos.size() << "\n";
    cout << "fac_lab = " << fac_lab.size() << "\n";
    cout << "tet_lab = " << tet_lab.size() << "\n";
    cout << "fac_adj_tet = " << fac_adj_tet.size() << "\n";
    cout << "edg_adj_fac = " << edg_adj_fac.size() << "\n";
    cout << "dom_tetras = " << dom_tetras.size() << "\n";
    cout << "dom_faces = " << dom_faces.size() << "\n";
    cout << "n_nodes = " << n_nodes << "\n";
    cout << "n_edges = " << n_edges << "\n";
    cout << "n_faces = " << n_faces << "\n";
    cout << "n_tetras = " << n_tetras << "\n";
    cout << "n_domains = " << n_domains << "\n";
}

void mdl_msh::clear()
{
    tet_nodes.clear();
    tet_edges.clear();
    tet_faces.clear();
    fac_nodes.clear();
    fac_edges.clear();
    edg_nodes.clear();
    nod_pos.clear();
    fac_lab.clear();
    tet_lab.clear();
    fac_adj_tet.clear();
    edg_adj_fac.clear();
    dom_tetras.clear();
    dom_faces.clear();
    n_nodes = 0;
    n_edges = 0;
    n_faces = 0;
    n_tetras = 0;
    n_domains = 0;
    max_edg_marker = -INT_MAX;
    max_fac_marker = -INT_MAX;
    max_tet_marker = -INT_MAX;
}

void mdl_msh::regularize_mesh()
{
    max_edg_marker = INT_MIN;
    max_fac_marker = INT_MIN;
    max_tet_marker = INT_MIN;
    map<pair<size_t, size_t>, size_t> edgesMap;
    map<tuple<size_t, size_t, size_t>, size_t> facesMap;
    if (strcmp(type.c_str(), "EDGE") == 0)
    { // if not edges
        for (size_t i = 0; i < n_edges; i++)
        {
            sort(edg_nodes[i].begin(), edg_nodes[i].end());
            edgesMap[make_pair(edg_nodes[i][0], edg_nodes[i][1])] = i;
        }
        fac_edges.clear();
        edg_adj_fac.clear();
        fac_edges.resize(n_faces);
        edg_adj_fac.resize(n_edges);
        for (size_t i = 0; i < n_faces; i++)
        {
            sort(fac_nodes[i].begin(), fac_nodes[i].end());
            fac_edges[i].push_back(
                edgesMap[make_pair(fac_nodes[i][1], fac_nodes[i][2])]);
            fac_edges[i].push_back(
                edgesMap[make_pair(fac_nodes[i][0], fac_nodes[i][2])]);
            fac_edges[i].push_back(
                edgesMap[make_pair(fac_nodes[i][0], fac_nodes[i][1])]);
            facesMap[make_tuple(fac_nodes[i][0], fac_nodes[i][1],
                                fac_nodes[i][2])] = i;
            for (size_t j = 0; j < 3; j++)
            {
                edg_adj_fac[fac_edges[i][j]].push_back(i);
            }
        }
    }
    if (strcmp(type.c_str(), "TETRA") == 0)
    {
        tet_edges.clear();
        tet_faces.clear();
        fac_adj_tet.clear();
        tet_edges.resize(n_tetras);
        tet_faces.resize(n_tetras);
        fac_adj_tet.resize(n_faces);
        // get_mesh_statistics();
        for (size_t i = 0; i < n_tetras; i++)
        {
            sort(tet_nodes[i].begin(), tet_nodes[i].end());
            tet_edges[i].push_back(
                edgesMap[make_pair(tet_nodes[i][0], tet_nodes[i][1])]);
            tet_edges[i].push_back(
                edgesMap[make_pair(tet_nodes[i][0], tet_nodes[i][2])]);
            tet_edges[i].push_back(
                edgesMap[make_pair(tet_nodes[i][0], tet_nodes[i][3])]);
            tet_edges[i].push_back(
                edgesMap[make_pair(tet_nodes[i][1], tet_nodes[i][2])]);
            tet_edges[i].push_back(
                edgesMap[make_pair(tet_nodes[i][1], tet_nodes[i][3])]);
            tet_edges[i].push_back(
                edgesMap[make_pair(tet_nodes[i][2], tet_nodes[i][3])]);
            tet_faces[i].push_back(facesMap[make_tuple(
                tet_nodes[i][1], tet_nodes[i][2], tet_nodes[i][3])]);
            tet_faces[i].push_back(facesMap[make_tuple(
                tet_nodes[i][0], tet_nodes[i][2], tet_nodes[i][3])]);
            tet_faces[i].push_back(facesMap[make_tuple(
                tet_nodes[i][0], tet_nodes[i][1], tet_nodes[i][3])]);
            tet_faces[i].push_back(facesMap[make_tuple(
                tet_nodes[i][0], tet_nodes[i][1], tet_nodes[i][2])]);
            for (size_t j = 0; j < 4; j++)
            {
                fac_adj_tet[tet_faces[i][j]].push_back(i);
            }
        }
    }
    for (size_t i = 0; i < edg_lab.size(); i++)
        max_edg_marker = max(max_edg_marker, edg_lab[i]);
    for (size_t i = 0; i < fac_lab.size(); i++)
        max_fac_marker = max(max_fac_marker, fac_lab[i]);
    for (size_t i = 0; i < tet_lab.size(); i++)
        max_tet_marker = max(max_tet_marker, tet_lab[i]);
    // get_mesh_statistics();
}

void mdl_msh::complete_mesh()
{
    map<pair<size_t, size_t>, size_t> edgesMap;
    map<tuple<size_t, size_t, size_t>, size_t> facesMap;
    vector<size_t> tmpEdg(2), tmpFac(3);
    pair<size_t, size_t> newEdg;
    tuple<size_t, size_t, size_t> newFac;
    size_t eidx = 0, fidx = 0;
    // edg_nodes.clear();
    for (size_t i = 0; i < edg_nodes.size(); i++)
    {
        sort(edg_nodes[i].begin(), edg_nodes[i].end());
        edgesMap[make_pair(edg_nodes[i][0], edg_nodes[i][1])] = i;
    }
    for (size_t i = 0; i < fac_nodes.size(); i++)
    {
        sort(fac_nodes[i].begin(), fac_nodes[i].end());
        facesMap[make_tuple(fac_nodes[i][0], fac_nodes[i][1], fac_nodes[i][2])] = i;
    }

    // populating maps
    for (size_t i = 0; i < n_tetras; i++)
    {
        sort(tet_nodes[i].begin(), tet_nodes[i].end());
        newEdg = make_pair(tet_nodes[i][0], tet_nodes[i][1]);
        if (edgesMap.find(newEdg) == edgesMap.end())
        {
            edgesMap[newEdg] = eidx++;
            edg_nodes.push_back(vector<size_t>{newEdg.first, newEdg.second});
        }
        newEdg = make_pair(tet_nodes[i][0], tet_nodes[i][2]);
        if (edgesMap.find(newEdg) == edgesMap.end())
        {
            edgesMap[newEdg] = eidx++;
            edg_nodes.push_back(vector<size_t>{newEdg.first, newEdg.second});
        }
        newEdg = make_pair(tet_nodes[i][0], tet_nodes[i][3]);
        if (edgesMap.find(newEdg) == edgesMap.end())
        {
            edgesMap[newEdg] = eidx++;
            edg_nodes.push_back(vector<size_t>{newEdg.first, newEdg.second});
        }
        newEdg = make_pair(tet_nodes[i][1], tet_nodes[i][2]);
        if (edgesMap.find(newEdg) == edgesMap.end())
        {
            edgesMap[newEdg] = eidx++;
            edg_nodes.push_back(vector<size_t>{newEdg.first, newEdg.second});
        }
        newEdg = make_pair(tet_nodes[i][1], tet_nodes[i][3]);
        if (edgesMap.find(newEdg) == edgesMap.end())
        {
            edgesMap[newEdg] = eidx++;
            edg_nodes.push_back(vector<size_t>{newEdg.first, newEdg.second});
        }
        newEdg = make_pair(tet_nodes[i][2], tet_nodes[i][3]);
        if (edgesMap.find(newEdg) == edgesMap.end())
        {
            edgesMap[newEdg] = eidx++;
            edg_nodes.push_back(vector<size_t>{newEdg.first, newEdg.second});
        }
        newFac = make_tuple(tet_nodes[i][1], tet_nodes[i][2], tet_nodes[i][3]);
        if (facesMap.find(newFac) == facesMap.end())
        {
            facesMap[newFac] = fidx++;
            fac_nodes.push_back(vector<size_t>{get<0>(newFac), get<1>(newFac), get<2>(newFac)});
        }
        newFac = make_tuple(tet_nodes[i][0], tet_nodes[i][2], tet_nodes[i][3]);
        if (facesMap.find(newFac) == facesMap.end())
        {
            facesMap[newFac] = fidx++;
            fac_nodes.push_back(vector<size_t>{get<0>(newFac), get<1>(newFac), get<2>(newFac)});
        }
        newFac = make_tuple(tet_nodes[i][0], tet_nodes[i][1], tet_nodes[i][3]);
        if (facesMap.find(newFac) == facesMap.end())
        {
            facesMap[newFac] = fidx++;
            fac_nodes.push_back(vector<size_t>{get<0>(newFac), get<1>(newFac), get<2>(newFac)});
        }
        newFac = make_tuple(tet_nodes[i][0], tet_nodes[i][1], tet_nodes[i][2]);
        if (facesMap.find(newFac) == facesMap.end())
        {
            facesMap[newFac] = fidx++;
            fac_nodes.push_back(vector<size_t>{get<0>(newFac), get<1>(newFac), get<2>(newFac)});
        }
    }
    n_edges = edg_nodes.size();
    n_faces = fac_nodes.size();
    // populating tets related data
    tet_edges.clear();
    tet_faces.clear();
    fac_adj_tet.clear();
    tet_edges.resize(n_tetras);
    tet_faces.resize(n_tetras);
    // get_mesh_statistics();
    for (size_t i = 0; i < n_tetras; i++)
    {
        sort(tet_nodes[i].begin(), tet_nodes[i].end());
        tet_edges[i].push_back(
            edgesMap[make_pair(tet_nodes[i][0], tet_nodes[i][1])]);
        tet_edges[i].push_back(
            edgesMap[make_pair(tet_nodes[i][0], tet_nodes[i][2])]);
        tet_edges[i].push_back(
            edgesMap[make_pair(tet_nodes[i][0], tet_nodes[i][3])]);
        tet_edges[i].push_back(
            edgesMap[make_pair(tet_nodes[i][1], tet_nodes[i][2])]);
        tet_edges[i].push_back(
            edgesMap[make_pair(tet_nodes[i][1], tet_nodes[i][3])]);
        tet_edges[i].push_back(
            edgesMap[make_pair(tet_nodes[i][2], tet_nodes[i][3])]);
        tet_faces[i].push_back(facesMap[make_tuple(
            tet_nodes[i][1], tet_nodes[i][2], tet_nodes[i][3])]);
        tet_faces[i].push_back(facesMap[make_tuple(
            tet_nodes[i][0], tet_nodes[i][2], tet_nodes[i][3])]);
        tet_faces[i].push_back(facesMap[make_tuple(
            tet_nodes[i][0], tet_nodes[i][1], tet_nodes[i][3])]);
        tet_faces[i].push_back(facesMap[make_tuple(
            tet_nodes[i][0], tet_nodes[i][1], tet_nodes[i][2])]);
    }
    fac_adj_tet.resize(n_faces);
    for (size_t i = 0; i < n_tetras; i++)
        for (size_t j = 0; j < 4; j++)
            fac_adj_tet[tet_faces[i][j]].push_back(i);
    fac_edges.resize(n_faces);
    for (size_t i = 0; i < n_faces; i++)
    {
        sort(fac_nodes[i].begin(), fac_nodes[i].end());
        fac_edges[i].push_back(
            edgesMap[make_pair(fac_nodes[i][1], fac_nodes[i][2])]);
        fac_edges[i].push_back(
            edgesMap[make_pair(fac_nodes[i][0], fac_nodes[i][2])]);
        fac_edges[i].push_back(
            edgesMap[make_pair(fac_nodes[i][0], fac_nodes[i][1])]);
    }
    edg_adj_fac.resize(n_edges);
    for (size_t i = 0; i < n_faces; i++)
        for (size_t j = 0; j < 3; j++)
            edg_adj_fac[fac_edges[i][j]].push_back(i);
    // populating domains data
    n_domains = 1;
    dom_tetras.resize(1);
    dom_faces.resize(1);
    for (size_t i = 0; i < n_domains; i++){
        for( size_t j = 0; j < n_tetras; j++)
            dom_tetras[i].push_back(j);
        for( size_t j = 0; j < n_faces; j++)
            if (fac_adj_tet[j].size() == 1)
                dom_faces[i].push_back(j);
    }
}

void mdl_msh::refine_homogeneous()
{
    vector<vector<double>> new_nod_pos(n_nodes + n_edges,
                                       vector<double>(3));
    vector<vector<size_t>> new_tet_nodes(n_tetras * 8,
                                         vector<size_t>(4));
    vector<vector<size_t>> new_tet_edges(n_tetras * 8,
                                         vector<size_t>(6));
    vector<vector<size_t>> new_tet_faces(n_tetras * 8,
                                         vector<size_t>(4));
    vector<vector<size_t>> new_fac_nodes(n_faces * 4 + n_tetras * 8,
                                         vector<size_t>(3));
    vector<vector<size_t>> new_fac_edges(n_faces * 4 + n_tetras * 8,
                                         vector<size_t>(3));
    vector<vector<size_t>> new_edg_nodes(
        n_edges * 2 + n_faces * 3 + n_tetras, vector<size_t>(2));
    vector<int> new_edg_lab(n_edges * 2 + n_faces * 3 + n_tetras, 0);
    vector<int> new_fac_lab(n_faces * 4 + n_tetras * 8, 0);
    vector<int> new_tet_lab(n_tetras * 8, 0);
    vector<vector<size_t>> new_edg_adj_fac(n_edges * 2 + n_faces * 3 +
                                           n_tetras);
    vector<vector<size_t>> new_fac_adj_tet(n_faces * 4 + n_tetras * 8);
    //
    map<pair<size_t, size_t>, size_t> edgesMap;
    map<tuple<size_t, size_t, size_t>, size_t> facesMap;
    //
    for (size_t nid = 0; nid < n_nodes; nid++)
        new_nod_pos[nid] = nod_pos[nid];
    for (size_t eid = 0; eid < n_edges; eid++)
    {
        new_nod_pos[n_nodes + eid][0] =
            (nod_pos[edg_nodes[eid][0]][0] + nod_pos[edg_nodes[eid][1]][0]) / 2;
        new_nod_pos[n_nodes + eid][1] =
            (nod_pos[edg_nodes[eid][0]][1] + nod_pos[edg_nodes[eid][1]][1]) / 2;
        new_nod_pos[n_nodes + eid][2] =
            (nod_pos[edg_nodes[eid][0]][2] + nod_pos[edg_nodes[eid][1]][2]) / 2;
    }
    size_t tet_lvl = 0;
    size_t fac_lvl = 0;
    size_t edg_lvl = 0;
    if (strcmp(type.data(), "TRIA") == 0)
    {
        vector<size_t> nod_glob(6), edg_tmp(2), fac_tmp(3);
        for (size_t fid = 0; fid < n_faces; fid++)
        {
            for (unsigned int i = 0; i < 3; i++)
            {
                nod_glob[i] = fac_nodes[fid][i];
                nod_glob[3 + i] = n_nodes + fac_edges[fid][i];
            }
            // populating edges
            for (unsigned int i = 0; i < 9; i++)
            {
                edg_tmp[0] = nod_glob[fac_edg_nodes_loc_map_id[2 * i]];
                edg_tmp[1] = nod_glob[fac_edg_nodes_loc_map_id[2 * i + 1]];
                sort(edg_tmp.begin(), edg_tmp.end());
                if (edgesMap.find(make_pair(edg_tmp[0], edg_tmp[1])) ==
                    edgesMap.end())
                {
                    edgesMap[make_pair(edg_tmp[0], edg_tmp[1])] = edg_lvl;
                    new_edg_nodes[edg_lvl][0] = edg_tmp[0];
                    new_edg_nodes[edg_lvl][1] = edg_tmp[1];
                    if (fac_edg_lab_loc_map_id[i] > -1)
                    {
                        new_edg_lab[edg_lvl] =
                            edg_lab[fac_edges[fid][fac_edg_lab_loc_map_id[i]]];
                    }
                    ++edg_lvl;
                }
            }
            // populating faces
            for (unsigned int i = 0; i < 4; i++)
            {
                fac_tmp[0] = nod_glob[fac_fac_nodes_loc_map_id[3 * i]];
                fac_tmp[1] = nod_glob[fac_fac_nodes_loc_map_id[3 * i + 1]];
                fac_tmp[2] = nod_glob[fac_fac_nodes_loc_map_id[3 * i + 2]];
                sort(fac_tmp.begin(), fac_tmp.end());
                new_fac_nodes[fac_lvl][0] = fac_tmp[0];
                new_fac_nodes[fac_lvl][1] = fac_tmp[1];
                new_fac_nodes[fac_lvl][2] = fac_tmp[2];
                new_fac_edges[fac_lvl][0] =
                    edgesMap[make_pair(fac_tmp[1], fac_tmp[2])];
                new_fac_edges[fac_lvl][1] =
                    edgesMap[make_pair(fac_tmp[0], fac_tmp[2])];
                new_fac_edges[fac_lvl][2] =
                    edgesMap[make_pair(fac_tmp[0], fac_tmp[1])];
                new_fac_lab[fac_lvl] = fac_lab[fid];
                for (size_t j = 0; j < 3; j++)
                {
                    size_t eid = new_fac_edges[fac_lvl][j];
                    new_edg_adj_fac[eid].push_back(fac_lvl);
                }
                ++fac_lvl;
            }
        }
    }
    if (strcmp(type.data(), "TETRA") == 0)
    {
        vector<size_t> nod_glob(10), edg_tmp(2), fac_tmp(3), tet_tmp(4);
        for (size_t tid = 0; tid < n_tetras; tid++)
        {
            for (unsigned int i = 0; i < 4; i++)
                nod_glob[i] = tet_nodes[tid][i];
            for (unsigned int i = 0; i < 6; i++)
                nod_glob[4 + i] = n_nodes + tet_edges[tid][i];
            // populating edges
            for (unsigned int i = 0; i < 25; i++)
            {
                edg_tmp[0] = nod_glob[tet_edg_nodes_loc_map_id[2 * i]];
                edg_tmp[1] = nod_glob[tet_edg_nodes_loc_map_id[2 * i + 1]];
                sort(edg_tmp.begin(), edg_tmp.end());
                if (edgesMap.find(make_pair(edg_tmp[0], edg_tmp[1])) ==
                    edgesMap.end())
                {
                    edgesMap[make_pair(edg_tmp[0], edg_tmp[1])] = edg_lvl;
                    new_edg_nodes[edg_lvl][0] = edg_tmp[0];
                    new_edg_nodes[edg_lvl][1] = edg_tmp[1];
                    ++edg_lvl;
                }
            }
            // populating faces
            for (unsigned int i = 0; i < 24; i++)
            {
                fac_tmp[0] = nod_glob[tet_fac_nodes_loc_map_id[3 * i]];
                fac_tmp[1] = nod_glob[tet_fac_nodes_loc_map_id[3 * i + 1]];
                fac_tmp[2] = nod_glob[tet_fac_nodes_loc_map_id[3 * i + 2]];
                sort(fac_tmp.begin(), fac_tmp.end());
                if (facesMap.find(make_tuple(fac_tmp[0], fac_tmp[1],
                                             fac_tmp[2])) == facesMap.end())
                {
                    facesMap[make_tuple(fac_tmp[0], fac_tmp[1], fac_tmp[2])] =
                        fac_lvl;
                    new_fac_nodes[fac_lvl][0] = fac_tmp[0];
                    new_fac_nodes[fac_lvl][1] = fac_tmp[1];
                    new_fac_nodes[fac_lvl][2] = fac_tmp[2];
                    new_fac_edges[fac_lvl][0] =
                        edgesMap[make_pair(fac_tmp[1], fac_tmp[2])];
                    new_fac_edges[fac_lvl][1] =
                        edgesMap[make_pair(fac_tmp[0], fac_tmp[2])];
                    new_fac_edges[fac_lvl][2] =
                        edgesMap[make_pair(fac_tmp[0], fac_tmp[1])];
                    if (tet_fac_lab_loc_map_id[i] > -1)
                    {
                        new_fac_lab[fac_lvl] =
                            fac_lab[tet_faces[tid][tet_fac_lab_loc_map_id[i]]];
                    }
                    ++fac_lvl;
                }
            }
            for (unsigned int i = 0; i < 8; i++)
            {
                for (unsigned int j = 0; j < 4; j++)
                {
                    tet_tmp[j] = nod_glob[tet_tet_nodes_loc_map_id[i * 4 + j]];
                }
                sort(tet_tmp.begin(), tet_tmp.end());
                for (unsigned int j = 0; j < 4; j++)
                {
                    new_tet_nodes[tet_lvl][j] = tet_tmp[j];
                }
                new_tet_lab[tet_lvl] = tet_lab[tid];
                new_tet_edges[tet_lvl][0] =
                    edgesMap[make_pair(tet_tmp[0], tet_tmp[1])];
                new_tet_edges[tet_lvl][1] =
                    edgesMap[make_pair(tet_tmp[0], tet_tmp[2])];
                new_tet_edges[tet_lvl][2] =
                    edgesMap[make_pair(tet_tmp[0], tet_tmp[3])];
                new_tet_edges[tet_lvl][3] =
                    edgesMap[make_pair(tet_tmp[1], tet_tmp[2])];
                new_tet_edges[tet_lvl][4] =
                    edgesMap[make_pair(tet_tmp[1], tet_tmp[3])];
                new_tet_edges[tet_lvl][5] =
                    edgesMap[make_pair(tet_tmp[2], tet_tmp[3])];
                new_tet_faces[tet_lvl][0] =
                    facesMap[make_tuple(tet_tmp[1], tet_tmp[2], tet_tmp[3])];
                new_tet_faces[tet_lvl][1] =
                    facesMap[make_tuple(tet_tmp[0], tet_tmp[2], tet_tmp[3])];
                new_tet_faces[tet_lvl][2] =
                    facesMap[make_tuple(tet_tmp[0], tet_tmp[1], tet_tmp[3])];
                new_tet_faces[tet_lvl][3] =
                    facesMap[make_tuple(tet_tmp[0], tet_tmp[1], tet_tmp[2])];
                for (size_t j = 0; j < 4; j++)
                {
                    size_t fid = new_tet_faces[tet_lvl][j];
                    new_fac_adj_tet[fid].push_back(tet_lvl);
                }
                ++tet_lvl;
            }
        }
    }
    nod_pos = new_nod_pos;
    edg_nodes = new_edg_nodes;
    edg_lab = new_edg_lab;
    edg_adj_fac = new_edg_adj_fac;
    fac_nodes = new_fac_nodes;
    fac_edges = new_fac_edges;
    fac_lab = new_fac_lab;
    fac_adj_tet = new_fac_adj_tet;
    tet_nodes = new_tet_nodes;
    tet_edges = new_tet_edges;
    tet_faces = new_tet_faces;
    tet_lab = new_tet_lab;
    n_nodes = n_nodes + n_edges;
    n_edges = n_edges * 2 + n_faces * 3 + n_tetras;
    n_faces = n_faces * 4 + n_tetras * 8;
    n_tetras = n_tetras * 8;
}

void mdl_msh::save_vtk_mesh(string vtkMshName)
{
    unsigned int n_bc_mtrl_faces = 0;
    for (size_t i = 0; i < n_faces; i++)
    {
        if (fac_lab[i] > -1)
            ++n_bc_mtrl_faces;
        else if (fac_adj_tet[i].size() > 1)
        {
            if (tet_lab[fac_adj_tet[i][0]] != tet_lab[fac_adj_tet[i][1]])
            {
                --fac_lab[i];
                ++n_bc_mtrl_faces;
            }
        }
    }
    ofstream out_vol_msh(string(vtkMshName + "_volmsh.vtk").data());
    out_vol_msh << "# vtk DataFile Version 2.0\n";
    out_vol_msh << "Mesh data\n";
    out_vol_msh << "ASCII\n";
    out_vol_msh << "DATASET UNSTRUCTURED_GRID\n";
    out_vol_msh << "POINTS " << n_nodes << " double \n";
    for (size_t i = 0; i < n_nodes; i++)
    {
        out_vol_msh << setprecision(15) << nod_pos[i][0] << " ";
        out_vol_msh << setprecision(15) << nod_pos[i][1] << " ";
        out_vol_msh << setprecision(15) << nod_pos[i][2] << "\n";
    }
    out_vol_msh << "CELLS " << n_tetras << " " << 5 * n_tetras << "\n";
    for (size_t i = 0; i < n_tetras; i++)
    {
        out_vol_msh << 4 << " ";
        out_vol_msh << tet_nodes[i][0] << " ";
        out_vol_msh << tet_nodes[i][1] << " ";
        out_vol_msh << tet_nodes[i][2] << " ";
        out_vol_msh << tet_nodes[i][3] << "\n";
    }
    out_vol_msh << "CELL_TYPES " << n_tetras << "\n";
    for (size_t i = 0; i < n_tetras; i++)
    {
        out_vol_msh << 10 << "\n";
    }
    out_vol_msh << "CELL_DATA " << n_tetras << "\n";
    out_vol_msh << "SCALARS "
                << "Materials int 1\n";
    out_vol_msh << "LOOKUP_TABLE default\n";
    for (size_t i = 0; i < n_tetras; i++)
    {
        out_vol_msh << tet_lab[i] << "\n";
    }
    out_vol_msh.close();

    ofstream out_srf_msh(string(vtkMshName + "_srfmsh.vtk").data());
    out_srf_msh << "# vtk DataFile Version 2.0\n";
    out_srf_msh << "Mesh data\n";
    out_srf_msh << "ASCII\n";
    out_srf_msh << "DATASET UNSTRUCTURED_GRID\n";
    out_srf_msh << "POINTS " << n_nodes << " double \n";
    for (size_t i = 0; i < n_nodes; i++)
    {
        out_srf_msh << setprecision(15) << nod_pos[i][0] << " ";
        out_srf_msh << setprecision(15) << nod_pos[i][1] << " ";
        out_srf_msh << setprecision(15) << nod_pos[i][2] << "\n";
    }
    out_srf_msh << "CELLS " << n_bc_mtrl_faces << " " << 4 * n_bc_mtrl_faces
                << "\n";
    for (size_t i = 0; i < n_faces; i++)
    {
        if (fac_lab[i] != -1)
        {
            out_srf_msh << 3 << " ";
            out_srf_msh << fac_nodes[i][0] << " ";
            out_srf_msh << fac_nodes[i][1] << " ";
            out_srf_msh << fac_nodes[i][2] << "\n";
        }
    }
    out_srf_msh << "CELL_TYPES " << n_bc_mtrl_faces << "\n";
    for (size_t i = 0; i < n_faces; i++)
    {
        if (fac_lab[i] != -1)
            out_srf_msh << 5 << "\n";
    }
    out_srf_msh << "CELL_DATA " << n_bc_mtrl_faces << "\n";
    out_srf_msh << "SCALARS "
                << "Boundaries int 1\n";
    out_srf_msh << "LOOKUP_TABLE default\n";
    for (size_t i = 0; i < n_faces; i++)
    {
        if (fac_lab[i] != -1)
            out_srf_msh << fac_lab[i] << "\n";
    }
    out_srf_msh.close();
    for (size_t i = 0; i < n_faces; i++)
        if (fac_lab[i] < -1)
            fac_lab[i] = -1;
}

vector<vector<double>> mdl_msh::tet_geo(size_t id)
{
    vector<vector<double>> cGeo(4, vector<double>(3));
    cGeo[0] = nod_pos[tet_nodes[id][0]];
    cGeo[1] = nod_pos[tet_nodes[id][1]];
    cGeo[2] = nod_pos[tet_nodes[id][2]];
    cGeo[3] = nod_pos[tet_nodes[id][3]];
    return cGeo;
}

vector<vector<double>> mdl_msh::fac_geo(size_t id)
{
    vector<vector<double>> cGeo(3, vector<double>(3));
    cGeo[0] = nod_pos[fac_nodes[id][0]];
    cGeo[1] = nod_pos[fac_nodes[id][1]];
    cGeo[2] = nod_pos[fac_nodes[id][2]];
    return cGeo;
}

vector<vector<double>> mdl_msh::fac_geo2(size_t id)
{
    vector<double> v0 = nod_pos[fac_nodes[id][0]];
    vector<double> v1 = nod_pos[fac_nodes[id][1]];
    vector<double> v2 = nod_pos[fac_nodes[id][2]];
    for (unsigned int i = 0; i < 3; i++)
    {
        v1[i] -= v0[i];
        v2[i] -= v0[i];
    }
    vector<double> u = v1;
    vector<double> n(3), v(3);
    n[0] = v1[1] * v2[2];
    n[1] = v1[2] * v2[0];
    n[2] = v1[0] * v2[1];
    double norm2_u = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
    double norm2_n = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    for (unsigned int i = 0; i < 3; i++)
    {
        u[i] /= norm2_u;
        n[i] /= norm2_n;
    }
    v[0] = n[1] * u[2];
    v[1] = n[2] * u[0];
    v[2] = n[0] * u[1];
    vector<vector<double>> cGeo2(3, vector<double>(2, 0.0));
    cGeo2[1][0] = v1[0] * u[0] + v1[1] * u[1] + v1[2] * u[2];
    cGeo2[2][0] = v2[0] * u[0] + v2[1] * u[1] + v2[2] * u[2];
    cGeo2[2][1] = v2[0] * v[0] + v2[1] * v[1] + v2[2] * v[2];
    return cGeo2;
}

vector<vector<double>> mdl_msh::edg_geo(size_t id)
{
    vector<vector<double>> cGeo(2, vector<double>(3));
    cGeo[0] = nod_pos[edg_nodes[id][0]];
    cGeo[1] = nod_pos[edg_nodes[id][1]];
    return cGeo;
}

vector<double> mdl_msh::int_node(size_t id)
{
    vector<size_t> nfac = fac_nodes[id];
    vector<size_t> ntet = tet_nodes[fac_adj_tet[id][0]];
    size_t intid;
    for (unsigned int i = 0; i < 4; i++)
    {
        bool found = true;
        intid = ntet[i];
        for (unsigned int j = 0; j < 3; j++)
            if (ntet[i] == nfac[j])
            {
                found = false;
            }
        if (found)
        {
            break;
        }
    }
    return nod_pos[intid];
}

vector<double> mdl_msh::int_node(size_t id, size_t &ref_face)
{
    vector<size_t> nfac = fac_nodes[id];
    vector<size_t> ntet = tet_nodes[fac_adj_tet[id][ref_face]];
    size_t intid = 0;
    for (unsigned int i = 0; i < 4; i++)
    {
        bool found = true;
        intid = ntet[i];
        for (unsigned int j = 0; j < 3; j++)
            if (ntet[i] == nfac[j])
            {
                found = false;
            }
        if (found)
        {
            ref_face = i;
            break;
        }
    }
    return nod_pos[intid];
}

// double mdl_msh::tet_vol(size_t id) {
////         1      [ax bx cx dx]T
////    V = --- det [ay by cy dy]
////         6      [az bz cz dz]
////                [ 1  1  1  1]
//    vector<vector<double> > matrix = tet_geo(id);
//    matrix.resize(4,4);
//    matrix.col(3).fill(1.0);
//    return abs(arma::det(matrix]]/6.0;
//}
//
// double mdl_msh::tet_mean_edg_length(size_t id) {
//    vector<vector<double> > matrix = tet_geo(id);
//    return (arma::norm(matrix[1)-matrix[0),2) +
//            arma::norm(matrix[2)-matrix[0),2) +
//            arma::norm(matrix[3)-matrix[0),2) +
//            arma::norm(matrix[2)-matrix[1),2) +
//            arma::norm(matrix[3)-matrix[1),2) +
//            arma::norm(matrix[3)-matrix[2),2]] / 6.0;
//}
//
// double mdl_msh::tet_max_edg_length(size_t id) {
//    vector<vector<double> > matrix = tet_geo(id);
//    double max_edge = 0.0;
//    max_edge = max(max_edge, arma::norm(matrix[1)-matrix[0),2]];
//    max_edge = max(max_edge, arma::norm(matrix[2)-matrix[0),2]];
//    max_edge = max(max_edge, arma::norm(matrix[3)-matrix[0),2]];
//    max_edge = max(max_edge, arma::norm(matrix[2)-matrix[1),2]];
//    max_edge = max(max_edge, arma::norm(matrix[3)-matrix[1),2]];
//    max_edge = max(max_edge, arma::norm(matrix[3)-matrix[2),2]];
//    return max_edge;
//}

void mdl_msh::get_bounding_info()
{
    bounding_box.resize(2);
    bounding_box[0].assign(3, DBL_MAX);
    bounding_box[1].assign(3, -DBL_MAX);
    for (size_t i = 0; i < nod_pos.size(); i++)
    {
        bounding_box[0][0] = min(bounding_box[0][0], nod_pos[i][0]);
        bounding_box[0][1] = min(bounding_box[0][1], nod_pos[i][1]);
        bounding_box[0][2] = min(bounding_box[0][2], nod_pos[i][2]);
        bounding_box[1][0] = max(bounding_box[1][0], nod_pos[i][0]);
        bounding_box[1][1] = max(bounding_box[1][1], nod_pos[i][1]);
        bounding_box[1][2] = max(bounding_box[1][2], nod_pos[i][2]);
    }
    double x_dim, y_dim, z_dim;
    x_dim = abs(bounding_box[1][0] - bounding_box[0][0]);
    y_dim = abs(bounding_box[1][1] - bounding_box[0][1]);
    z_dim = abs(bounding_box[1][2] - bounding_box[0][2]);
    max_dimension = max(max_dimension, x_dim);
    max_dimension = max(max_dimension, y_dim);
    max_dimension = max(max_dimension, z_dim);
}

double mdl_msh::get_geom_dim()
{
    double dim = 0;
    for (unsigned int i = 0; i < nod_pos.size(); i++)
    {
        dim = max(dim, abs(nod_pos[i][0]));
        dim = max(dim, abs(nod_pos[i][1]));
        dim = max(dim, abs(nod_pos[i][2]));
    }
    dim *= 1e-1;
    for (int i = -15; i < 15; i++)
    {
        if (dim < pow(10, double(i)))
            return pow(10, double(i));
    }
    return 0;
}
