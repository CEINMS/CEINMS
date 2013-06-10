
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;

#include <stdlib.h>

namespace Hybrid {
   
    //TODO:: ricordarsi di far iniziare i muscoli da predirre con il valore al tempo t1
    template<typename NMSmodelT, typename StaticComputationMode>
    StaticComputation<NMSmodelT, StaticComputationMode>::StaticComputation(NMSmodelT& subject, 
                                                                           const vector<string>& muscleNamesWithEMGtoTrack, 
                                                                           const vector<string>& muscleNamesWithEMGtoPredict): 
    subject_(subject), computationMode_(subject) {
        
        if(!subject_.haveTheseMuscles(muscleNamesWithEMGtoTrack) || !subject_.haveTheseMuscles(muscleNamesWithEMGtoPredict)) {
            std::cout << "Error in RecursiveEMGs_Hybrid: muscle name not found\n";
            exit(EXIT_FAILURE);
        }   
        
        subject_.getMusclesIndexFromMusclesList(muscleIndexWithEMGtoTrack_, muscleNamesWithEMGtoTrack); 
        subject_.getMusclesIndexFromMusclesList(muscleIndexWithEMGtoPredict_, muscleNamesWithEMGtoPredict); 

        //concatenate muscleIndexWithEMGtoTrack_ and muscleIndexWithEMGtoPredict_
        muscleIndexWithEMGtoOptimize_.assign(muscleIndexWithEMGtoTrack_.begin(), muscleIndexWithEMGtoTrack_.end());
        muscleIndexWithEMGtoOptimize_.insert( muscleIndexWithEMGtoOptimize_.end(), muscleIndexWithEMGtoPredict_.begin(), muscleIndexWithEMGtoPredict_.end() );    
    
        vector<double> currentEMGData;
        subject_.getEmgs(currentEMGData);
        
        for(unsigned i = 0; i < muscleIndexWithEMGtoTrack_.size(); ++i)
            initialValueOfTrackedEMGs_.push_back(currentEMGData.at(muscleIndexWithEMGtoTrack_.at(i)));
    }


    template<typename NMSmodelT, typename StaticComputationMode>
    void StaticComputation<NMSmodelT, StaticComputationMode>::setExternalTorques(const vector<double>& externalTorques, const vector<string>& dofNames) {
        
        vector<string> dofNamesFromSubject;
        subject_.getDoFNames(dofNamesFromSubject);
        if(dofNamesFromSubject != dofNames)
        {
            cout << "StaticComputation: dofNames from the subject and external torques dof names are different\n";
            exit(EXIT_FAILURE);
        }
        
        externalTorques_ = externalTorques;
    }


    template<typename NMSmodelT, typename StaticComputationMode>
    void StaticComputation<NMSmodelT, StaticComputationMode>::getExternalTorques(vector<double>& externalTorques) const {
        
        externalTorques = externalTorques_;
    }


    template<typename NMSmodelT, typename StaticComputationMode>
    void StaticComputation<NMSmodelT, StaticComputationMode>::getTorques(vector<double>& torques) {
        
        computationMode_.getTorques(torques);
    }


    template<typename NMSmodelT, typename StaticComputationMode>
    void StaticComputation<NMSmodelT, StaticComputationMode>::getCurrentEMGs(vector<double>& emgValues) const {
        
        subject_.getEmgs(emgValues);
    }

    
    template<typename NMSmodelT, typename StaticComputationMode>
    void StaticComputation<NMSmodelT, StaticComputationMode>::getInitialValuesOfTrackedEMGs(vector<double>& emgValues) const {
        
        emgValues.assign(initialValueOfTrackedEMGs_.begin(), initialValueOfTrackedEMGs_.end());
    }
    
    
     
    template<typename NMSmodelT, typename StaticComputationMode>
    void StaticComputation<NMSmodelT, StaticComputationMode>::getAdjustedValuesOfTrackedEMGs(vector<double>& emgValues) const {
        
        emgValues.resize(muscleIndexWithEMGtoTrack_.size());
        vector<double> currentEMGData;
        subject_.getEmgs(currentEMGData);
        
        for(unsigned i = 0; i < muscleIndexWithEMGtoTrack_.size(); ++i)
            emgValues.at(i) = currentEMGData.at(muscleIndexWithEMGtoTrack_.at(i));
        
    }
    
    
    
}


















