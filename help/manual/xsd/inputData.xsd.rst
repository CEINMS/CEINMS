.. _inputDataDescFile:

Trial (input) data description file
#####################################

.. only:: html

   .. contents::


XML example
------------------------

.. code-block:: xml

   TODO: fill in code block


An input data escription file consists of a root element named ``inputData`` that contains the following elements:

- ``muscleTendonLengthFile``: the location (absolute or relative to the location of the xml file itself) of the file containing the length of the muscle-tendon units during the trial
- ``momentArmsFiles``: a list of ``momentArmsFile`` elements, each being the location (absolute or relative to the location of the xml file itself) of the file containing the moment arms  of the muscle-tendon units, relative to the degree of freedom given as ``dofName`` attribute, during the trial
- ``emgFile``: the location (absolute or relative to the location of the xml file itself) of the file containing the EMG excitations recorded for the trial
- (optionally) ``externalTorquesFile``: the location (absolute or relative to the location of the xml file itself) of the file containing the torque that the subject exerts at each joint.

For a more detailed explanation of the format and content of all input data files, see :ref:`prepareExperimentalData`.


XSD grammar
------------------------
.. literalinclude:: ../../../cfg/XSD/inputData.xsd
   :language: xml
   :encoding: latin-1
