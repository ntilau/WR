#ifndef PHYS_CORE_H
#define PHYS_CORE_H

#include "prj_core.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

template <typename T> class sp_node {
public:
    sp_node(size_t i) : idx(i), val(0) {}
    ~sp_node() {}
    bool operator==(sp_node<T> tmp) {
        return idx == tmp.idx;
    }
    bool operator==(const sp_node<T> &tmp) const {
        return idx == tmp.idx;
    }
    T val;
    size_t idx;
};

template <typename T> class sp_vec : public vector<sp_node<T>> {
public:
    typedef typename vector<sp_node<T>>::iterator iterator;
    typedef typename vector<sp_node<T>>::const_iterator const_iterator;
    T &operator[](size_t i) {
        iterator it = find(this->begin(), this->end(), sp_node<T>(i));
        if (it == this->end()) {
            this->push_back(sp_node<T>(i));
            return this->back().val;
        }
        return it->val;
    }
    T operator[](size_t i) const {
        const_iterator it = find(this->begin(), this->end(), sp_node<T>(i));
        if (it == this->end())
            return T(0);
        else
            return it->val;
    }
    vector<T> full() {
        vector<T> tmp(this->dim(), 0);
        for (iterator it = this->begin(); it != this->end(); it++)
            tmp[it->idx] = it->val;
        return tmp;
    }
    size_t nnz() {
        size_t tmp = 0;
        for (iterator it = this->begin(); it != this->end(); it++)
            ++tmp;
        return tmp;
    }
    size_t dim() {
        size_t tmp = 0;
        for (iterator it = this->begin(); it != this->end(); it++)
            tmp = max(tmp, it->idx);
        return tmp + 1;
    }
};

template <typename T> ostream &operator<<(ostream &out, sp_vec<T> v) {
    out << "sparse vector\n";
    for (typename sp_vec<T>::iterator it = v.begin(); it != v.end(); it++) {
        out << "[" << it->idx << "]=" << it->val << "\n";
    }
    return out;
}

template <typename T> class sp_mat : public vector<sp_vec<T>> {
public:
    typedef typename vector<sp_vec<T>>::iterator iterator;
    T &operator()(size_t i, size_t j) {
        if (i >= this->size()) {
            for (size_t row = this->size(); row <= i; row++)
                this->push_back(sp_vec<T>());
        }
        return this->operator[](i)[j];
    }
    sp_vec<T> operator*(const sp_vec<T> &vec) {
        sp_vec<T> tmp;
        for (typename sp_mat<T>::iterator itr = this->begin(); itr != this->end();
             itr++) {
            for (typename sp_vec<T>::iterator itc = itr->begin(); itc != itr->end();
                 itc++) {
                T val = itc->val * vec[itc->idx];
                if (val != T(0))
                    tmp[itr - this->begin()] += val;
            }
        }
        return tmp;
    }
    size_t nnz() {
        size_t tmp = 0;
        for (typename sp_mat<T>::iterator itr = this->begin(); itr != this->end();
             itr++)
            tmp += itr->nnz();
        return tmp;
    }
    void save(string name) {
        ofstream mat_file(name.data());
        for (typename sp_mat<T>::iterator itr = this->begin(); itr != this->end();
             itr++) {
            for (typename sp_vec<T>::iterator itc = itr->begin(); itc != itr->end();
                 itc++) {
                mat_file << itr - this->begin() + 1 << " " << itc->idx + 1 << " "
                         << setprecision(16) << itc->val << "\n";
            }
        }
        mat_file.close();
    }
};

template <typename T> ostream &operator<<(ostream &out, sp_mat<T> v) {
    out << "sparse matrix\n";
    for (typename sp_mat<T>::iterator itr = v.begin(); itr != v.end(); itr++) {
        for (typename sp_vec<T>::iterator itc = itr->begin(); itc != itr->end();
             itc++) {
            out << "[" << itr - v.begin() << "][" << itc->idx << "]=" << itc->val
                << "\n";
        }
    }
    return out;
}

template <typename T>
ostream &operator<<(ostream &out, const vector<T> &vec) {
    // out << "matrix " << mat.val.size() << "\n";
    out << "{ ";
    for (size_t i = 0; i < vec.size(); i++)
        out << "[" << i << "] " << vec[i] << " ";
    out << "}\n";
    return out;
}

template <typename T>
ostream &operator<<(ostream &out,
                         const vector<vector<T>> &mat) {
    // out << "matrix " << mat.val.size() << "\n";
    out << "{ ";
    for (size_t i = 0; i < mat.size(); i++)
        out << "[" << i << "] " << mat[i] << " ";
    out << "}\n";
    return out;
}

template <typename T>
vector<vector<T>> operator*(const vector<vector<T>> &l,
const vector<vector<T>> &r) {
    unsigned int m = l.size(), n = r[0].size(), p = r.size();
    vector<vector<T>> tmp(m, vector<T>(n, 0));
    for (unsigned int i = 0; i < m; i++)
        for (unsigned int j = 0; j < n; j++)
            for (unsigned int k = 0; k < p; k++)
                tmp[i][j] += l[i][k] * r[k][j];
    return tmp;
}

template <typename T>
vector<vector<T>> operator^(const vector<vector<T>> &l,
const vector<vector<T>> &r) {
    unsigned int m = l.size(), n = r.size(), p = r[0].size();
    vector<vector<T>> tmp(m, vector<T>(n, 0));
    for (unsigned int i = 0; i < m; i++)
        for (unsigned int j = 0; j < n; j++)
            for (unsigned int k = 0; k < p; k++)
                tmp[i][j] += l[i][k] * r[j][k];
    return tmp;
}

template <typename T>
vector<vector<T>> &operator+=(vector<vector<T>> &l,
const vector<vector<T>> &r) {
    unsigned int m = l.size(), n = l[0].size();
    for (unsigned int i = 0; i < m; i++)
        for (unsigned int j = 0; j < n; j++)
            l[i][j] += r[i][j];
    return l;
}

template <typename T>
vector<vector<T>> operator*(const vector<vector<T>> &l,
const T &r) {
    unsigned int m = l.size(), n = l[0].size();
    vector<vector<T>> tmp(l);
    for (unsigned int i = 0; i < m; i++)
        for (unsigned int j = 0; j < n; j++)
            tmp[i][j] *= r;
    return tmp;
}

template <typename T>
vector<vector<T>> &operator*=(vector<vector<T>> &l,
const T &r) {
    unsigned int m = l.size(), n = l[0].size();
    for (unsigned int i = 0; i < m; i++)
        for (unsigned int j = 0; j < n; j++)
            l[i][j] *= r;
    return l;
}

extern "C" void dnaupd_(int *ido, char *problem, int *n, char *whichArr,
                        int *nev, double *tol, double *residArr, int *ncv,
                        double *vMatrix, int *ldv, int *iparamArr,
                        int *ipntrArr, double *workdArr, double *worklArr,
                        int *lworkl, int *info);

extern "C" void dneupd_(int *rvec, char *stringArr, int *selectArr,
                        double *dArrReal, double *dArrImag, double *vMatrix,
                        int *ldv, double *sigmaReal, double *sigmaImag,
                        double *workev, char *bmat, int *n, char *whichArr,
                        int *nev, double *tol, double *residArr, int *ncv,
                        double *vMatrix1, int *ldv1, int *iparamArr,
                        int *ipntrArr, double *workdArr, double *worklArr,
                        int *lworkl, int *ierr);

extern "C" void znaupd_(int *ido, char *bmat, int *n, char *which, int *nev,
                        double *tol, complex<double> *resid, int *ncv,
                        complex<double> *v, int *ldv, int *iparam,
                        int *ipntr, complex<double> *workd,
                        complex<double> *workl, int *lworkl, double *rwork,
                        int *info);

extern "C" void zneupd_(int *rvec, char *All, int *select,
                        complex<double> *d, complex<double> *v,
                        int *ldv, double *sigma, complex<double> *workev,
                        char *bmat, int *n, char *which, int *nev, double *tol,
                        complex<double> *resid, int *ncv,
                        complex<double> *v1, int *ldv1, int *iparam,
                        int *ipntr, complex<double> *workd,
                        complex<double> *workl, int *lworkl, double *rwork,
                        int *ierr);

class solver {
public:
    solver(mdl_core &mdl);
    virtual ~solver();

protected:
    enum type { HGRAD, HCURL, HDIV, EYE, GRAD, CURL, DIV, EDGE, TRIA, TETRA };
    void compute_label_map();
    void compute_dof_num();
    void solve_eig_zmumps(sp_mat<complex<double>> &a,
                          sp_mat<complex<double>> &b, size_t &n_t,
                          size_t &n_z, double &kk, int &n_modes);
    void solve_dmumps(sp_mat<double> &matrix, sp_mat<double> &rhs);
    void solve_zmumps(sp_mat<complex<double>> &matrix,
                      sp_mat<complex<double>> &rhs);
    vector<size_t> get_dofs(type s, type e, size_t id);
    pair<double, vector<vector<double>>>
    get_jac(vector<vector<double>> geo);
    vector<vector<double>> get_shape(type s, type e, type o,
                                  vector<double> pos);
    vector<vector<double>>
                                  get_stiff_mat(type s, type e, unsigned int i,
                                          pair<double, vector<vector<double>>> &jac);
    vector<vector<double>>
                                  get_mass_mat(type s, type e, unsigned int i,
                                          pair<double, vector<vector<double>>> &jac);
    void analyze_em_e_eig();
    void analyze_em_e_fd();
    void analyze_e_v_stat();
    void analyze_h_a_stat();

private:
    mdl_core &mdl;
    struct DOFS {
        size_t hgrad = 0, hcurl = 0, hdiv = 0;
    } dof_num;
    map<int, unsigned int> mtrl_map, bc_map;
};

#endif // PHYS_CORE_H
