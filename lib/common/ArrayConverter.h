//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_ArrayConverter_h
#define ceinms_ArrayConverter_h

#include <vector>

namespace ceinms {
    namespace ArrayConverter{
        
        template<typename T>
        void toStdVector(const OpenSim::Array<T>& srcArray, std::vector<T>& dstVector) {
            dstVector.clear();
            int size = srcArray.getSize();
            dstVector.resize(size);
            for (int i = 0; i < size; ++i)
                dstVector.at(i) = srcArray.get(i);
        }
        
        template<typename T>
        void fromStdVector(OpenSim::Array<T>& dstArray, const std::vector<T>& srcVector) {
            for (typename std::vector<T>::const_iterator it(srcVector.begin()); it != srcVector.end(); ++it)
                dstArray.append(*it);
        }
    }
}

#endif