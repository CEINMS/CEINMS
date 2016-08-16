//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#ifndef ModelEvaluationOfflineStiffness_h
#define ModelEvaluationOfflineStiffness_h

#include <vector>
#include <list>
#include <string>
#include "ModelEvaluationBase.h"

namespace ceinms {
    template <typename NMSmodelT, typename Logger>
    class ModelEvaluationOfflineStiffness : public ModelEvaluationBase<Logger> {

    public:
        ModelEvaluationOfflineStiffness() = delete;
        ModelEvaluationOfflineStiffness(ceinms::InputConnectors& inputConnectors, ceinms::OutputConnectors& outputConnectors, NMSmodelT& subject, const std::vector< std::string >& valuesToLog);
        ~ModelEvaluationOfflineStiffness();

        void setOutputDirectory(const std::string& outputDir);
        void operator()();

    private:
        void readDataFromQueues();
        void initOfflineCurve();
        NMSmodelT& subject_;
        std::string outputDir_;
        std::vector< std::string > dofNames_;
        std::vector< std::string > dofNamesWithExtTorque_;

        std::list< ceinms::InputConnectors::FrameType > lmtDataFromQueue_, emgDataFromQueue_, externalTorquesDataFromQueue_;
        std::list< std::vector < ceinms::InputConnectors::FrameType > > maDataFromQueue_, dMaDataFromQueue_;
        unsigned noDof_;
        double globalEmDelay_;
    };
}

#include "ModelEvaluationOfflineStiffness.cpp"

#endif
