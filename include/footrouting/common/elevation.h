#pragma once

#include <cstdint>

namespace footrouting {

using elevation_t = int16_t;
using elevation_diff_t = int16_t;

constexpr elevation_t NO_ELEVATION_DATA = -32767;

}  // namespace footrouting
