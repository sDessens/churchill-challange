#ifndef UTIL_H
#define UTIL_H

#include "point_search.h"

#include <algorithm>
#include <iostream>

static std::ostream &operator<<(std::ostream &os, const Point &p) {
    return os << "point(" << p.x << ", " << p.y << ", " << p.rank << ")";
}

static std::ostream &operator<<(std::ostream &os, const Rect &r) {
    return os << "rect([" << r.lx << ", " << r.ly << "][" << r.hx << ", " << r.hy << "])";
}

namespace util {

    struct is_inside {
        is_inside(const Rect rect)
            :   m_rect(rect)
        {}

        bool operator()(const Point p) const {
            return (m_rect.lx <= p.x && m_rect.hx >= p.x) &&
                    (m_rect.ly <= p.y && m_rect.hy >= p.y);
        }

    private:
        Rect m_rect;
    };

    inline bool point_rank_less(const Point a, const Point b) {
        return a.rank < b.rank;
    }

    inline bool point_less_x(const Point a, const Point b) {
        return a.x < b.x;
    }

    inline bool point_less_y(const Point a, const Point b) {
        return a.y < b.y;
    }

    inline float extract_x(const Point& p) {
        return p.x;
    }

    inline float extract_y(const Point& p) {
        return p.y;
    }

    inline bool point_ne_x(const Point& a, const Point& b) {
        return a.x != b.x;
    }

    inline bool point_ne_y(const Point& a, const Point& b) {
        return a.y != b.y;
    }
}


#endif // UTIL_H
