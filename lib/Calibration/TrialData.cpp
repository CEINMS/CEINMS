//__________________________________________________________________________
// Author(s): Claudio Pizzolato, Monica Reggiani - September 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//         monica.reggiani@gmail.com
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <stdlib.h>
#include <math.h>



void TrialData::crop(const double from, const double to) {

    if(noLmtSteps_ != noTorqueSteps_) {
        cout << "ERROR in cropping trial\n";
        exit(EXIT_FAILURE);
    }
    
    if(to <= from || from < 0 || from > 1 || to < 0 || to > 1){
        cout << "cropping failure\n";
        return;
    }
    
    cout << "CROPPING TRIAL:\n";
    cout << "noStep: " << noLmtSteps_ << endl;
    cout << "noLmtData (rows): " << lmtData_.size() << endl;
    cout << "noLmtData (cols): " << lmtData_.at(0).size() << endl;
    
    unsigned startSample = static_cast<unsigned>(ceil(from*noLmtSteps_));
    unsigned stopSample  = static_cast<unsigned>(ceil(to*noLmtSteps_));
    
    cout << "from/to: " << startSample << "/" << stopSample << endl;
    
    //crop lmt data
    {
    vector< vector<double> >::const_iterator cropStart = lmtData_.begin() + startSample;
    vector< vector<double> >::const_iterator cropEnd   = lmtData_.begin() + stopSample;
    vector< vector<double> > temp(cropStart, cropEnd);
    lmtData_ = temp;      
    }
    
    {
    vector<double>::const_iterator cropStart = lmtTimeSteps_.begin() + startSample;
    vector<double>::const_iterator cropEnd   = lmtTimeSteps_.begin() + stopSample;
    vector<double> temp(cropStart, cropEnd);
    lmtTimeSteps_ = temp;
    }
    
    noLmtSteps_ = lmtTimeSteps_.size();
    
    cout << "noLmtSteps: " << noLmtSteps_ << endl;
    cout << "noLmtData: " << lmtData_.size() << endl;
    
    //crop torque data
    for(unsigned dofCt = 0; dofCt < noDoF_; ++dofCt) {
        vector<double>::const_iterator cropStart = torqueData_.at(dofCt).begin() + startSample;
        vector<double>::const_iterator cropEnd   = torqueData_.at(dofCt).begin() + stopSample;
        vector<double> temp(cropStart, cropEnd);
        torqueData_.at(dofCt) = temp;
    }
    noTorqueSteps_= torqueData_.at(0).size();
    
    
    cout << "noTorqueSteps: " << noTorqueSteps_ << endl;
    cout << "torqueData" << endl;
    //crop ma data
    for(unsigned dofCt = 0; dofCt < noDoF_; ++dofCt) {
            vector< vector<double> >::const_iterator cropStart = maData_.at(dofCt).begin() + startSample;
            vector< vector<double> >::const_iterator cropEnd   = maData_.at(dofCt).begin() + stopSample;
            vector< vector<double> > temp(cropStart, cropEnd);
            maData_.at(dofCt) = temp;
    }
    
}
