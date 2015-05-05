//__________________________________________________________________________
// Author(s): Claudio Pizzolato - October 2013
// email:  claudio.pizzolato@griffithuni.edu.au
//
// DO NOT REDISTRIBUTE WITHOUT PERMISSION
//__________________________________________________________________________
//

#include <string>
using std::string;
#include "subject.hxx"
using SubjectXsd::CalibrationInfoType;
using SubjectXsd::CalibratedType;
using SubjectXsd::SubjectType;

#include <fstream>
#include <iostream>
#include <list>
//#include "XmlWriter.h"
namespace ceinms {
    //TODO: it should re-parse all the info from the subject, instead of re-writing the oldXml
    XmlWriter::XmlWriter(const string& startXmlFile)
        :outFilename_("calibratedSubject.xml"), startXmlFile_(startXmlFile)
    {
        SubjectXmlReader sxr(startXmlFile);
        subjectPointer_ = sxr.getSubject();

    }


    void XmlWriter::setOutputFilename(const string& outFilename) {

        outFilename_ = outFilename;
    }


    void XmlWriter::addCalibrationInfo(const std::string& calibrationXmlFilename) {

        CalibrationInfoType::uncalibrated_optional& uncalibratedOpt(subjectPointer_->calibrationInfo().uncalibrated());
        if (uncalibratedOpt.present()) {
            uncalibratedOpt.reset();
            CalibratedType::calibrationSequence_type myCalibrationSequence;
            myCalibrationSequence.push_back(calibrationXmlFilename);
            CalibratedType::startSubjectFile_type myStartSubject(startXmlFile_);
            CalibratedType myCalibrated(myStartSubject, myCalibrationSequence);
            subjectPointer_->calibrationInfo().calibrated(myCalibrated);
        }
        else {
            SubjectType::calibrationInfo_type& myCalibrationInfo(subjectPointer_->calibrationInfo());
            SubjectType::calibrationInfo_type::calibrated_type::calibrationSequence_type& myCalibrationSequence(myCalibrationInfo.calibrated()->calibrationSequence());
            myCalibrationSequence.push_back(calibrationXmlFilename);
        }
    }


    template <typename NMSmodelT>
    void XmlWriter::setParameters(NMSmodelT& subject) {


        SubjectType::mtuDefault_type& mtuDefault(subjectPointer_->mtuDefault());
        SubjectType::mtuSet_type& mtuSet(subjectPointer_->mtuSet());
        MTUSetType::mtu_sequence& mtuSequence(mtuSet.mtu());

        for (MTUSetType::mtu_iterator i(mtuSequence.begin()); i != mtuSequence.end(); ++i) {
            string muscleName = (*i).name();
            typename NMSmodelT::MTUtype currentMuscle;
            subject.getMuscle(currentMuscle, muscleName);
            (*i).c1(currentMuscle.getC1());
            (*i).c2(currentMuscle.getC2());
            (*i).shapeFactor(currentMuscle.getShapeFactor());
            (*i).optimalFibreLength(currentMuscle.getOptimalFibreLength());
            (*i).pennationAngle(currentMuscle.getPennationAngle());
            (*i).tendonSlackLength(currentMuscle.getTendonSlackLength());
            (*i).maxContractionVelocity(currentMuscle.getMaxContractionVelocity());
            mtuDefault.percentageChange(currentMuscle.getPercentageChange());
            mtuDefault.damping(currentMuscle.getDamping());
            mtuDefault.emDelay(currentMuscle.getEmDelay());
            //  (*i).emDelay(currentMuscle.getEmDelay());
            //NOTE: single emDelay not available yet
            (*i).maxIsometricForce(currentMuscle.getMaxIsometricForce());
            (*i).strengthCoefficient(currentMuscle.getStrengthCoefficient());
        }


    }


    void XmlWriter::write() {

        xml_schema::namespace_infomap map;
        map[""].name = "";
        map[""].schema = "subject.xsd";

        std::ofstream ofs(outFilename_.c_str());
        subject(ofs, *subjectPointer_, map);
    }

}