#include "Result.h"

namespace ceinms {

    bool operator==(const Result& lhs, const Result& rhs) {
        return lhs.equals(rhs);
    }
}
