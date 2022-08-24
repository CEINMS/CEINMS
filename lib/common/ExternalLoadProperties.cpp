#include "ceinms/ExternalLoadProperties.h"

namespace ceinms {

    ExternalLoadProperties::ExternalLoadProperties(const std::string& xmlFilename) :
        xmlFilename_(xmlFilename) {

        SimTK::Xml::Document extLoadXml(xmlFilename);   //SimTK::Xml -> SimTK::Xml::Document
        parseXml(extLoadXml.getRootElement());

        if (!SimTK::Pathname::fileExists(datafile_)) {
            bool isAbsolute(false);
            std::string currentDir, filename, extension;
            SimTK::Pathname::deconstructPathname(xmlFilename_, isAbsolute, currentDir, filename, extension);
            datafile_ = SimTK::Pathname::getAbsolutePathname(currentDir + SimTK::Pathname::getPathSeparator() + datafile_);
        }

        if (!SimTK::Pathname::fileExists(external_loads_model_kinematics_file_)) {
            bool isAbsolute(false);
            std::string currentDir, filename, extension;
            SimTK::Pathname::deconstructPathname(xmlFilename_, isAbsolute, currentDir, filename, extension);
            external_loads_model_kinematics_file_ = SimTK::Pathname::getAbsolutePathname(currentDir + SimTK::Pathname::getPathSeparator() + external_loads_model_kinematics_file_);
        }
    }

    void ExternalLoadProperties::parseXml(SimTK::Xml::Element elt) {
        SimTK::Xml::element_iterator eIt(elt.element_begin());
        if (eIt->getElementTag() == "ExternalLoads")
            parseExternalLoads(*eIt);
    }

    void ExternalLoadProperties::parseExternalLoads(SimTK::Xml::Element elt) {
        SimTK::Xml::attribute_iterator ap = elt.attribute_begin();
        if (ap->getName() == "name")
            name_ = ap->getValue();

        SimTK::Xml::element_iterator eIt(elt.element_begin());
        for (eIt; eIt != elt.element_end(); ++eIt) {
            if (eIt->getElementTag() == "objects") parseObjects(*eIt);
            if (eIt->getElementTag() == "datafile") datafile_ = eIt->getValue();
            if (eIt->getElementTag() == "external_loads_model_kinematics_file") external_loads_model_kinematics_file_ = eIt->getValue();
            if (eIt->getElementTag() == "lowpass_cutoff_frequency_for_load_kinematics") lowpass_cutoff_frequency_for_load_kinematics_ = eIt->getValueAs<double>();
        }
    }

    void ExternalLoadProperties::parseObjects(SimTK::Xml::Element elt) {
        SimTK::Xml::element_iterator eIt(elt.element_begin());
        for (eIt; eIt != elt.element_end(); ++eIt) {
            if (eIt->getElementTag() == "ExternalForce") {
                ExternalForceProperties extForce(*eIt);
                externalForcesProperties_.push_back(extForce);
            }
        }
    }

    std::size_t ExternalLoadProperties::getExternalForcesSize() const{
        return externalForcesProperties_.size();
    }

    std::vector<ExternalForceProperties> ExternalLoadProperties::getExternalForces() const {
        return externalForcesProperties_;
    }

    std::vector<std::string> ExternalLoadProperties::getExternalForcesNames() const {
        std::vector<std::string> names;
        for (auto& e : externalForcesProperties_)
            names.emplace_back(e.getName());
        return names;
    }

    std::string ExternalLoadProperties::getName() const {
        return name_;
    }

    std::string ExternalLoadProperties::getDatafile() const {
        return datafile_;
    }

    std::string ExternalLoadProperties::getKinematicsFile() const {
        return external_loads_model_kinematics_file_;
    }

    double ExternalLoadProperties::getCutoffFrequency() const {
        return lowpass_cutoff_frequency_for_load_kinematics_;
    }

    std::string ExternalLoadProperties::getForceIdentifier(const std::string& forceName) const {
        std::string forceIdentifier;
        for (auto& e : externalForcesProperties_) {
            if (e.getName() == forceName)
                forceIdentifier = e.getForceIdentifier();
        }
        return forceIdentifier;
    }

    std::string ExternalLoadProperties::getPointIdentifier(const std::string& forceName) const {
        std::string pointIdentifier;
        for (auto& e : externalForcesProperties_) {
            if (e.getName() == forceName)
                pointIdentifier = e.getPointIdentifier();
        }
        return pointIdentifier;
    }

    std::string ExternalLoadProperties::getTorqueIdentifier(const std::string& forceName) const {
        std::string torqueIdentifier;
        for (auto& e : externalForcesProperties_) {
            if (e.getName() == forceName)
                torqueIdentifier = e.getTorqueIdentifier();
        }
        return torqueIdentifier;
    }
}
