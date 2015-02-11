#ifndef SOLUTION_H
#define SOLUTION_H

#include "point_search.h"
#include "aligned_allocator.h"
#include "binary_search.h"
#include "rank_heap.h"

#include <vector>
#include <array>

class Solution {
public:
    Solution(const Point *points_begin, const Point *points_end);

    /**
     * run search_linear() for the first 1000-or-so points. If we haven't found
     * 'count' points yet, run search_mipmap()
     */
    point_index search(const Rect rect, const point_index count, Point *out_points);

    /**
     * Search linearly over all points. The points are sorted by rank.
     * Only a small percentage of all points are explored.
     */
    point_index search_linear(const Rect rect, const point_index count, Point *out_points);

    /**
     * Search over the data by using an mipmap like data structure.
     *
     * The data structure contains multiple mipmap levels. The first level (L)
     * contains N points. The second level (L+1) contains N*G points. Where G
     * is the growth factor. The growth factor is usually 2 or 3.
     *
     * Each level L contains the N points with the lowest ranking, each point
     * is present in exactly one mipmap level.
     *
     * Each mipmap level is stored twice, once sorted by the X dimension, once
     * sorted by the Y dimension. This allows us to binary search to the
     * start and end of the rect. We can then decide if we process the
     * X or Y mipmap for that level.
     *
     * Finally, there exists an 'cascading' mapping table which speeds up
     * the binary searches. This works by assuming the data of mipmap L+1
     * is likely to be uniformly distrubuted along L.
     */
    point_index search_mipmap(const Rect &rect, const point_index count, Point *out_points);

private:
    // an sorted vector of points. Sorted by rank.
    std::vector<Point> m_points;

    // data structures for linear scan
    std::vector<float, aligned_allocator<float, 64>> m_x_coord;
    std::vector<float, aligned_allocator<float, 64>> m_y_coord;

    // data structures for mipmap scan.
    std::vector<bin_search> m_x_mipmaps;
    std::vector<bin_search> m_y_mipmaps;

    // data structures to speed up searching in the mipmaps
    std::vector<std::vector<point_index>> m_x_lower_cascading;
    std::vector<std::vector<point_index>> m_x_upper_cascading;

    std::vector<std::vector<point_index>> m_y_lower_cascading;
    std::vector<std::vector<point_index>> m_y_upper_cascading;

    // max-heap used by the mipmaps.
    RankHeap m_heap;
};



#endif // SOLUTION_H
