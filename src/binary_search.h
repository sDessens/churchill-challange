#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include "point_search.h"
#include "aligned_allocator.h"

#include "util.h"

#include <vector>
#include <algorithm>



class bin_search {
public:
    bin_search() {}


    point_index lower_bound(float value) const {return lower_bound(value, 0, (point_index)m_values.size());}
    point_index lower_bound(float value, point_index first, point_index last) const;

    point_index upper_bound(float value) const {return upper_bound(value, 0, (point_index)m_values.size());}
    point_index upper_bound(float value, point_index first, point_index last) const;

    point_index size() const {return m_values.size();}
    const float* values() const {return m_values.data();}
    const float* other_values() const {return m_other_values.data();}
    const point_index* indices() const {return m_indices.data();}


    template<typename It>
    friend bin_search make_bin_search_from_x(It first, It last);
    template<typename It>
    friend bin_search make_bin_search_from_y(It first, It last);

private:
    std::vector<float, aligned_allocator<float, 64>> m_values; // sorted
    std::vector<float, aligned_allocator<float, 64>> m_other_values;
    std::vector<point_index, aligned_allocator<float, 64>> m_indices;
};

inline point_index bin_search::lower_bound(float value, point_index first, point_index last) const
{
    auto it = std::lower_bound(begin(m_values) + first,
                               begin(m_values) + last,
                               value,
                               [](float a, float b){return a < b;});
    return (point_index)std::distance(begin(m_values), it);
}

inline point_index bin_search::upper_bound(float value, point_index first, point_index last) const
{
    auto it = std::upper_bound(begin(m_values) + first,
                               begin(m_values) + last,
                               value,
                               [](float a, float b){return a < b;});
    return (point_index)std::distance(begin(m_values), it);
}

template<typename It>
bin_search make_bin_search_from_x(It first, It last)
{
    std::vector<Point> points(first, last);
    std::sort(begin(points), end(points), util::point_less_x);
    bin_search result;
    result.m_values.reserve(points.size());
    result.m_other_values.reserve(points.size());
    result.m_indices.reserve(points.size());
    for(Point p : points) {
        result.m_values.push_back(p.x);
        result.m_other_values.push_back(p.y);
        result.m_indices.push_back(p.rank);
    }
    return std::move(result);
}

template<typename It>
bin_search make_bin_search_from_y(It first, It last)
{
    std::vector<Point> points(first, last);
    std::sort(begin(points), end(points), util::point_less_y);
    bin_search result;
    result.m_values.reserve(points.size());
    result.m_other_values.reserve(points.size());
    result.m_indices.reserve(points.size());
    for(Point p : points) {
        result.m_values.push_back(p.y);
        result.m_other_values.push_back(p.x);
        result.m_indices.push_back(p.rank);
    }
    return std::move(result);
}

#endif // BINARY_SEARCH_H
