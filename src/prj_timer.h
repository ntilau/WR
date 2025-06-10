#ifndef PRJ_TIMER_H
#define PRJ_TIMER_H

#include <time.h>
#include <string>

class prj_timer
{
public:
    prj_timer()
    {
        tic();
    }
    ~prj_timer() {}
    void tic()
    {
        lc = clock();
    }
    double toc()
    {
        clock_t cc = clock();
        return (cc - lc) / 1000.0;
    }
    std::string strtoc()
    {
        clock_t cc = clock();
        std::stringstream timing;
        timing << (cc - lc) / 1000.0 << " s";
        return timing.str();
    }

private:
    clock_t lc;
};

#endif // PRJ_TIMER_H