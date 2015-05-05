//__________________________________________________________________________
// Author(s): Monica Reggiani - June 2014
// email:  monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_QueueData_h
#define ceinms_QueueData_h

namespace ceinms {
    template <typename T>
    struct QueueData {
        double time;
        T data;
    };
}
#endif