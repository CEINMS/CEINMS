.. _executionConfigFile:

Execution configuration file
##############################

.. only:: html

   .. contents::


XML example
------------------------
.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <execution xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
               xsi:noNamespaceSchemaLocation="execution.xsd">
      <NMSmodel>
         <type>
            <hybrid>
                <synthMTUs weight="50">psoas_r illiacus_r</synthMTUs>
                <adjustMTUs weight="10">
                                   recfem_r vasmed_r vaslat_r vasint_r bicfemsh_r
                                   gmin1_r gmin2_r gmin3_r gmed1_r gmed2_r gmed3_r
                                   tfl_r addbrev_r addlong_r addmag1_r addmag2_r
                                   addmag3_r gra_r sar_r semiten_r semimem_r bicfemlh_r
                                   gmax1_r gmax2_r gmax3_r gaslat_r gasmed_r sol_r perlong_r
                                   tibant_r perter_r perbrev_r
                </adjustMTUs>
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
    </execution>

An execution configuration file consists of a root element named ``execution`` that contains the following elements:

- ``NMSmodel``: the specification of muscular model to simulate (see `NMSmodel`_)
-  an ``online`` or ``offline`` empty element, that determines wheter fiber velocities are computed using a causal filter (``online``) or a non-causal filter (``offline``, recommended for most applications)
- (optional) ``elaboratedDoFs`` to select the degrees of freedom (see :ref:`subjectDof` ) to elaborate

    .. note:: this feature is not implemented yet.

.. _executionNMSmodel:

NMSmodel
========

This is the specification of the type of muscular model to use among the ones described in :ref:`introNMSmodels` and the desired operation mode:

- ``type``: the operation mode might contain an ``openLoop`` empty element, or an ``hybrid`` element
- ``tendon``: the behaviour of the tendon: it can be (see :ref:`introTendonModels` )

    - ``stiff`` (ST) rigid tendon
    - ``integrationElastic`` (IET) integration elastic tendon
    - ``equilibriumElastic`` (EET) equilibrium elastic tendon

- ``activation``: the formula to use to model the excitation-to-activation non-linear relation (see :ref:`introNeuralToMuscle` )

    - ``exponential``
    - ``piecewise``

hybrid
======

The hybrid execution modality (see :ref:`introHybrid`) configuration is provided here:

- ``alpha`` weight for the sum of the squared difference between experimental joint moments provided as :ref:`input data <prepareDataInputData>` and the joint moments estimated by CEINMS
- ``beta`` weight for the sum of squared muscle excitations
- ``gamma`` weight for the sum of the squared difference between experimental and predicted muscle excitations

By default, the weightings ``alpha``, ``beta``, and ``gamma`` are set to 1. To facilitate the creation of the xml, the parameter ``beta`` can be specified as the attribute ``weight`` of the ``synthMTUs`` tag, while ``gamma`` can be specified as the attribute ``weight`` of the ``adjustMTUs`` tag. See example above.

- ``synthMTUs`` list of muscles for which no EMG signal is available, therefore excitation is only predicted using static optimization techniques
- ``adjustMTUs`` list of muscles for which an EMG reference signal to "adjust" is provided


XSD grammar
------------------------
.. literalinclude:: ../../../cfg/XSD/execution.xsd
   :language: xml
   :encoding: latin-1
