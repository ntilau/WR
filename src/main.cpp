#define __cpp_lib_filesystem 201703

#include "prj_core.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 2)
        {
            cout << "Usage: fes /path/to/project_name.ext" << endl;
        }
        else
        {
            prj_core p(argv[1]);
        }
    }
    catch (string error)
    {
        cout << error << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
