CEINMS configuration files
##########################

CEINMS is entirely configured through a set of XML files. XSD schemas for these files are provided together with the software.
A list of the XML files is given below. XML file *names* are given as examples, but they are not fixed, as they are configurable themselves; what matters is that their *content* matches the corresponding XSD grammar.

The main files that govern the configuration of the CEINMS executables are:

- :ref:`ceinmsSetup.xml<xsd_ceinmsSetup_xml>`: a CEINMS setup file (according to :ref:`ceinmsSetup.xsd <xsd_ceinmsSetup_xsd>`)
- ``ceinmsCalibrationSetup.xml``: a CEINMScalibrate setup file (according to :doc:`ceinmsCalibrationSetup.xsd </manual/xsd/ceinmsCalibrationSetup.xsd>`)

These files point to the other xml files that determine the whole configuration for the execution of the CEINMS software:

- ``subject.xml``: a description of the model of the subject under investigation (according to :doc:`subject.xsd </manual/xsd/subject.xsd>`)
- ``inputData.xml``: a description of the experimental data that guide the simulation (according to :doc:`inputData.xsd </manual/xsd/inputData.xsd>`)
- ``emgGenerator.xml``: a description of the relationship between the recorded EMG channels and the muscles in the subject model (according to ``emgGenerator.xsd``)

then, for CEINMS-calibrate

- ``calibration.xml``: a file that specifies which model parameters to calibrate and how to perform the calibration (according to ``calibration.xsd``)

and for CEINMS

- ``execution.xml``: a file that specifies the configuration for the simulation (according to ``execution.xsd``)

.. only:: html

    For convenience, here are all the XSD files:

.. only:: latex

    In the following, an example of each configuration file is provided and explained, and the corresponding grammar scheme is reported.

.. toctree::
   :maxdepth: 1

   CEINMS setup XSD </manual/xsd/ceinmsSetup.xsd>
   CEINMScalibrate setup XSD </manual/xsd/ceinmsCalibrationSetup.xsd>
   Subject XSD </manual/xsd/subject.xsd>
   InputData XSD </manual/xsd/inputData.xsd>

