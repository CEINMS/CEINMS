//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ceinms_ModelEvaluationOffline_h
#define ceinms_ModelEvaluationOffline_h

#include <vector>
#include <list>
#include <string>
#include "ModelEvaluationBase.h"

namespace ceinms {

    template <typename NMSmodelT, typename Logger>
    class ModelEvaluationOffline : public ModelEvaluationBase < Logger > {

    public:
        ModelEvaluationOffline() = delete;
        ModelEvaluationOffline(InputConnectors& inputConnectors, OutputConnectors& outputConnectors, NMSmodelT& subject, const std::vector< std::string >& valuesToLog);
        ~ModelEvaluationOffline();

        void setOutputDirectory(const std::string& outputDir);
        void operator()();

    private:
        void readDataFromQueues();
        void initOfflineCurve();
        NMSmodelT& subject_;
        std::string outputDir_;
        std::vector< std::string > dofNames_;
        std::vector< std::string > dofNamesWithExtTorque_;

        std::list< InputConnectors::FrameType > lmtDataFromQueue_, emgDataFromQueue_, externalTorquesDataFromQueue_;
        std::list< std::vector < InputConnectors::FrameType > > maDataFromQueue_;
        unsigned noDof_;
        double globalEmDelay_;
    };
}

#include "ModelEvaluationOffline.cpp"
#endif
