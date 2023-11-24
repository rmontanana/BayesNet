#ifndef TIMER_H
#define TIMER_H
#include <chrono>
#include <string>
#include <sstream>

namespace platform {
    class Timer {
    private:
        std::chrono::high_resolution_clock::time_point begin;
        std::chrono::high_resolution_clock::time_point end;
    public:
        Timer() = default;
        ~Timer() = default;
        void start() { begin = std::chrono::high_resolution_clock::now(); }
        void stop() { end = std::chrono::high_resolution_clock::now(); }
        double getDuration()
        {
            stop();
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double >> (end - begin);
            return time_span.count();
        }
        std::string getDurationString()
        {
            double duration = getDuration();
            double durationShow = duration > 3600 ? duration / 3600 : duration > 60 ? duration / 60 : duration;
            std::string durationUnit = duration > 3600 ? "h" : duration > 60 ? "m" : "s";
            std::stringstream ss;
            ss << std::setw(7) << std::setprecision(2) << std::fixed << durationShow << " " << durationUnit << " ";
            return ss.str();
        }
    };
} /* namespace platform */
#endif /* TIMER_H */