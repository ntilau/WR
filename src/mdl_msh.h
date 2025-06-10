#ifndef MDL_MSH_H
#define MDL_MSH_H

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

class mdl_msh
{
public:
    mdl_msh();
    mdl_msh(mdl_msh *); // copy constructor
    ~mdl_msh();
    vector<string> mesh_type = {"TRIA", "TETRA"};
    string type = "TETRA";
    // methods
    double get_geom_dim();
    double max_dimension = 0.0;
    vector<vector<double>> bounding_box;
    void get_bounding_info();
    void write_prj_file(string &name);
    void read_prj_file(string &name);
    void read_tetgen_files(string &name);
    void read_triangle_files(string &name);
    void save_vtk_mesh(string);
    void regularize_mesh();
    void complete_mesh();
    void refine_homogeneous();
    void get_mesh_statistics();
    void clear();
    //
    vector<vector<double>> tet_geo(size_t);
    vector<vector<double>> fac_geo(size_t);
    vector<vector<double>> fac_geo2(size_t);
    vector<vector<double>> edg_geo(size_t);
    vector<double> int_node(size_t);
    vector<double> int_node(size_t, size_t &);
    // members
    vector<vector<size_t> > tet_nodes;
    vector<vector<size_t> > tet_edges;
    vector<vector<size_t> > tet_faces;
    vector<vector<size_t> > fac_nodes;
    vector<vector<size_t> > fac_edges;
    vector<vector<size_t> > edg_nodes;
    vector<vector<double> > nod_pos;
    vector<int> edg_lab;
    vector<int> fac_lab;
    vector<int> tet_lab;
    vector<vector<size_t>> fac_adj_tet;
    vector<vector<size_t>> edg_adj_fac;
    vector<vector<size_t>> dom_tetras;
    vector<vector<size_t>> dom_faces;
    size_t n_nodes = 0, n_edges = 0, n_faces = 0, n_tetras = 0, n_domains = 0;
    int max_edg_marker = -SIZE_MAX, max_fac_marker = -SIZE_MAX, max_tet_marker = -SIZE_MAX;
};

#endif // MDL_MSH_H