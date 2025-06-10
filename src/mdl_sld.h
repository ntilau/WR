#ifndef MDL_SLD_H
#define MDL_SLD_H

#include <climits>
#include <cstdint>
#include <vector>
#include <string>


using namespace std;

class mdl_sld
{
public:
    mdl_sld();
    ~mdl_sld();
    void clear();
    bool read_poly_file(string &name);
    bool read_stl_file(string &name);
    void write_prj_file(string &name);
    void read_prj_file(string &name);
    unsigned int check_dim();
    double get_geom_dim();
    double max_dimension = 0.0;
    vector<vector<double>> bounding_box;
    void get_bounding_info();
    /// poly based modeling primitives
    vector<vector<double>> nodes;
    vector<vector<size_t>> edges;
    vector<vector<int>> edges_marker;
    struct face_type
    {
        vector<vector<size_t>> polygons;
        vector<vector<double>> holes;
    };
    vector<face_type> faces;
    vector<vector<int>> faces_marker;
    vector<vector<double>> faces_normals;
    vector<vector<double>> holes;
    vector<vector<double>> regions;
    vector<int> regions_marker;
    int max_edges_marker = INT_MIN, 
        max_faces_marker = INT_MIN,
        max_regions_marker = INT_MIN;
    unsigned int dim = 0;
    string tetgen_command = "pqaAfee";
    string triangle_command = "pqaAe";
    vector<int> bc_markers;
    vector<int> mtrl_markers;
    string sld_name;
};

#endif // MDL_SLD_H