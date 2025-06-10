#ifndef PRJ_LOGGER_H
#define PRJ_LOGGER_H

#include <sstream>

class prj_logger : public std::ostringstream
{
public:
    template <typename T>
    prj_logger &operator<<(T a)
    {
        oss << a;
        return *this;
    }
private:
    std::ostringstream oss;
};

#endif // PRJ_LOGGER_H