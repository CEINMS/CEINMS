Execution configuration file
##############################

.. only:: html

   .. contents::


XML example
------------------------
.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <execution xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="execution.xsd">
      <NMSmodel>
         <type>
            <hybrid>
                <alpha> 1 </alpha>
                <beta> 2 </beta>
                <gamma> 3 </gamma>
                <trackedMuscles>
                                   recfem_r vasmed_r vaslat_r vasint_r bicfemsh_r
                                   gmin1_r gmin2_r gmin3_r gmed1_r gmed2_r gmed3_r
                                   tfl_r addbrev_r addlong_r addmag1_r addmag2_r
                                   addmag3_r gra_r sar_r semiten_r semimem_r bicfemlh_r
                                   gmax1_r gmax2_r gmax3_r gaslat_r gasmed_r sol_r perlong_r
                                   tibant_r perter_r perbrev_r
                </trackedMuscles>
                <predictedMuscles> psoas_r illiacus_r</predictedMuscles>
                <algorithm>
                    <simulatedAnnealing>
                        <noEpsilon>4</noEpsilon>
                        <rt>0.3</rt>
                        <T>20</T>
                        <NS>15</NS>
                        <NT>5</NT>
                        <epsilon>1.E-3</epsilon>
                        <maxNoEval>200000</maxNoEval>
                    </simulatedAnnealing>
                </algorithm>
            </hybrid>
         </type>

         <tendon>
              <stiff/>
         </tendon>

         <activation>
              <exponential/>
         </activation>
      </NMSmodel>

      <online/>

      <elaboratedDoFs>
      </elaboratedDoFs>

    </execution>

An execution configuration file consists of a root element named ``execution`` that contains the following elements:

- ``NMSmodel``: the specification of muscular model to simulate (see `NMSmodel`_)
-  an ``online`` or ``offline`` empty element, depending on: TODO: what?
- (optional) ``elaboratedDoFs`` to select the degrees of freedom (see :ref:`subjectDof` ) to elaborate

    .. note:: this feature is not implemented yet.

.. _executionNMSmodel:

NMSmodel
========

This is the specification of the type of muscular model to use among the ones described in :ref:`introNMSmodels` and the desired operation mode:

- ``type``: the operation mode might contain an ``openLoop`` empty element, or an ``hybrid`` element  TODO: check if they match what we wrote in the introduction
- ``tendon``: the behaviour of the tendon: it can be (see :ref:`introTendonModels` )

    - ``stiff`` (ST) rigid tendon
    - ``elastic`` (IET) integration elastic tendon TODO: check if this is correct
    - ``elasticBiSec`` (EET) equilibrium elastic tendon

- ``activation``: the formula to use to model the excitation-to-activation non-linear relation (see :ref:`introNeuralToMuscle` )

    - ``exponential``
    - ``piecewise``

hybrid
======

.. todo:: add intro theory?

The hybrid execution modality configuration is provided here:

- ``alpha`` weight for TODO revise everything
- ``beta`` weight for
- ``gamma`` weight for
- ``trackedMuscles`` list of muscles for which an EMG reference signal to "track" is provided
- ``predictedMuscles`` list of muscles for which no EMG signal is available, therefore activation is only predicted using static optimization techniques
- ``algorithm`` the optimization algorithm that solves the hybrid static optimization problem, see :ref:`optimizationalgorithm`



XSD grammar
------------------------
.. literalinclude:: ../../../cfg/XSD/execution.xsd
   :language: xml
   :encoding: latin-1
