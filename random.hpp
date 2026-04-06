#ifndef URANDOM_RANDOM_HPP
#define URANDOM_RANDOM_HPP

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

namespace urandom {

namespace detail {

inline void secure_bytes(void* buf, size_t n) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) {
        perror("urandom open failed");
        abort();
    }
    size_t r = fread(buf, 1, n, f);
    fclose(f);
    if (r != n) abort();
}

inline uint64_t secure_u64() {
    uint64_t a,b,c,d;
    secure_bytes(&a,8);
    secure_bytes(&b,8);
    secure_bytes(&c,8);
    secure_bytes(&d,8);
    a ^= b << 13;
    b ^= c >> 9;
    c ^= d << 7;
    d ^= a >> 17;
    return a ^ b ^ c ^ d;
}

}

template<typename T>
T random(T min, T max) {
    if constexpr (std::is_integral_v<T>) {
        if (min > max) std::swap(min, max);
        uint64_t range = (uint64_t)max - (uint64_t)min + 1;
        uint64_t limit = UINT64_MAX - UINT64_MAX % range;
        uint64_t r;
        do {
            r = detail::secure_u64();
        } while(r >= limit);
        return (T)((uint64_t)min + r % range);
    } else if constexpr (std::is_floating_point_v<T>) {
        double t = (double)detail::secure_u64() / (double)UINT64_MAX;
        return (T)(min + (max - min) * t);
    } else {
        throw std::invalid_argument("unsupported type");
    }
}

inline std::string random_string(size_t len, const std::string& pool) {
    if (pool.empty()) throw std::invalid_argument("empty pool");
    std::string s;
    s.reserve(len);
    size_t n = pool.size();
    for (size_t i=0; i<len; ++i) {
        s += pool[random<size_t>(0, n-1)];
    }
    return s;
}

template<size_t N>
std::string random_string(size_t len, const char (&arr)[N]) {
    return random_string(len, std::string(arr, N));
}

inline std::string random_string(size_t len, const std::vector<std::string>& pools) {
    std::string combined;
    for (const auto& p : pools) combined += p;
    return random_string(len, combined);
}

}

#endif
