#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#ifdef  _WIN32
#include <iphlpapi.h>
#include <psapi.h>
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#include <thread>
#include <unistd.h>
#endif

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 40
#endif
#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 40
#endif

class Config
{
public:
    inline static std::string get_info()
    {
        std::stringstream tag;
        std::string host, user, memory, cores, threads;
#ifdef _WIN32
        host = get_var("COMPUTERNAME");
        user = get_var("USER");
        cores = get_var("NUMBER_OF_PROCESSORS");
        threads = get_var("OMP_NUM_THREADS");
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        if (GlobalMemoryStatusEx(&statex))
            memory = statex.ullAvailPhys / 1048576;
#else
        char hostname[HOST_NAME_MAX];
        char login_r[LOGIN_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);
        getlogin_r(login_r, LOGIN_NAME_MAX);
        const char *username = getlogin();
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        if (hostname != NULL)
            host = std::string(hostname);
        if (username != NULL)
            user = std::string(username);
        else if (login_r != NULL)
            user = std::string(login_r);
        memory = std::to_string(pages * page_size / 1048576);
        cores = std::to_string(std::thread::hardware_concurrency());
        threads = std::to_string(std::thread::hardware_concurrency());
#endif
        tag << "Machine = " << host << " (" << user << ")\n";
        tag << "Memory  = " << memory << " MB\n";
        tag << "Cores   = " << cores; // << "\n";
        // tag << "Threads = " << threads;
        return tag.str();
    }

#ifdef _WIN32
    inline static std::string get_var(const std::string name)
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

    inline static int get_int(const std::string name)
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

    inline static std::string get_proc_mem()
    {
        std::stringstream out;
        double physiPeak, physiPres;
#ifdef __linux__
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
        out << "Used RAM: " << physiPres << " MB |" << physiPeak << "|\n";
        return out.str();
    }

};

#endif

