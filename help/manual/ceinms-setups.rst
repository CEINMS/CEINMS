CEINMS configuration files
##########################

CEINMS is entirely configured through XML files. Each file type is described by an `XML schema (XSD) <http://www.w3schools.com/schema/>`_. This allows to check the *formal* correctness of an XML file without even launching CEINMS, using any XML-validating software (for example, `XML Copy Editor <http://xml-copy-editor.sourceforge.net/>`_). XSD files are provided together with the software.

Configuration files are presented in the following sections. An example of each file is provided and explained, and the corresponding grammar scheme is reported.
Files are divided into three categories:

**execution** to define how to perform the simulation (see :doc:`/manual/ceinms-ceinms`)

.. toctree::
   :maxdepth: 2

   CEINMS main setup </manual/xsd/ceinmsSetup.xsd>
   Execution parameters</manual/xsd/execution.xsd>

**calibration** to define how to perform the model calibration (see :doc:`/manual/ceinms-ceinmscalibrate`)

.. toctree::
   :maxdepth: 2

   CEINMScalibrate main setup </manual/xsd/ceinmsCalibrationSetup.xsd>
   Calibration parameters</manual/xsd/calibration.xsd>

**common data** to describe the data that is required by the former categories, i.e., the model under investigation and how to find the input (experimental) data

.. toctree::
   :maxdepth: 2

   Subject description </manual/xsd/subject.xsd>
   Trial (input data) description </manual/xsd/inputData.xsd>
   EMG experimental-to-model mappings </manual/xsd/emgGenerator.xsd>

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
   :maxdepth: 2

   CEINMS setup XSD </manual/xsd/ceinmsSetup.xsd>
   CEINMScalibrate setup XSD </manual/xsd/ceinmsCalibrationSetup.xsd>
   Subject XSD </manual/xsd/subject.xsd>
   InputData XSD </manual/xsd/inputData.xsd>

