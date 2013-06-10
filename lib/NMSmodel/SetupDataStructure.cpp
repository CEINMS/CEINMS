// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com

#include <iostream>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <string>
using std::string;

#include <stdio.h>
#include <fstream>

#include "NMSmodel.h"
#include "MTU.h"
#include "DoF.h"
#include "Curve.h"
//#include "SetupDataStructure.h"

template<typename NMSmodelT>
SetupDataStructure<NMSmodelT>::SetupDataStructure(const string& configurationFile)
:subjectPointer(subject(configurationFile.c_str()))  {  }


template<typename NMSmodelT>
void SetupDataStructure<NMSmodelT>::createCurves() {
    
    NMSmodelType::muscleDefault_type& muscleDefault(subjectPointer->muscleDefault());
    MuscleDefaultType::Curve_sequence& curveSequence( muscleDefault.Curve());
    for ( MuscleDefaultType::Curve_iterator i = curveSequence.begin(); i != curveSequence.end(); ++i) {
        // each i is a curve
        string curveName = (*i).name();
        cout << curveName << endl;
        vector<double> x;
    
        PointsSequenceType xPoints =(*i).xPoints();
        PointsSequenceType::iterator pointsIt;
    
        for (pointsIt = xPoints.begin(); pointsIt != xPoints.end(); ++pointsIt) {
            double currentX = (*pointsIt); 
            x.push_back(currentX); 
        }
        
        vector<double> y;
        
        PointsSequenceType yPoints =(*i).yPoints();
    
        for (pointsIt = yPoints.begin(); pointsIt != yPoints.end(); ++pointsIt) {
            double currentY = (*pointsIt); 
            y.push_back(currentY); 
        }
        
    //     for (int i = 0; i < x.size(); ++i)
    //       cout << x[i] << " " << y[i] << endl;
    
        if (curveName == "activeForceLength")  activeForceLengthCurve_.resetPointsWith(x,y);
        if (curveName == "passiveForceLength") passiveForceLengthCurve_.resetPointsWith(x,y);
        if (curveName == "forceVelocity")      forceVelocityCurve_.resetPointsWith(x,y);
    }    
  
    cout << "activeForceLength" << endl <<  activeForceLengthCurve_ << endl;
    cout << "passiveForceLength" << endl <<  passiveForceLengthCurve_ << endl;
    cout <<  "forceVelocity" << endl <<  forceVelocityCurve_ << endl;
}   


template<typename NMSmodelT>
void SetupDataStructure<NMSmodelT>::createMuscles(NMSmodelT& mySubject) {
    

    
    NMSmodelType::muscleDefault_type& muscleDefault(subjectPointer->muscleDefault());
    NMSmodelType::muscles_type& muscles(subjectPointer->muscles());
    MusclesType::muscle_sequence& muscleSequence( muscles.muscle() );
    for (MusclesType::muscle_iterator i(muscleSequence.begin()); i != muscleSequence.end(); ++i) {
        string muscleName = (*i).name();
        MTUT newMuscle(muscleName);
        double c1 = (*i).C1();
        double c2 = (*i).C2();
        double shapeFactor = (*i).shapeFactor();
        double optimalFiberLength = (*i).optimalFiberLength();
        double pennationAngle = (*i).pennationAngle();
        double tendonSlackLength = (*i).tendonSlackLength();
        double percentageChange = muscleDefault.percentageChange();
        double damping = muscleDefault.damping();
        double maxIsometricForce = (*i).maxIsometricForce();
        double strengthCoefficient = (*i).strengthCoefficient();
        newMuscle.setParametersToComputeActivation(c1, c2, shapeFactor);
        newMuscle.setCurves(activeForceLengthCurve_, passiveForceLengthCurve_, forceVelocityCurve_);
        newMuscle.setParametersToComputeForces(optimalFiberLength, pennationAngle, 
                tendonSlackLength, percentageChange, damping, maxIsometricForce, strengthCoefficient);
        mySubject.addMuscle(newMuscle);
    } 
}


template<typename NMSmodelT>
void SetupDataStructure<NMSmodelT>::createDoFs(NMSmodelT& mySubject) {

    NMSmodelType::DoFs_type& dofs(subjectPointer->DoFs());
    DoFsType::DoF_sequence& dofSequence( dofs.DoF());
    DoFsType::DoF_iterator i;
    for (i = dofSequence.begin(); i != dofSequence.end(); ++i) {
    
        string dofName = (*i).name();
        DoFT newDoF(dofName);
        
        // Now have a look at the muscles we have
        MuscleSequenceType currentSequence =(*i).muscleSequence();
        MuscleSequenceType::iterator muscleIt;
    
        for (muscleIt = currentSequence.begin(); muscleIt != currentSequence.end(); ++muscleIt) {
            string currentMuscle(*muscleIt);
             
            vectorMTUitrT found; 
            if (!mySubject.haveThisMuscle(currentMuscle, found)) {
                cout << currentMuscle << " not configured. Sorry, we have to exit!\n";
                exit(EXIT_FAILURE);
            }
            else 
                newDoF.addNewMuscle(found);
        }   
 
        mySubject.addDoF(newDoF);   
    }      
} 
 

template<typename NMSmodelT>
void SetupDataStructure<NMSmodelT>::writeXMLCalibratedFile(NMSmodelT& mySubject, const string& XMLfilename) {
    
    NMSmodelType::muscleDefault_type& muscleDefault(subjectPointer->muscleDefault());
    NMSmodelType::muscles_type& muscles(subjectPointer->muscles());
    MusclesType::muscle_sequence& muscleSequence( muscles.muscle() );  
  
    for (MusclesType::muscle_iterator i(muscleSequence.begin()); i != muscleSequence.end(); ++i) {
        string muscleName = (*i).name();
        MTUT currentMuscle;
        mySubject.getMuscle(currentMuscle, muscleName);
        (*i).C1(currentMuscle.getC1());
        (*i).C2(currentMuscle.getC2());
        (*i).shapeFactor(currentMuscle.getShapeFactor());
        (*i).optimalFiberLength(currentMuscle.getOptimalFiberLength());
        (*i).pennationAngle(currentMuscle.getPennationAngle());
        (*i).tendonSlackLength(currentMuscle.getTendonSlackLength());
        muscleDefault.percentageChange(currentMuscle.getPercentageChange());
        muscleDefault.damping(currentMuscle.getDamping());
        (*i).maxIsometricForce(currentMuscle.getMaxIsometricForce());
        (*i).strengthCoefficient(currentMuscle.getStrengthCoefficient());   
   }  
    xml_schema::namespace_infomap map;
    map[""].name = "";
    map[""].schema = "NMSmodel.xsd";
    
    cout << "SUBJECT.XML is" << endl;
        // Serialize to a file.
        //
    std::ofstream ofs (XMLfilename.c_str());
    subject(ofs, *subjectPointer, map);
    
}
  
  