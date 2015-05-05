//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - June 2014
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#ifndef ceinms_ExternalTorquesFromX_h
#define ceinms_ExternalTorquesFromX_h

#include <vector>
#include <string>
#include "InputConnectors.h"

namespace ceinms {
    class ExternalTorquesFromX
    {
    public:
        template <typename NMSModelT>
        ExternalTorquesFromX(InputConnectors& inputConnectors, const NMSModelT& subject);
        virtual ~ExternalTorquesFromX();
        virtual void operator()() {};
        void updateExternalTorques(const std::vector<double>& currentExternalTorquesData, double currentTime);
    protected:
        std::vector< std::string > dofNames_;
        InputConnectors& inputConnectors_;
    };


    template <typename NMSModelT>
    ExternalTorquesFromX::ExternalTorquesFromX(InputConnectors& inputConnectors, const NMSModelT& subject) :inputConnectors_(inputConnectors)
    {
        subject.getDoFNames(dofNames_);
    }
}

#endif
