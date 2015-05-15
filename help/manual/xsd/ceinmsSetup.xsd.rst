CEINMS main setup
#################

.. only:: html

   .. contents::

.. _xsd_ceinmsSetup_xml:

This file is a simple list of the configuration and data files that define a CEINMS simulation.


CEINMS setup XML example
------------------------

.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <ceinms xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
            xsi:noNamespaceSchemaLocation="ceinmsSetup.xsd">
      <subjectFile>subjectCalibrated.xml</subjectFile>
      <inputDataFile>../TestData/walking1/walking1.xml</inputDataFile>
      <executionFile>Execution/executionTest.xml</executionFile>
      <excitationGeneratorFile>excitationGenerator-16To34.xml</excitationGeneratorFile>
      <outputDirectory>../../OutputData/walking1/</outputDirectory>
    </ceinms>

A CEINMS setup file consists of a root element named ``ceinms`` that contains the following elements:

- ``subjectFile`` the location of the :doc:`subject description file </manual/xsd/subject.xsd>`
- ``inputDataFile`` the location of the :doc:`trial (input) data description file </manual/xsd/inputData.xsd>`
- ``executionFile`` the location of the :doc:`execution parameters file </manual/xsd/execution.xsd>`
- ``excitationGeneratorFile`` the location of the :doc:`/manual/xsd/excitationGenerator.xsd`
- ``outputDirectory`` the folder where output results are saved


.. _xsd_ceinmsSetup_xsd:

CEINMS setup XSD grammar
------------------------

.. literalinclude:: ../../../cfg/XSD/ceinmsSetup.xsd
   :language: xml
   :encoding: latin-1
