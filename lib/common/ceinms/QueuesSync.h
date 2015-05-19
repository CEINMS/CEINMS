#ifndef ceinms_QueuesSync_h
#define ceinms_QueuesSync_h
#include <tuple>

namespace ceinms {
    namespace QueuesSync {

        template<typename T>
        typename T::type syncToTime(double t, T& queue) {

            typename T::type frame;
            do {
                frame = queue.pop();
            } while (frame.time < t);
            return frame;
        }

        template<typename A1, typename... Args>
        auto sync(A1& queue1, Args&... queues) -> decltype(std::make_tuple(queue1.pop(), queues.pop()...)) {

            auto frame1(queue1.pop());
            return std::make_tuple(frame1, syncToTime(frame1.time, queues)...);
        }
    }

}
#endif