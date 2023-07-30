#pragma once

#include <algorithm>
#include <vector>

template<typename T>
bool vector_equal(const std::vector<T>& lhs, const std::vector<T>& rhs) noexcept {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}
