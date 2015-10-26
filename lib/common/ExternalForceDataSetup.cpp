#include "ceinms/ExternalForceDataSetup.h"

namespace ceinms {

    std::vector<ExternalForceDataSetup> parseExternalLoadsXml(const std::string& xmlFilename) {

        OpenSim::XMLDocument document(xmlFilename);
        SimTK::Xml::Element root = document.getRootDataElement();
        SimTK::Xml::element_iterator extForceElementIter = root.element_begin("objects")->element_begin("ExternalForce");

        std::vector<ExternalForceDataSetup> externalForcesSetups;
        for (extForceElementIter; extForceElementIter != root.element_begin("objects")->element_end(); ++extForceElementIter) {
            ExternalForceDataSetup fds;
            fds.name = extForceElementIter->getOptionalAttributeValue("name");
            fds.forceIdentifier = extForceElementIter->getRequiredElementValue("force_identifier");
            fds.pointIdentifier = extForceElementIter->getRequiredElementValue("point_identifier");
            fds.torqueIdentifier = extForceElementIter->getRequiredElementValue("torque_identifier");
            fds.appliedToBodyName = extForceElementIter->getRequiredElementValue("applied_to_body");
            fds.forceExpressedInBodyName = extForceElementIter->getRequiredElementValue("force_expressed_in_body");
            fds.pointExpressedInBodyName = extForceElementIter->getRequiredElementValue("point_expressed_in_body");
            externalForcesSetups.push_back(fds);
        }

        return externalForcesSetups;
    }
}