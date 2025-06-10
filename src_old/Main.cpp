#include <time.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <armadillo>

#include "Config.h"
//#include "Mem.h"
//#include "MAC.h"
#include "Option.h"
#include "Project.h"
#include "EqSys.h"

int main(int argc, char* argv[])
{
    time_t t;
    Option opt;
    opt.set(argc, argv);
    std::ofstream logFile(std::string(opt.name + "_Log.txt").data(), std::ios::app);
    try
    {
        std::cout << "----------------------------------------\n";
        std::cout << "                FE-Wave                 \n";
        std::cout << "----------------------------------------\n";
        opt.LIMITED = false;//!MAC::CheckMAC();
        t = time(NULL);
        logFile << "# START: " << asctime(localtime(&t)) << Config::get_info();
        /*
        if(opt.highp)
        {
            Config::SetPriorityRealTime();
        }
        Config::GetPriority(logFile);
        if(opt.verbose)
        {
            Config::GetPriority(std::cout);
        }
        MemStat::AvailableMemory(logFile);
        if(opt.verbose)
        {
            MemStat::AvailableMemory(std::cout);
        }
        */
        arma::wall_clock totTimer;
        totTimer.tic();
        Project prj(logFile, opt);
        EqSys cSys(logFile, &prj);
        if(opt.verbose)
        {
            std::cout << Config::get_proc_mem();
            std::cout << "++ " << totTimer.toc() << " s\n";
        }
        logFile << "++ " << totTimer.toc() << " s\n";
    }
    catch(std::string err)
    {
        t = time(NULL);
        std::cout << "### ERROR: " << err << " ###\n";
        logFile << "### ERROR: " << err << " ###\n";
        logFile << "# END: " << asctime(localtime(&t)) << "\n";
        logFile.close();
        std::cout << "\a\a\a\a\a";
        return EXIT_FAILURE;
    }
    t = time(NULL);
    logFile << "# END: " << asctime(localtime(&t)) << "\n";
    logFile.close();
    return EXIT_SUCCESS;
}
