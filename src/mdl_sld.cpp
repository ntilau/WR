#include "mdl_sld.h"

#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include <climits>
#include <cfloat>
#include <cstring>
#include <stdio.h>
#include <string>
#include <iomanip>
#include <math.h>

mdl_sld::mdl_sld() {}

mdl_sld::~mdl_sld() {}

double mdl_sld::get_geom_dim()
{
    double dimx, dimy, dimz;
    vector<vector<double>> transpose(3, vector<double>(nodes.size()));
    for (unsigned int i = 0; i < nodes.size(); i++)
        for (unsigned int j = 0; j < 3; j++)
            transpose[j][i] = nodes[i][j];
    dimx = *max_element(transpose[0].begin(), transpose[0].end()) -
           *min_element(transpose[0].begin(), transpose[0].end());
    dimy = *max_element(transpose[1].begin(), transpose[1].end()) -
           *min_element(transpose[1].begin(), transpose[1].end());
    dimz = *max_element(transpose[2].begin(), transpose[2].end()) -
           *min_element(transpose[2].begin(), transpose[2].end());
    return dimx * dimy * dimz;
}

bool mdl_sld::read_stl_file(string &name)
{
    clear();
    map<tuple<double, double, double>, size_t> nod_map;
    bool is_binary = false;
    string tmp_str, tmp_str_sld;
    string line;
    vector<double> tmp_pnt(3);
    vector<size_t> tmp_tri(3);
    size_t nod_cnt = 0, fac_cnt = 0;
    ifstream stl_file(string(name + ".stl").data());
    if (stl_file.is_open())
    {
        getline(stl_file, line);
        istringstream iss;
        iss.str(line);
        iss >> tmp_str_sld;
        if (tmp_str_sld == "solid")
        {
            iss >> sld_name;
            cout << "Solid name: " << sld_name << "\n";
            dim = 3;
            do
            {
                getline(stl_file, line);
                iss.clear();
                iss.str(line);
                iss >> tmp_str; // facet
                if (tmp_str == "endsolid")
                {
                    getline(stl_file, line);
                    iss.clear();
                    iss.str(line);
                    iss >> tmp_str; // facet
                    if (tmp_str == "endsolid")
                        break;
                    else if (stl_file.eof())
                        break;
                }
                iss >> tmp_str;    // normal
                iss >> tmp_pnt[0]; // ni
                iss >> tmp_pnt[1]; // nj
                iss >> tmp_pnt[2]; // nk
                faces_normals.push_back(tmp_pnt);
                getline(stl_file, line);
                iss.clear();
                iss.str(line);
                iss >> tmp_str; // outer
                iss >> tmp_str; // loop
                getline(stl_file, line);
                iss.clear();
                iss.str(line);
                iss >> tmp_str;    // vertex
                iss >> tmp_pnt[0]; // v1x
                iss >> tmp_pnt[1]; // v1y
                iss >> tmp_pnt[2]; // v1z
                if (nod_map.find(make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])) ==
                    nod_map.end())
                {
                    nod_map[make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])] =
                        nod_cnt++;
                }
                tmp_tri[0] =
                    nod_map[make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])];
                getline(stl_file, line);
                iss.clear();
                iss.str(line);
                iss >> tmp_str;    // vertex
                iss >> tmp_pnt[0]; // v2x
                iss >> tmp_pnt[1]; // v2y
                iss >> tmp_pnt[2]; // v2z
                if (nod_map.find(make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])) ==
                    nod_map.end())
                {
                    nod_map[make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])] =
                        nod_cnt++;
                }
                tmp_tri[1] =
                    nod_map[make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])];
                getline(stl_file, line);
                iss.clear();
                iss.str(line);
                iss >> tmp_str;    // vertex
                iss >> tmp_pnt[0]; // v3x
                iss >> tmp_pnt[1]; // v3y
                iss >> tmp_pnt[2]; // v3z
                if (nod_map.find(make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])) ==
                    nod_map.end())
                {
                    nod_map[make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])] =
                        nod_cnt++;
                }
                tmp_tri[2] =
                    nod_map[make_tuple(tmp_pnt[0], tmp_pnt[1], tmp_pnt[2])];
                face_type fac_ply;
                fac_ply.polygons.push_back(tmp_tri);
                faces.push_back(fac_ply);
                faces_marker.push_back(vector<int>(1, 1));
                fac_cnt++;
                getline(stl_file, line); // endloop
                getline(stl_file, line); // endfacet
            } while (!stl_file.eof());
            stl_file.close();
            cout << "Nodes = " << nod_cnt << "\n";
            cout << "Faces = " << fac_cnt << "\n";
            nodes.resize(nod_cnt);
            for (map<tuple<double, double, double>, size_t>::iterator it =
                     nod_map.begin();
                 it != nod_map.end(); it++)
            {
                vector<double> node(3);
                node[0] = get<0>(it->first);
                node[1] = get<1>(it->first);
                node[2] = get<2>(it->first);
                nodes[it->second] = node;
            }
            max_faces_marker = 1;
            bc_markers.push_back(1);
        }
        else
        {
            is_binary = true;
        }
    }
    else
    {
        cout << name + ".stl not found\n";
        return false;
    }
    return true;
}

bool mdl_sld::read_poly_file(string &name)
{
    clear();
    size_t tmp_int, n_pts;
    double tmp_dbl;
    string tmp_str;
    string line;
    ifstream poly_file(string(name + ".poly").data());
    if (poly_file.is_open())
    {
        {
            do
            {
                getline(poly_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            istringstream iss(line);
            iss >> tmp_int;
            nodes.resize(tmp_int);
            iss >> n_pts;
            for (size_t i = 0; i < nodes.size(); i++)
            {
                do
                {
                    getline(poly_file, line);
                } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
                istringstream iss(line);
                iss >> tmp_int;
                iss >> tmp_dbl;
                nodes[i].push_back(tmp_dbl);
                iss >> tmp_dbl;
                nodes[i].push_back(tmp_dbl);
                iss >> tmp_dbl;
                if (n_pts < 3)
                    nodes[i].push_back(0.0);
                else
                    nodes[i].push_back(tmp_dbl);
            }
        }
        dim = check_dim();
        cout << "Model is " << dim << "-dimensional\n";
        if (dim == 3)
        {
            do
            {
                getline(poly_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            istringstream iss(line);
            iss >> tmp_int;
            faces.resize(tmp_int);
            faces_marker.resize(tmp_int);
            unsigned int n_faces_marker;
            iss >> n_faces_marker;
            for (size_t i = 0; i < faces.size(); i++)
            {
                int polygons = 0, hole = 0, bmark = 0;
                do
                {
                    getline(poly_file, line);
                } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
                istringstream iss(line);
                iss >> polygons;
                iss >> hole;
                iss >> bmark;
                max_faces_marker = max(max_faces_marker, bmark);
                if (find(bc_markers.begin(), bc_markers.end(), bmark) ==
                    bc_markers.end())
                    bc_markers.push_back(bmark);
                faces_marker[i].push_back(bmark);
                faces[i].polygons.resize(polygons);
                if (hole > 0)
                    faces[i].holes.resize(hole);
                for (size_t j = 0; j < polygons; j++)
                {
                    unsigned int nodes_nbr = 0;
                    do
                    {
                        getline(poly_file, line);
                    } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
                    istringstream iss(line);
                    iss >> nodes_nbr;
                    for (unsigned int k = 0; k < nodes_nbr; k++)
                    {
                        iss >> tmp_int;
                        if (faces[i].polygons[j].size() > 0)
                        {
                            if (faces[i].polygons[j].back() == (tmp_int - 1))
                            {
                                do
                                {
                                    getline(poly_file, line);
                                } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
                                iss.clear();
                                iss.str("");
                                iss.str(line);
                                iss >> tmp_int;
                            }
                        }
                        faces[i].polygons[j].push_back(tmp_int - 1);
                    }
                }
                for (size_t j = 0; j < hole; j++)
                {
                    unsigned int nodes_nbr = 0;
                    do
                    {
                        getline(poly_file, line);
                    } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
                    istringstream iss(line);
                    iss >> tmp_int;
                    iss >> tmp_dbl;
                    faces[i].holes[j].push_back(tmp_dbl);
                    iss >> tmp_dbl;
                    faces[i].holes[j].push_back(tmp_dbl);
                    iss >> tmp_dbl;
                    faces[i].holes[j].push_back(tmp_dbl);
                }
            }
        }
        else if (dim == 2)
        {
            do
            {
                getline(poly_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            istringstream iss(line);
            iss >> tmp_int;
            edges.resize(tmp_int);
            edges_marker.resize(tmp_int);
            iss >> tmp_int;
            size_t n_edges_marker = tmp_int;
            for (size_t i = 0; i < edges.size(); i++)
            {
                do
                {
                    getline(poly_file, line);
                } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
                istringstream iss(line);
                iss >> tmp_int;
                iss >> tmp_int;
                edges[i].push_back(tmp_int - 1);
                iss >> tmp_int;
                edges[i].push_back(tmp_int - 1);
                for (unsigned int j = 0; j < n_edges_marker; j++)
                {
                    iss >> tmp_int;
                    int bmark = tmp_int;
                    max_edges_marker = max(max_edges_marker, bmark);
                    edges_marker[i].push_back(bmark);
                    if (find(bc_markers.begin(), bc_markers.end(), bmark) ==
                        bc_markers.end())
                        bc_markers.push_back(bmark);
                }
            }
        }
        // importing holes
        {
            do
            {
                getline(poly_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            istringstream iss(line);
            iss >> tmp_int;
            if (tmp_int > 0)
            {
                holes.resize(tmp_int);
                for (size_t i = 0; i < holes.size(); i++)
                {
                    do
                    {
                        getline(poly_file, line);
                    } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
                    istringstream iss(line);
                    iss >> tmp_int;
                    iss >> tmp_dbl;
                    holes[i].push_back(tmp_dbl);
                    iss >> tmp_dbl;
                    holes[i].push_back(tmp_dbl);
                    if (dim == 2)
                        holes[i].push_back(0.0);
                }
            }
        }
        // importing regions
        {
            do
            {
                getline(poly_file, line);
            } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
            istringstream iss(line);
            iss >> tmp_int;
            if (tmp_int > 0)
            {
                regions.resize(tmp_int);
                regions_marker.resize(tmp_int);
                for (size_t i = 0; i < regions.size(); i++)
                {
                    do
                    {
                        getline(poly_file, line);
                    } while (line.compare(0, 1, "#") == 0 || line.size() == 0);
                    istringstream iss(line);
                    iss >> tmp_int;
                    iss >> tmp_dbl;
                    regions[i].push_back(tmp_dbl);
                    iss >> tmp_dbl;
                    regions[i].push_back(tmp_dbl);
                    if (dim == 3)
                    {
                        iss >> tmp_dbl;
                        regions[i].push_back(tmp_dbl);
                    }
                    else if (dim == 2)
                        regions[i].push_back(0.0);
                    iss >> regions_marker[i];
                    max_regions_marker = max(max_regions_marker, regions_marker[i]);
                    if (find(mtrl_markers.begin(), mtrl_markers.end(),
                                  regions_marker[i]) == mtrl_markers.end())
                        mtrl_markers.push_back(regions_marker[i]);
                }
            }
        }
        sort(bc_markers.begin(), bc_markers.end());
        sort(mtrl_markers.begin(), mtrl_markers.end());
        return true;
    }
    else
    {
        cout << name + ".poly not found\n";
        return false;
    }
}

void mdl_sld::clear()
{
    vector<vector<double>>().swap(nodes);
    vector<vector<size_t>>().swap(edges);
    vector<vector<int>>().swap(edges_marker);
    vector<face_type>().swap(faces);
    vector<vector<int>>().swap(faces_marker);
    vector<vector<double>>().swap(holes);
    vector<vector<double>>().swap(regions);
    vector<int>().swap(regions_marker);
    vector<vector<double>>().swap(bounding_box);
    vector<int>().swap(bc_markers);
    vector<int>().swap(mtrl_markers);
    max_dimension = 0.0;
    dim = 0;
    max_edges_marker = -INT_MAX;
    max_faces_marker = -INT_MAX;
    max_regions_marker = -INT_MAX;
}

void mdl_sld::get_bounding_info()
{
    bounding_box.resize(2);
    bounding_box[0].assign(3, DBL_MAX);
    bounding_box[1].assign(3, -DBL_MAX);
    for (size_t i = 0; i < nodes.size(); i++)
    {
        bounding_box[0][0] = min(bounding_box[0][0], nodes[i][0]);
        bounding_box[0][1] = min(bounding_box[0][1], nodes[i][1]);
        bounding_box[0][2] = min(bounding_box[0][2], nodes[i][2]);
        bounding_box[1][0] = max(bounding_box[1][0], nodes[i][0]);
        bounding_box[1][1] = max(bounding_box[1][1], nodes[i][1]);
        bounding_box[1][2] = max(bounding_box[1][2], nodes[i][2]);
    }
    double x_dim, y_dim, z_dim;
    x_dim = abs(bounding_box[1][0] - bounding_box[0][0]);
    y_dim = abs(bounding_box[1][1] - bounding_box[0][1]);
    z_dim = abs(bounding_box[1][2] - bounding_box[0][2]);
    max_dimension = max(max_dimension, x_dim);
    max_dimension = max(max_dimension, y_dim);
    max_dimension = max(max_dimension, z_dim);
}

unsigned int mdl_sld::check_dim()
{
    double x, y, z;
    if (nodes.size() >= 1)
    {
        x = nodes[0][0];
        y = nodes[0][1];
        z = nodes[0][2];
    }
    double x_norm = 0, y_norm = 0, z_norm = 0;
    for (size_t i = 0; i < nodes.size(); i++)
    {
        x_norm += abs(nodes[i][0] * nodes[i][0]);
        y_norm += abs(nodes[i][1] * nodes[i][1]);
        z_norm += abs(nodes[i][2] * nodes[i][2]);
    }
    x_norm = sqrt(x_norm);
    y_norm = sqrt(y_norm);
    z_norm = sqrt(z_norm);
    if (x == x_norm || y == y_norm || z == z_norm)
        return 2;
    else
        return 3;
}

void mdl_sld::write_prj_file(string &name)
{
    ofstream sld_out_file(string(name + ".core").c_str(),
                               ios::out | ios::ate | ios::app);
    sld_out_file << "#Sld_Nodes " << nodes.size() << "\n";
    for (size_t i = 0; i < nodes.size(); i++)
    {
        sld_out_file << scientific << setprecision(16) << nodes[i][0]
                     << " " << setprecision(16) << nodes[i][1] << " "
                     << setprecision(16) << nodes[i][2] << "\n";
    }
    if (dim == 2)
    {
        sld_out_file << "#Sld_Edges " << edges.size() << " "
                     << edges_marker[0].size() << "\n";
        for (size_t i = 0; i < edges.size(); i++)
        {
            for (size_t j = 0; j < edges[i].size(); j++)
                sld_out_file << edges[i][j] << " ";
            for (size_t j = 0; j < edges_marker[i].size(); j++)
                sld_out_file << edges_marker[i][j] << " ";
            sld_out_file << "\n";
        }
    }
    if (dim == 3)
    {
        sld_out_file << "#Sld_Faces " << faces.size() << "\n";
        for (size_t i = 0; i < faces.size(); i++)
        {
            sld_out_file << faces[i].polygons.size() << " " << faces[i].holes.size()
                         << " " << faces_marker[i].back() << "\n";
            for (size_t j = 0; j < faces[i].polygons.size(); j++)
            {
                sld_out_file << faces[i].polygons[j].size() << " ";
                for (size_t k = 0; k < faces[i].polygons[j].size(); k++)
                {
                    sld_out_file << faces[i].polygons[j][k] << " ";
                }
                sld_out_file << "\n";
            }
            for (size_t j = 0; j < faces[i].holes.size(); j++)
            {
                sld_out_file << scientific << setprecision(16)
                             << faces[i].holes[j][0] << " " << setprecision(16)
                             << faces[i].holes[j][1] << " " << setprecision(16)
                             << faces[i].holes[j][2] << "\n";
            }
        }
    }
    sld_out_file << "#Sld_Holes " << holes.size() << "\n";
    for (size_t i = 0; i < holes.size(); i++)
    {
        sld_out_file << scientific << setprecision(16) << holes[i][0]
                     << " " << setprecision(16) << holes[i][1] << " "
                     << setprecision(16) << holes[i][2] << "\n";
    }
    sld_out_file << "#Sld_Regions " << regions.size() << "\n";
    for (size_t i = 0; i < regions.size(); i++)
    {
        sld_out_file << scientific << setprecision(16) << regions[i][0]
                     << " " << setprecision(16) << regions[i][1] << " "
                     << setprecision(16) << regions[i][2] << " "
                     << regions_marker[i] << "\n";
    }
    sld_out_file.close();
}

void mdl_sld::read_prj_file(string &name)
{
    clear();
    ifstream sld_in_file(string(name + ".core").c_str(), ios::in);
    string line;
    istringstream iss;
    unsigned int tmp_uint;
    double tmp_dbl;
    string tmp_str;
    if (sld_in_file.is_open())
    {
        while (getline(sld_in_file, line))
        {
            iss.clear();
            iss.str(line);
            iss >> tmp_str;
            if (strcmp(tmp_str.data(), "#Sld_Nodes") == 0)
            {
                iss >> tmp_uint;
                nodes.resize(tmp_uint);
                for (size_t i = 0; i < nodes.size(); i++)
                {
                    getline(sld_in_file, line);
                    iss.clear();
                    iss.str(line);
                    nodes[i].resize(3);
                    iss >> nodes[i][0];
                    iss >> nodes[i][1];
                    iss >> nodes[i][2];
                }
            }
            if (strcmp(tmp_str.data(), "#Sld_Edges") == 0)
            {
                dim = 2;
                iss >> tmp_uint;
                edges.resize(tmp_uint);
                edges_marker.resize(tmp_uint);
                iss >> tmp_uint;
                for (size_t i = 0; i < edges.size(); i++)
                {
                    edges[i].resize(2);
                    edges_marker[i].resize(tmp_uint);
                }
                for (size_t i = 0; i < edges.size(); i++)
                {
                    getline(sld_in_file, line);
                    iss.clear();
                    iss.str(line);
                    iss >> edges[i][0];
                    iss >> edges[i][1];
                    for (unsigned int j = 0; j < edges_marker[i].size(); j++)
                    {
                        iss >> edges_marker[i][j];
                        max_edges_marker = max(max_edges_marker, edges_marker[i][j]);
                        if (find(bc_markers.begin(), bc_markers.end(),
                                      edges_marker[i][j]) == bc_markers.end())
                            bc_markers.push_back(edges_marker[i][j]);
                    }
                }
            }
            if (strcmp(tmp_str.data(), "#Sld_Faces") == 0)
            {
                dim = 3;
                iss >> tmp_uint;
                faces.resize(tmp_uint);
                faces_marker.resize(tmp_uint);
                for (size_t i = 0; i < faces.size(); i++)
                {
                    getline(sld_in_file, line);
                    iss.clear();
                    iss.str(line);
                    unsigned int polygons, hole;
                    int bmark;
                    iss >> polygons;
                    iss >> hole;
                    iss >> bmark;
                    faces[i].polygons.resize(polygons);
                    faces[i].holes.resize(hole);
                    faces_marker[i].push_back(bmark);
                    max_faces_marker = max(max_faces_marker, bmark);
                    if (find(bc_markers.begin(), bc_markers.end(), bmark) ==
                        bc_markers.end())
                        bc_markers.push_back(bmark);
                    for (unsigned int j = 0; j < faces[i].polygons.size(); j++)
                    {
                        getline(sld_in_file, line);
                        iss.clear();
                        iss.str(line);
                        iss >> tmp_uint;
                        faces[i].polygons[j].resize(tmp_uint);
                        for (unsigned int k = 0; k < faces[i].polygons[j].size(); k++)
                            iss >> faces[i].polygons[j][k];
                    }
                    for (unsigned int j = 0; j < faces[i].holes.size(); j++)
                    {
                        getline(sld_in_file, line);
                        iss.clear();
                        iss.str(line);
                        faces[i].holes[j].resize(3);
                        for (unsigned int k = 0; k < 3; k++)
                            iss >> faces[i].holes[j][k];
                    }
                }
            }
            if (strcmp(tmp_str.data(), "#Sld_Holes") == 0)
            {
                iss >> tmp_uint;
                holes.resize(tmp_uint);
                for (size_t i = 0; i < holes.size(); i++)
                {
                    getline(sld_in_file, line);
                    iss.clear();
                    iss.str(line);
                    nodes[i].resize(3);
                    iss >> nodes[i][0];
                    iss >> nodes[i][1];
                    iss >> nodes[i][2];
                }
            }
            if (strcmp(tmp_str.data(), "#Sld_Regions") == 0)
            {
                iss >> tmp_uint;
                regions.resize(tmp_uint);
                regions_marker.resize(tmp_uint);
                for (size_t i = 0; i < regions.size(); i++)
                {
                    getline(sld_in_file, line);
                    iss.clear();
                    iss.str(line);
                    regions[i].resize(3);
                    iss >> regions[i][0];
                    iss >> regions[i][1];
                    iss >> regions[i][2];
                    iss >> regions_marker[i];
                    max_regions_marker = max(max_regions_marker, regions_marker[i]);
                    if (find(mtrl_markers.begin(), mtrl_markers.end(),
                                  regions_marker[i]) == mtrl_markers.end())
                        mtrl_markers.push_back(regions_marker[i]);
                }
            }
        }
    }
    sld_in_file.close();
}