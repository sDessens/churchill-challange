#ifndef DLL_H
#define DLL_H


#ifdef CHURCHILL_EXPORTS
#define CHURCHILL_API __declspec(dllexport)
#else
#define CHURCHILL_API __declspec(dllimport)
#endif

#include "point_search.h"

extern "C" {

CHURCHILL_API SearchContext* __stdcall create(const Point* points_begin, const Point* points_end);
CHURCHILL_API point_index __stdcall search(SearchContext* sc, const Rect rect, const point_index count, Point* out_points);
CHURCHILL_API SearchContext* __stdcall destroy(SearchContext* sc);

}


#endif // DLL_H
