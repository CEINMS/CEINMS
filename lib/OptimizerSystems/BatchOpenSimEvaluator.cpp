#include "ceinms/BatchOpenSimEvaluator.h"
using std::string;
using std::vector;
#include "ceinms/Mapper.h"
#include "ceinms/ArrayConverter.h"

namespace ceinms {

    BatchOpenSimEvaluator::BatchOpenSimEvaluator() {}

    BatchOpenSimEvaluator::BatchOpenSimEvaluator(
        const std::vector<TrialData>& osimTrialData) {
        setOpenSimTrialData(osimTrialData);
    }

    void BatchOpenSimEvaluator::setOpenSimTrialData(const std::vector<TrialData>& osimTrialData) {

        osimTrialData_ = osimTrialData;
        for (auto& e : osimTrialData) {
            std::cout << e.getExternalLoadFile() << std::endl << std::flush;
            std::cout << e.getOpenSimModelFile() << std::endl << std::flush;
            jointContactForcesFromOpenSim_.emplace_back(new JointContactFromOpensim(e.getOpenSimModelFile(), e.getExternalLoadFile()));
        }
    }

    void BatchOpenSimEvaluator::evaluate(const std::vector<Result>& resultsFromCeinms) {
        
        openSimResults_ = resultsFromCeinms;
       
        for (int i(0); i < resultsFromCeinms.size(); ++i) {
            openSimResults_.at(i).jointReactionForces = evaluate(jointContactForcesFromOpenSim_.at(i), resultsFromCeinms.at(i), osimTrialData_.at(i));
        }
    }

    DataTable<SimTK::Vec3> BatchOpenSimEvaluator::evaluate(std::unique_ptr<JointContactFromOpensim>& jointContactModel, const Result& resultFromCeinms, const TrialData& osimTrialData) {
        //map muscles and torques from resultsFromCeinms to a mapper.. so it's possible to put them in the correct order
       
        vector<string> inputMuscleNames(resultFromCeinms.forces.getLabels());
       vector<string> outputMuslceNames;
       jointContactModel->getMuscleNamesFromOsimModel(outputMuslceNames);
       
         Mapper muscleMapper(inputMuscleNames, outputMuslceNames);
       
       vector<string> inputDofNames(resultFromCeinms.torques.getLabels());
       vector<string> outputDofNames;
       jointContactModel->getCoordinateNamesFromOsimModel(outputDofNames);
       Mapper dofMapper(inputDofNames, outputDofNames);
       
       DataTable<SimTK::Vec3> jointReactionForces;
       std::size_t nTimeSteps(resultFromCeinms.forces.getNRows());
        for (std::size_t iRow(0); iRow < nTimeSteps; ++iRow) {
       
            auto time(resultFromCeinms.forces.getTime(iRow));
            auto muscleForces(muscleMapper.map(resultFromCeinms.forces.getRow(iRow)));
            auto predictedTorques(dofMapper.map(resultFromCeinms.torques.getRow(iRow)));
            auto q(osimTrialData.getQ().getRow(iRow));
            auto qd(osimTrialData.getQd().getRow(iRow));
            auto qdd(osimTrialData.getQdd().getRow(iRow));
            jointContactModel->setTime(time);
            jointContactModel->setQ(q);
            jointContactModel->setQd(qd);
            jointContactModel->setQdd(qdd);
            jointContactModel->calculateJointReactionForces(ArrayConverter::toSimTKVector(muscleForces), ArrayConverter::toSimTKVector(predictedTorques));
            SimTK::Vector_<SimTK::Vec3> reactionForces;
            SimTK::Vector_<SimTK::Vec3> reactionTorques;
            jointContactModel->getAllReactionForces(reactionForces, reactionTorques);
            jointReactionForces.pushRow(time, ArrayConverter::fromSimTKVector(reactionForces));
       
        }
         std::vector<std::string> jointNames;
         jointContactModel->getJointNamesFromOsimModel(jointNames);
         jointReactionForces.setLabels(jointNames);
        return jointReactionForces;
    }


    std::vector<Result> BatchOpenSimEvaluator::getResults() const {
        return openSimResults_;
    }


}