#include "solution.h"

#include "util.h"
#include "timer.h"
#include "rank_heap.h"
#include "aligned_allocator.h"

#include <algorithm>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <cassert>


#ifdef _MSC_VER
// msvc does not have an expect compiler builtin
#define __builtin_expect(x, dummy) (x)

// msvc is missing this compiler intrinsic. technically this instruction does not exist,
// but it's convinient and may be added to the instruction set in the future.
#define _mm256_extract_epi32(mm, i) _mm_extract_epi32(_mm256_extractf128_si256(mm, i >= 4), i % 4)
#endif

const point_index AVX_COUNT = 1 << 11;

/**
 * Build an vector such that
 * result[lower_bound(from, value)    ] <= lower_bound(to, value)
 * result[lower_bound(from, value) + 1] <  lower_bound(to, value)
 * The resulting data structure is 2 items larger than from.size();
 */
std::vector<point_index> make_lower_cascading(const bin_search &from, const bin_search &to)
{
    std::vector<point_index> result;
    result.reserve(from.size() + 2);

    point_index from_index = 0;
    point_index to_index = 0;
    result.push_back(0);
    for(;from_index < (point_index)from.size(); from_index++) {
        to_index = to.lower_bound(from.values()[from_index], to_index, (point_index)to.size());
        result.push_back(to_index);
    }
    result.push_back((point_index)to.size());
    return result;
}

/**
 * Build an vector such that
 * result[upper_bound(from, value)    ] <= upper_bound(to, value)
 * result[upper_bound(from, value) + 1] <  upper_bound(to, value)
 * The resulting data structure is 2 items larger than from.size();
 */
std::vector<point_index> make_upper_cascading(const bin_search &from, const bin_search &to)
{
    std::vector<point_index> result;
    result.reserve(from.size() + 2);

    point_index from_index = 0;
    point_index to_index = 0;
    result.push_back(0);
    for(;from_index < (point_index)from.size(); from_index++) {
        to_index = to.upper_bound(from.values()[from_index], to_index, (point_index)to.size());
        result.push_back(to_index);
    }
    result.push_back((point_index)to.size());
    return result;
}

Solution::Solution(const Point *points_begin, const Point *points_end)
    :   m_points(points_begin, points_end)
{
    if (m_points.empty()) {
        return;
    }

    assert(m_points.size() >= AVX_COUNT);

    std::sort(begin(m_points), end(m_points), util::point_rank_less);

    // linear search data structure.
    m_x_coord.resize(AVX_COUNT);
    std::transform(begin(m_points), begin(m_points) + AVX_COUNT, begin(m_x_coord), util::extract_x);
    m_y_coord.resize(AVX_COUNT);
    std::transform(begin(m_points), begin(m_points) + AVX_COUNT, begin(m_y_coord), util::extract_y);

    // mipmap data structure
    // skip the indices already covered by linear search
    auto first = begin(m_points) + AVX_COUNT;
    auto pivot = first;
    auto last  = end(m_points);
    // this number is chosen in such a way that the last-level mipmap is as close to
    // the growth factor as possible.
    point_index size = 3050;
    while(pivot != last) {
        pivot = pivot + std::min(size, (point_index)std::distance(pivot, last));
        m_x_mipmaps.push_back(make_bin_search_from_x(first, pivot));
        m_y_mipmaps.push_back(make_bin_search_from_y(first, pivot));
        // for some reason a growth factor of 3 works better than 2.
        // and uses less memory
        size = size * 3;
        first = pivot;
    }

    // build cascading stuff. These accelate the binary searching. This works by creating
    // an mapping table that maps each index of mipmap n to an higher-level mipmap n+1.
    // since the mipmap level n+1 contains more and different elements than level n, we
    // still need to do an binary search to find the correct index at n+1. But we can
    // do that binary search over an much smaller range.
    for(int i = 0; i < (int)m_x_mipmaps.size() - 1; i++) {
        m_x_lower_cascading.push_back(make_lower_cascading(m_x_mipmaps[i], m_x_mipmaps[i+1]));
        m_x_upper_cascading.push_back(make_upper_cascading(m_x_mipmaps[i], m_x_mipmaps[i+1]));
        m_y_lower_cascading.push_back(make_lower_cascading(m_y_mipmaps[i], m_y_mipmaps[i+1]));
        m_y_upper_cascading.push_back(make_upper_cascading(m_y_mipmaps[i], m_y_mipmaps[i+1]));
    }
}

point_index Solution::search(const Rect rect, const point_index count, Point *out_points)
{
    if (count == 0) {
        return 0;
    }

    if (m_points.size() == 0) {
        return 0;
    }

    point_index avx_count = search_linear(rect, count, out_points);
    if (avx_count == count) {
        return avx_count;
    }

    point_index mipmap_count = search_mipmap(rect, count - avx_count, out_points + avx_count);
    return avx_count + mipmap_count;
}

point_index Solution::search_linear(const Rect rect, const point_index count, Point *out_points)
{
    static_assert((AVX_COUNT % 8) == 0, "AVX_COUNT must be a multiple of 8 for this function");

    __m256 rect_lx = _mm256_broadcast_ss(&rect.lx);
    __m256 rect_hx = _mm256_broadcast_ss(&rect.hx);
    __m256 rect_ly = _mm256_broadcast_ss(&rect.ly);
    __m256 rect_hy = _mm256_broadcast_ss(&rect.hy);

    point_index n = 0;
    for(point_index i = 0; i < AVX_COUNT; i+=8) {
        __m256 x = _mm256_load_ps(m_x_coord.data() + i);
        __m256 y = _mm256_load_ps(m_y_coord.data() + i);

        __m256 x_in = _mm256_and_ps(_mm256_cmp_ps(rect_lx, x, _CMP_LE_OQ),
                                    _mm256_cmp_ps(rect_hx, x, _CMP_GE_OQ));
        __m256 y_in = _mm256_and_ps(_mm256_cmp_ps(rect_ly, y, _CMP_LE_OQ),
                                    _mm256_cmp_ps(rect_hy, y, _CMP_GE_OQ));

        if (__builtin_expect(!_mm256_testz_ps(x_in, y_in), 0))
        {
            __m256i mask = _mm256_castps_si256(_mm256_and_ps(x_in, y_in));
            if (_mm256_extract_epi32(mask, 0)) {out_points[n++] = m_points[i+0]; if (n==count) return count;}
            if (_mm256_extract_epi32(mask, 1)) {out_points[n++] = m_points[i+1]; if (n==count) return count;}
            if (_mm256_extract_epi32(mask, 2)) {out_points[n++] = m_points[i+2]; if (n==count) return count;}
            if (_mm256_extract_epi32(mask, 3)) {out_points[n++] = m_points[i+3]; if (n==count) return count;}
            if (_mm256_extract_epi32(mask, 4)) {out_points[n++] = m_points[i+4]; if (n==count) return count;}
            if (_mm256_extract_epi32(mask, 5)) {out_points[n++] = m_points[i+5]; if (n==count) return count;}
            if (_mm256_extract_epi32(mask, 6)) {out_points[n++] = m_points[i+6]; if (n==count) return count;}
            if (_mm256_extract_epi32(mask, 7)) {out_points[n++] = m_points[i+7]; if (n==count) return count;}
        }
    }

    return (point_index)n;
}

void avx_search_single_bounds(const float* floats, const point_index* indices,
                              const float low_float, const float high_float,
                              const point_index count, RankHeap& heap)
{
    __m256 low = _mm256_broadcast_ss(&low_float);
    __m256 high = _mm256_broadcast_ss(&high_float);

    point_index i = 0;
    // align
    for(;(i < count) && ((uintptr_t)(floats+i) % sizeof(__m256)); i++)
    {
        if ((floats[i] >= low_float) && (floats[i] <= high_float)) {
            heap.push(indices[i]);
        }
    }

    const point_index sse_last = count - 7;
    for(;i < sse_last; i+=8)
    {
        __m256 a = _mm256_load_ps(floats + i);
        __m256 low_in = _mm256_cmp_ps(low,  a, _CMP_LE_OQ);
        __m256 high_in = _mm256_cmp_ps(high, a, _CMP_GE_OQ);

        if (__builtin_expect(!_mm256_testz_ps(low_in, high_in), 0))
        {
            __m256i mask = _mm256_castps_si256(_mm256_and_ps(low_in, high_in));
            if (_mm256_extract_epi32(mask, 0)) {heap.push(indices[i+0]);}
            if (_mm256_extract_epi32(mask, 1)) {heap.push(indices[i+1]);}
            if (_mm256_extract_epi32(mask, 2)) {heap.push(indices[i+2]);}
            if (_mm256_extract_epi32(mask, 3)) {heap.push(indices[i+3]);}
            if (_mm256_extract_epi32(mask, 4)) {heap.push(indices[i+4]);}
            if (_mm256_extract_epi32(mask, 5)) {heap.push(indices[i+5]);}
            if (_mm256_extract_epi32(mask, 6)) {heap.push(indices[i+6]);}
            if (_mm256_extract_epi32(mask, 7)) {heap.push(indices[i+7]);}
        }
    }

    for(;i < count; i++)
    {
        if ((floats[i] >= low_float) && (floats[i] <= high_float)) {
            heap.push(indices[i]);
        }
    }
}

point_index Solution::search_mipmap(const Rect &rect, point_index count, Point *out_points)
{
    m_heap.reset(count);

    point_index x_low, x_high;
    point_index y_low, y_high;

    size_t i;
    for(i = 0; i < m_x_mipmaps.size(); i++)
    {
        bin_search &x_mipmap = m_x_mipmaps[i];
        bin_search &y_mipmap = m_y_mipmaps[i];

        // the first level doesn't have cascading mapping tables.
        if (i != 0) {
            x_low  = x_mipmap.lower_bound(rect.lx, m_x_lower_cascading[i-1][x_low ], m_x_lower_cascading[i-1][x_low  + 1]);
            x_high = x_mipmap.upper_bound(rect.hx, m_x_upper_cascading[i-1][x_high], m_x_upper_cascading[i-1][x_high + 1]);

            y_low  = y_mipmap.lower_bound(rect.ly, m_y_lower_cascading[i-1][y_low ], m_y_lower_cascading[i-1][y_low  + 1]);
            y_high = y_mipmap.upper_bound(rect.hy, m_y_upper_cascading[i-1][y_high], m_y_upper_cascading[i-1][y_high + 1]);
        }
        else
        {
            x_low  = x_mipmap.lower_bound(rect.lx);
            x_high = x_mipmap.upper_bound(rect.hx, x_low, (point_index)x_mipmap.size());

            y_low  = y_mipmap.lower_bound(rect.ly);
            y_high = y_mipmap.upper_bound(rect.hy, y_low, (point_index)y_mipmap.size());
        }

        auto x_size = x_high - x_low;
        auto y_size = y_high - y_low;

        if (0 == x_size) continue;
        if (0 == y_size) continue;

        if ((x_size) < (y_size))
        {
            bin_search &mipmap = x_mipmap;
            point_index first = x_low;
            point_index last  = x_high;
            avx_search_single_bounds(mipmap.other_values() + first, mipmap.indices() + first,
                                     rect.ly, rect.hy, last - first, m_heap);
        }
        else
        {
            bin_search &mipmap = y_mipmap;
            point_index first = y_low;
            point_index last  = y_high;
            avx_search_single_bounds(mipmap.other_values() + first, mipmap.indices() + first,
                                     rect.lx, rect.hx, last - first, m_heap);
        }

        m_heap.sort();
        if (m_heap.full()) {
            break;
        }
    }

    for(point_index index : m_heap) {
        *out_points++ = m_points[index];
    }
    return (point_index)m_heap.size();
}
