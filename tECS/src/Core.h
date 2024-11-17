#pragma once

#ifdef TECS_BUILD_DLL
#define TECS_API __declspec(dllexport)

#define TECS_API

#else
#define TECS_API __declspec(dllimport)

#endif