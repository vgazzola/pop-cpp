/**
 * File : testplugin.cc
 * Author : Tuan Anh Nguyen
 * Description : test for plugin
 * Creation date : -
 *
 * Modifications :
 * Authors      Date            Comment
 * P.Kuonen     18.9.2012       Add "POP-C++ error" in error messages (PEKA)
 */

#include <dlfcn.h>
#include <stdio.h>
#include "pop_buffer_factory.h"
#include "pop_buffer_factory_finder.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage:  parocplugin module0 module2 ...\n");
        return 1;
    }
    pop_buffer_factory* (*CreateFactory)();

    for (int i = 1; i < argc; i++) {
        void* handler = dlopen(argv[i], RTLD_NOW | RTLD_LOCAL);
        if (handler == NULL) {
            LOG_ERROR("POP-C++ Error on dlopen(%s): %s", argv[i], dlerror());
            continue;
        }
        CreateFactory = (pop_buffer_factory * (*)())dlsym(handler, "ParocBufferFactory");

        if (CreateFactory == NULL) {
            LOG_ERROR("POP-C++ Error %s: Can not locate ParocBufferFactory", argv[i]);
        } else {
            pop_buffer_factory* test = CreateFactory();
            if (test == NULL) {
                LOG_ERROR("POP-C++ Error: Fail to create a buffer factory");
            } else {
                std::string str;
                test->GetBufferName(str);
                LOG_INFO("Buffer name:%s", str);
                delete test;
            }
        }
        dlclose(handler);
    }
    return 0;
}
