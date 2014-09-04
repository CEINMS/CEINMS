.. _inputDataDescFile:

Trial (input) data description file
#####################################

.. only:: html

   .. contents::


XML example
------------------------

.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <inputData xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="inputData.xsd">
        <muscleTendonLengthFile>MuscleAnalysis/subject01_MuscleAnalysis_Length.sto</muscleTendonLengthFile>
        <excitationsFile>emg.mot</excitationsFile>
        <momentArmsFiles>
            <momentArmsFile dofName="ankle_angle_r">MuscleAnalysis/subject01_MuscleAnalysis_MomentArm_ankle_angle_r.sto</momentArmsFile>
            <momentArmsFile dofName="hip_adduction_r">MuscleAnalysis/subject01_MuscleAnalysis_MomentArm_hip_adduction_r.sto</momentArmsFile>
            <momentArmsFile dofName="hip_flexion_r">MuscleAnalysis/subject01_MuscleAnalysis_MomentArm_hip_flexion_r.sto</momentArmsFile>
            <momentArmsFile dofName="knee_angle_r">MuscleAnalysis/subject01_MuscleAnalysis_MomentArm_knee_angle_r.sto</momentArmsFile>
        </momentArmsFiles>
        <externalTorquesFile>ID/inversedynamics.sto</externalTorquesFile>
    </inputData>

An input data description file consists of a root element named ``inputData`` that contains the following elements:

- ``muscleTendonLengthFile``: the location (absolute or relative to the location of the xml file itself) of the file containing the length of the muscle-tendon units during the trial
- ``momentArmsFiles``: a list of ``momentArmsFile`` elements, each being the location (absolute or relative to the location of the xml file itself) of the file containing the moment arms  of the muscle-tendon units, relative to the degree of freedom given as ``dofName`` attribute, during the trial
- ``excitationsFile``: the location (absolute or relative to the location of the xml file itself) of the file containing the excitations (usually, EMG signal envelopes) recorded for the trial
- (optionally) ``externalTorquesFile``: the location (absolute or relative to the location of the xml file itself) of the file containing the torque that the subject exerts at each joint.

For a more detailed explanation of the format and content of all input data files, see :ref:`prepareExperimentalData`.


XSD grammar
------------------------
.. literalinclude:: ../../../cfg/XSD/inputData.xsd
   :language: xml
   :encoding: latin-1
