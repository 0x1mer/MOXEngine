#pragma once

#ifdef MOX_EXPORTS
#define MOX_API __declspec(dllexport)
#else
#define MOX_API __declspec(dllimport)
#endif
