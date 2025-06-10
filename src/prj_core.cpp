#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>
#include <limits.h>
#elif _WIN32
#include <iphlpapi.h>
#include <psapi.h>
#include <windows.h>
#include <winsock2.h>
#else
#error "OS not supported!"
#endif

#include "prj_core.h"
#include "phys_core.h"

prj_core::prj_core(const std::string& name) : std::filesystem::path(name)
{
    tic();
    std::cout << "--- Compute Residuals ---" << std::endl;
    std::cout << get_loc_time() << std::endl;
    std::cout << get_info() << std::endl;
    std::cout << "--------------------------------" << std::endl;
    std::cout << "Opening " << this->c_str() << std::endl;
    model.import( parent_path().string(), stem().string(), extension().string());
    std::cout << get_stats(*this);
}

prj_core::~prj_core()
{
}


std::string prj_core::get_info()
{
    std::stringstream tag;
    std::string host, user, memory, cores, threads;
#if defined(__linux__) || defined(__APPLE__)
    char hostname[64];
	char login_r[32];
    gethostname(hostname, 64);
	//getlogin_r(login_r, LOGIN_NAME_MAX);
    const char* username = getlogin();
    const char* userenv = getenv("USER");
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
	if (hostname != NULL)
		host = std::string(hostname);
	if (username != NULL)
		user = std::string(username);
	else if (login_r != NULL)
		user = std::string(login_r);
    else if (userenv != NULL)
        user = std::string(userenv);
    memory = std::to_string(pages * page_size / 1048576);
    cores = std::to_string(std::thread::hardware_concurrency());
    threads = std::to_string(std::thread::hardware_concurrency());
#elif _WIN32
    host = get_var("COMPUTERNAME");
    user = get_var("USER");
    cores = get_var("NUMBER_OF_PROCESSORS");
    threads = get_var("OMP_NUM_THREADS");
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex))
        memory = statex.ullAvailPhys / 1048576;
#else
#error "OS not supported!"
#endif
    tag << "Machine = " << host << " (" << user << ")\n";
    tag << "Memory  = " << memory << " MB\n";
    tag << "Cores   = " << cores; // << "\n";
    // tag << "Threads = " << threads;
    return tag.str();
}

#ifdef _WIN32
std::string prj_core::get_var(const std::string name)
{
    char *ptr = getenv(name.c_str());
    std::string ret;
    if (ptr == NULL)
    {
        ret = std::string("");
    }
    else
    {
        ret = std::string(ptr);
    }
    return ret;
}

int prj_core::get_int(const std::string name)
{
    const std::string data = get_var(name);
    int ret = -1;
    if (data.size() != 0)
    {
        ret = atoi(data.c_str());
    }
    return ret;
}
#endif

std::string prj_core::get_proc_mem()
{
    std::stringstream out;
    double physiPeak, physiPres;
#if defined(__linux__) || defined(__APPLE__)
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    int who = RUSAGE_SELF;
    struct rusage usage;
    int ret = getrusage(who, &usage);
    physiPeak = pages * page_size / 1048576;
    physiPres = usage.ru_maxrss / 1024;
#elif _WIN32
    HANDLE hProcess = GetCurrentProcess();
    if (NULL == hProcess)
    {
        out << "Memory stats: Failed to acquire process handle";
        return out.str();
    }
    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        out << "Memory stats: Failed to acquire process memory information";
    }
    else
    {
        physiPeak = (double)pmc.PeakWorkingSetSize / 1048576;
        physiPres = (double)pmc.WorkingSetSize / 1048576;
    }
    CloseHandle(hProcess);
#endif
    out << "Used RAM: " << physiPres << " MB |" << physiPeak << "|";
    return out.str();
}

std::string prj_core::get_loc_time()
{
    time_t ct = time(NULL);
    std::string time(asctime(localtime(&ct)));
    time.pop_back();
    return time;
}

std::string prj_core::get_stats(prj_timer &t)
{
    return "- " + get_proc_mem() + " - " + t.strtoc() + "\n";
}
