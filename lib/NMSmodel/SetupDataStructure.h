// This is part of
// NeuroMuscoloSkeletal Model Software (NMS)
// Copyright (C) 2010 David Lloyd Massimo Sartori Monica Reggiani
//
// ?? Licenza ??
//
// The authors may be contacted via:
// email: massimo.sartori@gmail.com monica.reggiani@gmail.com


#ifndef SetupDataStructure_
#define SetupDataStructure_

#include <string>
#include "subject.hxx"
#include "Curve.h"

template<typename NMSmodelT>
class SetupDataStructure{
public:
    typedef typename NMSmodelT::MTUtype MTUT; 
    typedef typename NMSmodelT::DoFtype DoFT;
    typedef typename NMSmodelT::vectorMTUitr vectorMTUitrT;
    typedef Curve<CurveMode::Offline> CurveOffline;
    SetupDataStructure(const std::string& inputFile);
    void  createCurves();
    void  createMuscles(NMSmodelT& mySubject);
    void  createDoFs(NMSmodelT& mySubject);
    void  writeXMLCalibratedFile(NMSmodelT& mySubject, const std::string& XMLfilename);
private:
    // Create the body structure from the Input
    std::auto_ptr<SubjectType> subjectPointer_;
    CurveOffline forceVelocityCurve_;
    CurveOffline activeForceLengthCurve_;
    CurveOffline passiveForceLengthCurve_;
    CurveOffline tendonForceStrain_;
};

#include "SetupDataStructure.cpp"

#endif
