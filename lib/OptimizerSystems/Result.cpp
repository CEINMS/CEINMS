#include "Result.h"

namespace CEINMS {

    bool operator==(const Result& lhs, const Result& rhs) {
        return lhs.equals(rhs);
    }

}
