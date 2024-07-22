#pragma once

#ifdef TECS_BUILD_DLL
#define TECS_API __declspec(dllexport)

#else
#define TECS_API __declspec(dllimport)

#endif