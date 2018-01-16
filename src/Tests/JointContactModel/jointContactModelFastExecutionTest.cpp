#include "ceinms/JointContactFromOpensim.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/ContactModelXmlReader.h"
#include "ceinms/DataTable.h"
#include "ceinms/QueuesToIntersegmentalForces.h"
#include "ceinms/QueuesToJointReactionForces.h"
#include "ceinms/LoggerOnQueues.h"
#include "ceinms/concurrency/Queue.h"
#include "ceinms/concurrency/Latch.h"
#include "ceinms/FileUtils.h"
#include "ceinms/NMSmodel.h"
#include "ceinms/Activation/ExponentialActivation.h"
#include "ceinms/Tendon/ElasticTendon_BiSec.h"
#include "ceinms/Curve.h"
#include "ceinms/ArrayConverter.h"
#include "ceinms/GeneralisedCoordinatesData.h"
#include "ceinms/GeneralisedCoordinatesFromStorageFile.h"
#include "ceinms/ExternalForcesFromStorageFile.h"
#include "ceinms/QueueToStorageFile.h"
#include "ceinms/QueueToDataTable.h"
#include "ceinms/BatchOpenSimEvaluator.h"
#include <boost/filesystem.hpp>
using namespace ceinms;

#include <OpenSim/OpenSim.h>
using OpenSim::Storage;
using OpenSim::CoordinateActuator;
#include <OpenSim/Tools/InverseDynamicsTool.h>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>
using std::cout;
using std::endl;



class KriggGenericCF {

public:
    KriggGenericCF(const string& modelFilename, const string& kinematicsFilename, const string& externalLoadsXmlFilename);
    DataTable<SimTK::Vec3> getReactionForces(const DataTable<double>& muscleForces);
private:
    void calculateIntersegmentalForces();
    void calculateUnitaryContributionOfMuscleToContactForces();
    OpenSim::Model model_;
    int nForces_, nJoints_, nFrames_;
    vector<string> muscleNames_;
    BatchOpenSimEvaluator evaluator_;
    DataTable<SimTK::Vec3> intersegmentalForces_;
    std::map<string, DataTable<SimTK::Vec3>> unitaryContributionOfMuscleToContactForces_;

};

KriggGenericCF::KriggGenericCF(const string& modelFilename, const string& kinematicsFilename, const string& externalLoadsXmlFilename) :
    model_(modelFilename)
{
    nForces_ = model_.getForceSet().getSize();
    nJoints_ = model_.getJointSet().getSize();
    TrialData trialData;
    trialData.setOpenSimData(modelFilename, externalLoadsXmlFilename, kinematicsFilename);
    nFrames_ = trialData.getQ().getNRows();
    vector<TrialData> osimTrialData{ trialData };
    evaluator_.setOpenSimTrialData(osimTrialData);
    
    OpenSim::Array<string> muscleNames;
    model_.getForceSet().getNames(muscleNames);
    muscleNames_ = ArrayConverter::toStdVector(muscleNames);

    calculateIntersegmentalForces();
    calculateUnitaryContributionOfMuscleToContactForces();
}

void KriggGenericCF::calculateIntersegmentalForces() {

    //forces will be at 0 by default
    Result result(nForces_, nJoints_, nFrames_, nFrames_);
    vector<Result> results{ result };
    evaluator_.evaluate(results);
    intersegmentalForces_ = results[0].jointReactionForces;
}

void KriggGenericCF::calculateUnitaryContributionOfMuscleToContactForces() {

    Result result(nForces_, nJoints_, nFrames_, nFrames_);
    result.setMuscleNames(muscleNames_);
    vector<Result> results{ result };
    vector<double> empty(nForces_, 0.);
    for (int i(0); i < muscleNames_.size(); ++i) {
        vector<double> unitForce(empty);
        unitForce[i] = 1.;
        for (int r(0); r < nFrames_; ++r) {
            results[0].forces.setRow(r, unitForce);
        }
        evaluator_.evaluate(results);
        unitaryContributionOfMuscleToContactForces_.insert(
            std::make_pair(muscleNames_[i], results[0].jointReactionForces - intersegmentalForces_)
        );
    }
}


DataTable<SimTK::Vec3> KriggGenericCF::getReactionForces(const DataTable<double>& muscleForces) {

    DataTable<SimTK::Vec3> reactionForces = intersegmentalForces_;
    for (int i(0); i < muscleNames_.size(); ++i) {
        auto column(muscleForces.getColumn(i));
        auto& unitContribution(unitaryContributionOfMuscleToContactForces_[muscleNames_[i]]);
        for (int r(0); r < column.size(); ++r) {
            for (int c(0); c < unitContribution.getNColumns(); ++c) {
                unitContribution.at(r, c) *= column[i];
            }
        }
        reactionForces = reactionForces + unitContribution;
    }
    return reactionForces;
}

int main() {

    

    //NEED TO BE TESTED

    return 0;
}