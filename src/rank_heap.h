#ifndef RANKHEAP_H
#define RANKHEAP_H

#include "point_search.h"

#include <vector>
#include <algorithm>
#include <stdint.h>

class RankHeap
{
public:
    typedef std::vector<point_index> container_t;
    typedef container_t::iterator iterator;
    typedef container_t::const_iterator const_iterator;

    RankHeap() {}
    RankHeap(size_t capacity)
    {
        m_data.resize(capacity);
        m_begin = std::begin(m_data);
        m_end = m_begin;
    }

    size_t size() const {return std::distance(begin(), end());}

    bool full() const {return m_end == m_data.end();}

    point_index top() const {return *m_begin;}

    void reset(size_t capacity) {
        m_data.resize(capacity);
        m_begin = std::begin(m_data);
        m_end = m_begin;
    }

    void push(point_index index) throw() {
        if (m_end == m_data.end()) {
            if (index < top()) {
                std::pop_heap(m_begin, m_end);
                *(m_end-1) = index;
                std::push_heap(m_begin, m_end);
            }
        } else {
            *m_end++ = index;
            std::push_heap(m_begin, m_end);
        }
    }

    /**
     * Sort the range. This locks all existing items in-place and creates an smaller max-heap
     * with capacity-size() elements.
     */
    void sort() {
        std::sort_heap(m_begin, m_end);
        m_begin = m_end;
    }
    const_iterator begin() const {return std::begin(m_data);}
    const_iterator end() const {return m_end;}

    friend const_iterator begin(const RankHeap &heap) {return heap.begin();}
    friend const_iterator end(const RankHeap &heap) {return heap.end();}

private:
    container_t m_data;
    iterator m_begin;
    iterator m_end;
};

#endif // RANKHEAP_H
