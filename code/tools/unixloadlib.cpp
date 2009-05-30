// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void error_and_exit(char const* fn, char const* path)
{
    bool close = true;
    FILE *out = popen("c++filt", "w");
    if (!out)
    {
        out = stdout;
        close = false;
    }
    fprintf(out, "%s - failed for %s:\n%s\n", fn, path, dlerror());
    if (close)
        pclose(out);

    exit(1);
}

int main(int argc, char** argv)
{
    if (argc == 1)
        return 1;

    void* module = dlopen(argv[1], RTLD_NOW | RTLD_GLOBAL);
    if (!module)
        error_and_exit("dlopen", argv[1]);

    if (dlclose(module))
        error_and_exit("dlclose", argv[1]);

    return 0;
}



/** EOF @file */
