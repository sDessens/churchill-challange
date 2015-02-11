#include "dll.h"

#include "solution.h"
#include "timer.h"

//std::vector<double> m_times;

SearchContext* create(const Point *points_begin, const Point *points_end)
{
//    m_times.reserve(1000);
    return (SearchContext*)new Solution(points_begin, points_end);
}

point_index search(SearchContext *sc, const Rect rect, const point_index count, Point *out_points)
{
    Solution* sol = (Solution*)sc;
//    rdtsc_timer timer;
    auto result = sol->search(rect, count, out_points);
//    m_times.push_back(timer.elapsed());
    return result;
}


SearchContext *destroy(SearchContext *sc)
{
//    for(double d : m_times)
//    {
//        std::cout << d * 1000 << std::endl;
//    }
//    std::cout << "====" << std::endl;
//    m_times.clear();

    delete (Solution*)sc;
    return nullptr;
}
