#ifndef POINT_SEARCH_H
#define POINT_SEARCH_H

/* Given 10 million uniquely ranked points on a 2D plane, design a datastructure and an algorithm that can find the 20
most important points inside any given rectangle. The solution has to be reasonably fast even in the worst case, while
also not using an unreasonably large amount of memory.
Create an x64 Windows DLL, that can be loaded by the test application. The DLL and the functions will be loaded
dynamically. The DLL should export the following functions: "create", "search" and "destroy". The signatures and
descriptions of these functions are given below. You can use any language or compiler, as long as the resulting DLL
implements this interface. */

/* This standard header defines the sized types used. */
#include <stdint.h>

/* The following structs are packed with no padding. */
#pragma pack(push, 1)

typedef int32_t point_index;

/* Defines a point in 2D space with some additional attributes like id and rank. */
struct Point
{
	int8_t id;
    point_index rank;
	float x;
	float y;
};


/* Defines a rectangle, where a point (x,y) is inside, if x is in [lx, hx] and y is in [ly, hy]. */
struct Rect
{
	float lx;
	float ly;
	float hx;
	float hy;
};
#pragma pack(pop)

/* Declaration of the struct that is used as the context for the calls. */
struct SearchContext;

/* Load the provided points into an internal data structure. The pointers follow the STL iterator convention, where
"points_begin" points to the first element, and "points_end" points to one past the last element. The input points are
only guaranteed to be valid for the duration of the call. Return a pointer to the context that can be used for
consecutive searches on the data. */
typedef SearchContext* (__stdcall* T_create)(const Point* points_begin, const Point* points_end);

/* Search for "count" points with the smallest ranks inside "rect" and copy them ordered by smallest rank first in
"out_points". Return the number of points copied. "out_points" points to a buffer owned by the caller that
can hold "count" number of Points. */
typedef int32_t (__stdcall* T_search)(SearchContext* sc, const Rect rect, const int32_t count, Point* out_points);

/* Release the resources associated with the context. Return nullptr if successful, "sc" otherwise. */
typedef SearchContext* (__stdcall* T_destroy)(SearchContext* sc);


#endif
