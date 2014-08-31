CEINMScalibrate configuration file
##################################

.. only:: html

   .. contents::

This file is a simple list of the configuration and data files that define a CEINMS subject calibration procedure.


XML example
------------------------
.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <ceinmsCalibration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="ceinmsCalibrationSetup.xsd">
      <subjectFile>subject.xml</subjectFile>
      <emgGeneratorFile>emgGenerator/emgGenerator-16To34.xml</emgGeneratorFile>
      <calibrationFile>Calibration/calibrationTest.xml</calibrationFile>
      <outputSubjectFile>subjectCalibrated.xml</outputSubjectFile>
    </ceinmsCalibration>

A CEINMScalibrate setup file consists of a root element named ``ceinmsCalibration`` that contains the following elements:

- ``subjectFile`` the location of the :doc:`subject description file </manual/xsd/subject.xsd>`
- ``emgGeneratorFile`` the location of the :doc:`EMG mapping description file </manual/xsd/emgGenerator.xsd>`
- ``calibrationFile`` the location of the :doc:`calibration parameters file </manual/xsd/calibration.xsd>` (including which trials to calibrate on)
- ``outputSubjectFile`` the name of the output (calibrated) subject file.

XSD grammar
------------------------
.. literalinclude:: ../../../cfg/XSD/ceinmsCalibrationSetup.xsd
   :language: xml
   :encoding: latin-1
