#pragma once

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else   
    #define EXPORT
#endif

extern "C"{
    typedef void(*UpdateFunc)();

    EXPORT void init();
    EXPORT void update();
    EXPORT void shutdown();
}