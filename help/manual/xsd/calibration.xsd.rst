.. _calibConfigFile:

Calibration configuration file
##############################

.. only:: html

   .. contents::


XML example
------------------------

.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <calibration xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                 xsi:noNamespaceSchemaLocation="calibration.xsd">

      <algorithm>
        <simulatedAnnealing>
          <noEpsilon>4</noEpsilon>
          <rt>0.3</rt>
          <T>20</T>
          <NS>15</NS>
          <NT>5</NT>
          <epsilon>1.E-5</epsilon>
          <maxNoEval>200000</maxNoEval>
        </simulatedAnnealing>
      </algorithm>

      <NMSmodel>
        <type>
          <openLoop/>
        </type>

        <tendon>
          <stiff/>
        </tendon>

        <activation>
          <exponential/>
        </activation>
      </NMSmodel>

        <calibrationSteps>
            <step>
                <dofs>knee_angle_r hip_flexion_r hip_adduction_r ankle_angle_r</dofs>
                <objectiveFunction>
                    <minimizeTorqueError/>
                </objectiveFunction>
                <parameterSet>
                    <parameter>
                        <name>c1</name>
                        <single/>
                        <absolute>
                            <range>-0.95 -0.05</range>
                        </absolute>
                    </parameter>
                    <parameter>
                        <name>c2</name>
                        <single/>
                        <absolute>
                            <range>-0.95 -0.05</range>
                        </absolute>
                    </parameter>
                    <parameter>
                        <name>shapeFactor</name>
                        <single/>
                        <absolute>
                            <range>-2.999 -0.001</range>
                        </absolute>
                    </parameter>
                    <parameter>
                        <name>tendonSlackLength</name>
                        <single/>
                        <relativeToSubjectValue>
                            <range>0.85 1.15</range>
                        </relativeToSubjectValue>
                    </parameter>
                    <parameter>
                        <name>strengthCoefficient</name>
                        <muscleGroups>
                            <muscles>addbrev_r addlong_r addmag1_r
                                     addmag2_r addmag3_r </muscles>
                            <muscles>bicfemlh_r semimem_r semiten_r sar_r</muscles>
                            <muscles>bicfemsh_r</muscles>
                            <muscles>gaslat_r gasmed_r </muscles>
                            <muscles>gmax1_r gmax2_r gmax3_r</muscles>
                            <muscles>gmed1_r gmed2_r gmed3_r
                                     gmin1_r gmin2_r gmin3_r</muscles>
                            <muscles>gra_r recfem_r tfl_r</muscles>
                            <muscles>illiacus_r psoas_r</muscles>
                            <muscles>perbrev_r perlong_r perter_r tibant_r</muscles>
                            <muscles> sol_r</muscles>
                            <muscles>vasint_r vaslat_r vasmed_r</muscles>
                        </muscleGroups>
                        <absolute>
                            <range>0.5 2.5</range>
                        </absolute>
                    </parameter>

                </parameterSet>
            </step>
        </calibrationSteps>

      <trialSet>../../subject01/walking1/walking1.xml</trialSet>

    </calibration>


A calibration configuration file consists of a root element named ``calibration`` that contains the following elements:

- ``algorithm``: a description of the optimization algorithm to use (see `algorithm`_)
- ``NMSmodel``: a collection of options for the simulation of the model (see the corresponding section :ref:`executionNMSmodel` in the execution description file)
- ``calibrationSteps``: a list of ``step`` elements (at least one) that describe an optimization procedure (see `step`_)
- ``trialSet``: a list of XML files, each describing a trial to be used for calibration (see :ref:`inputDataDescFile`)

.. _optimizationalgorithm:

algorithm
=========

This is the optimization algorithm that is used to minimize the objective function associated to each calibration `step`_ (``objectiveFunction`` element). At the moment, only the *simulated annealing* algorithm is available, that is described in :ref:`introSimAnnealing`.

The parameters for the simulated annealing algorithm are:

- ``noEpsilon`` number of consecutive times that the cost function differs less than ``epsilon`` from the current optimum value
- ``rt`` coefficient by which the temperature is multiplied when it is reduced
- ``T`` initial value for the temperature
- ``NS`` number of evaluation steps after which the step length is re-adjusted
- ``NT`` number of step-length adjustment loops after which the temperature is decreased
- ``epsilon`` threshold value to consider negligible the difference of current cost function value from optimal one
- ``maxNoEval`` maximum number of evaluations of the cost function


step
====

This is the description of a calibration tasks.
It contains the following elements:

- ``dofs``: a list of the degrees of freedom to consider for model calibration
- ``objectiveFunction``: the target function to minimize in order to calibrate the subject model
- ``parameterSet``: a list of ``parameter`` elements, each describing how to calibrate one type of muscle parameter (see `parameter`_


parameter
=========

- ``name``: parameter identifier
- a specifier for which muscles "share" the same parameter value (this varies the number of parameters to calibrate):

    - ``global``: single value for all muscles, or
    - ``single``: different value for each muscle, or
    - ``muscleGroups``: a list of ``muscles`` elements, each containing a list of muscles that share the same value for the parameter

- a specifier for the calibration range. All elements of this type contain a ``range`` element, that consists of two numbers, representing respectively the minimum and maximum value of the range. The actual extremes depend on the type of specifier:

    - ``absolute``: the values in ``range`` are the absolute minimum and maximum values that are allowed for the parameter
    - ``relativeToSubjectValue``: the values in ``range`` represent the multiplying factor by which to multiply the initial value for this parameter (in the input subject description XML, see :ref:`subjectDescFile`) to obtain the actual minimum and maximum values.



XSD grammar
------------------------

.. literalinclude:: ../../../cfg/XSD/calibration.xsd
   :language: xml
   :encoding: latin-1
