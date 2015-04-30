#ifndef Ceinms_Result_h
#define Ceinms_Result_h

#include "DataTable.h"

namespace CEINMS {
    class Result{
    public:

        DataTable<double> forces, torques, activations, penalties;
        Result(size_t nMTUs, size_t nDofs, size_t nEmgs, size_t nLmtMa) :
            forces(nLmtMa, nMTUs),
            torques(nLmtMa, nDofs),
            activations(nEmgs, nMTUs),
            penalties(nLmtMa, nMTUs) {}
        bool equals(const Result& rhs) const {
            return (forces == rhs.forces      &&
                torques == rhs.torques     &&
                activations == rhs.activations &&
                penalties == rhs.penalties);

        }

    };

}
#endif