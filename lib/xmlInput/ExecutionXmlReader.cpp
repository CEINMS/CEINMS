//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//


#include "ExecutionXmlReader.h"
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;

ExecutionXmlReader::ExecutionXmlReader(const string& filename)
:runMode_(0) {

	try {
        std::auto_ptr<ExecutionType> executionPointer(execution(filename));
		executionPointer_ = executionPointer;
    }  
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
    readXml();
}

void ExecutionXmlReader::readXml() {
	
     try {
        ExecutionType::NMSmodel_type& myModel(executionPointer_->NMSmodel());
        ExecutionType::NMSmodel_type::activation_type& myActivation(myModel.activation());
        ExecutionType::NMSmodel_type::activation_type::exponential_optional& myExpOpt(myActivation.exponential());
        ExecutionType::NMSmodel_type::activation_type::piecewise_optional& myPWOpt(myActivation.piecewise());
 
        if(myExpOpt.present())
            runMode_ += NMSModelCfg::ExponentialActivation;
        else if(myPWOpt.present())
            runMode_ += NMSModelCfg::PiecewiseActivation;
        else {
            cout << "invalid XML: ActivationType not found\n";
            exit(EXIT_FAILURE);
        }
        
        ExecutionType::NMSmodel_type::tendon_type& myTendon(myModel.tendon());
        ExecutionType::NMSmodel_type::tendon_type::stiff_optional& myStiffOpt(myTendon.stiff());
        ExecutionType::NMSmodel_type::tendon_type::elastic_optional& myElsaticOpt(myTendon.elastic());
        ExecutionType::NMSmodel_type::tendon_type::elasticBiSec_optional& myElsaticBiSecOpt(myTendon.elasticBiSec());
        
        if(myStiffOpt.present())
            runMode_ += NMSModelCfg::StiffTendon;
        else if (myElsaticOpt.present())
            runMode_ += NMSModelCfg::ElasticTendon;
        else if (myElsaticBiSecOpt.present())
            runMode_ += NMSModelCfg::ElasticTendonBiSec;
        else {
            cout << "invalid XML: TendonType not found\n";
            exit(EXIT_FAILURE);
        }
            
        ExecutionType::NMSmodel_type::type_type& myType(myModel.type());
        ExecutionType::NMSmodel_type::type_type::hybrid_optional& myHybOpt(myType.hybrid());
        ExecutionType::NMSmodel_type::type_type::openLoop_optional& myOLOpt(myType.openLoop());
        if(myHybOpt.present())
            runMode_ += NMSModelCfg::Hybrid;
        else if(myOLOpt.present())
            runMode_ += NMSModelCfg::OpenLoop;
        else {
            cout << "invalid XML: Hybrid/OpenLoop Type not found\n";
            exit(EXIT_FAILURE);
        }
        
        ExecutionType::online_optional& myOnlineOpt(executionPointer_->online());
        ExecutionType::offline_optional& myOfflineOpt(executionPointer_->offline());
        if(myOnlineOpt.present())
            runMode_ += NMSModelCfg::Online;
        else if (myOfflineOpt.present())
            runMode_ += NMSModelCfg::Offline;
        else {
            cout << "invalid XML: Online/Offline Type not found\n";
            exit(EXIT_FAILURE);
        }
        
 
 //ANCORA DA IMPLEMENTARE
 /*       
        ExecutionType::samplingFrequency_optional& myFreqOpt(executionPointer->samplingFrequency());
        if(myFreqOpt.present())
            myExecutionCfg.frequency_ = ExecutionCfg::Enabled;
        else
            myExecutionCfg.frequency_ = ExecutionCfg::Disabled;
            
        ExecutionType::logging_type::txt_optional& myTxtOpt(executionPointer->logging().txt()); 
        ExecutionType::logging_type::csv_optional& myCsvOpt(executionPointer->logging().csv());
        ExecutionType::logging_type::mot_optional& myMotOpt(executionPointer->logging().mot());
        if(myTxtOpt.present())
            myExecutionCfg.logging_ = ExecutionCfg::txt;
        else if(myCsvOpt.present())
            myExecutionCfg.logging_ = ExecutionCfg::csv;
        else if(myMotOpt.present())
            myExecutionCfg.logging_ = ExecutionCfg::mot;
        else {
            cout << "invalid XML: Logging Type not found\n";
            exit(EXIT_FAILURE);
        }
*/        
    }  
    catch (const xml_schema::exception& e) {
        cout << e << endl;
        exit(EXIT_FAILURE);
    }
    
    
}


void ExecutionXmlReader::getMusclesToPredict(std::vector<std::string>& musclesToPredict) {
    
    ExecutionType::NMSmodel_type& myModel(executionPointer_->NMSmodel());
    ExecutionType::NMSmodel_type::type_type& myType(myModel.type());
    ExecutionType::NMSmodel_type::type_type::hybrid_optional& myHybOpt(myType.hybrid());
    if(!myHybOpt.present()) {
        cout << "Cannot get hybrid parameters, hybrid option not selected\n";
        exit(EXIT_FAILURE);
    }


    musclesToPredict.clear();
    HybridType::predictedMuscles_type& predictedMuscles(myType.hybrid()->predictedMuscles());
    for(unsigned mi = 0; mi < predictedMuscles.size(); ++mi)
        musclesToPredict.push_back(predictedMuscles.at(mi));
}


void ExecutionXmlReader::getMusclesToTrack(std::vector<std::string>& musclesToTrack) {
    
    ExecutionType::NMSmodel_type& myModel(executionPointer_->NMSmodel());
    ExecutionType::NMSmodel_type::type_type& myType(myModel.type());
    ExecutionType::NMSmodel_type::type_type::hybrid_optional& myHybOpt(myType.hybrid());
    if(!myHybOpt.present()) {
        cout << "Cannot get hybrid parameters, hybrid option not selected\n";
        exit(EXIT_FAILURE);
    }


    musclesToTrack.clear();
    HybridType::trackedMuscles_type& trackedMuscles(myType.hybrid()->trackedMuscles());
    for(unsigned mi = 0; mi < trackedMuscles.size(); ++mi)
        musclesToTrack.push_back(trackedMuscles.at(mi));
}

void ExecutionXmlReader::getHybridWeightings(double& alpha, double& beta, double& gamma) {

    ExecutionType::NMSmodel_type& myModel(executionPointer_->NMSmodel());
    ExecutionType::NMSmodel_type::type_type& myType(myModel.type());
    ExecutionType::NMSmodel_type::type_type::hybrid_optional& myHybOpt(myType.hybrid());
    if(!myHybOpt.present()) {
        cout << "Cannot get hybrid parameters, hybrid option not selected\n";
        exit(EXIT_FAILURE);
    }

    HybridType::alpha_type& myAlpha(myType.hybrid()->alpha());
    alpha = myAlpha;
    
    HybridType::beta_type& myBeta(myType.hybrid()->beta());
    beta = myBeta;

    HybridType::gamma_type& myGamma(myType.hybrid()->gamma());
    gamma = myGamma;
}


void ExecutionXmlReader::getAnnealingParameters(unsigned& nt, unsigned& ns, double& rt, double& t, unsigned& maxNoEval, double& epsilon, unsigned& noEpsilon) const {
	    
	ExecutionType::NMSmodel_type& myModel(executionPointer_->NMSmodel());
    ExecutionType::NMSmodel_type::type_type& myType(myModel.type());
    ExecutionType::NMSmodel_type::type_type::hybrid_optional& myHybOpt(myType.hybrid());
    if(!myHybOpt.present()) {
        cout << "Cannot get Simulated Annealing parameters, hybrid option not selected\n";
        exit(EXIT_FAILURE);
    }

	ExecutionType::NMSmodel_type::type_type::hybrid_type::algorithm_type& myAlgorithmType(myType.hybrid()->algorithm());
	ExecutionType::NMSmodel_type::type_type::hybrid_type::algorithm_type::simulatedAnnealing_type& mySimanType(myAlgorithmType.simulatedAnnealing());
	nt = mySimanType.NT();
	ns = mySimanType.NS();
	rt = mySimanType.rt();
	t  = mySimanType.T();
	maxNoEval = mySimanType.maxNoEval();
	epsilon = mySimanType.epsilon();
	noEpsilon = mySimanType.noEpsilon();
}


NMSModelCfg::RunMode ExecutionXmlReader::getRunMode() const {
    
    return static_cast<NMSModelCfg::RunMode>(runMode_);

}
