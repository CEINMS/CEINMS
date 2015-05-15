.. _ceinmsConfigFiles:

CEINMS configuration files
##########################

CEINMS is entirely configured through XML files. Each file type is described by an `XML schema (XSD) <http://www.w3schools.com/schema/>`_. This allows to check the *formal* correctness of an XML file without even launching CEINMS, using any XML-validating software (for example, `XML Copy Editor <http://xml-copy-editor.sourceforge.net/>`_). XSD files are provided together with the software.

Configuration files are presented in the following sections. An example of each file is provided and explained, and the corresponding grammar scheme is reported.
Files are divided into three categories:

.. only:: latex

    - :doc:`Execution (configuration parameters for a CEINMS simulation) </manual/ceinms-setups-execution>`
    - :doc:`Calibration (configuration parameters for a CEINMS calibration) </manual/ceinms-setups-calibration>`
    - :doc:`Data description (representation of the subject model and experimental data) </manual/ceinms-setups-commondata>`

.. toctree::
   :maxdepth: 1

    Execution (configuration parameters for a CEINMS simulation) </manual/ceinms-setups-execution>
    Calibration (configuration parameters for a CEINMS calibration) </manual/ceinms-setups-calibration>
    Data description (representation of the subject model and experimental data) </manual/ceinms-setups-commondata>
