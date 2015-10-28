#include <QCoreApplication>
#include <QVariant>

extern "C" {
	void resetStartTime(std::chrono::nanoseconds timestamp);
	std::chrono::nanoseconds getStartTime(std::chrono::nanoseconds timestamp, bool allowReset = true);
}

namespace qtmir {

template<typename T>
T compressTimestamp(std::chrono::nanoseconds timestamp)
{
    std::chrono::nanoseconds startTime = getStartTime(timestamp);
    std::chrono::nanoseconds result = (timestamp - startTime) / 1000000; // ms in nanosecond type to compare overflow

    if (std::numeric_limits<std::chrono::nanoseconds::rep>::max() > std::numeric_limits<T>::max() &&
        result > std::chrono::nanoseconds(std::numeric_limits<T>::max())) {
        // we've overflowed the boundaries of the millisecond type.
        resetStartTime(timestamp);
        return 0;
    }

    return result.count();
}

template<typename T>
std::chrono::nanoseconds uncompressTimestamp(T timestamp)
{
    auto tsNS = std::chrono::milliseconds(timestamp);
    return getStartTime(tsNS, false) + std::chrono::nanoseconds(tsNS);
}

}
