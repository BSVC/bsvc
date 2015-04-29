//
// Provides access to the system's time, in seconds.
//

#ifndef FRAMEWORK_TIME_HPP_
#define FRAMEWORK_TIME_HPP_

#include <ctime>

namespace Time {
// Returns the current system time in seconds.
std::time_t seconds() { return std::time(nullptr); }
}

#endif  // FRAMEWORK_TIME_HPP_
