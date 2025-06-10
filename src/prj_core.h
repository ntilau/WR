#ifndef PRJ_CORE_H
#define PRJ_CORE_H

#define FES_VERSION 0.0.0.1

#include <string>
#include <filesystem>

#include "mdl_core.h"
#include "prj_timer.h"
#include "prj_logger.h"

namespace fs = std::filesystem;

class prj_core : private fs::path, private prj_timer
{
public:
    prj_core(const std::string& name);
    ~prj_core();
    prj_logger log;
    // all the model information is stored here
    mdl_core model;

    std::string get_stats(prj_timer &); // Statistics
    std::string get_info();             // User and computer names, Max RAM, CPU cores and threads number
    std::string get_proc_mem();
    std::string get_loc_time();
    
private:
    std::string get_var(const std::string name);
    int get_int(const std::string name);
};

#endif // PRJ_CORE_H
