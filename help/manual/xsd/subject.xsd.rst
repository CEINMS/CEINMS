.. _subjectDescFile:

Subject description file
###############################

.. only:: html

   .. contents::


XML example
------------------------
.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <subject xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="subject.xsd">
        <mtuDefault>
            <emDelay>0.015</emDelay>
            <percentageChange>0.15</percentageChange>
            <damping>0.1</damping>
            <curve>
                <name>activeForceLength</name>
                <xPoints>-5 0 0.401 0.402 0.4035 0.52725 0.62875 0.71875 0.86125 1.045 1.2175 1.43875 1.61875 1.62 1.621 2.2 5</xPoints>
                <yPoints>0 0 0 0 0 0.226667 0.636667 0.856667 0.95 0.993333 0.77 0.246667 0 0 0 0 0</yPoints>
            </curve>
            <curve>
                <name>passiveForceLength</name>
                <xPoints>-5 0.998 0.999 1 1.1 1.2 1.3 1.4 1.5 1.6 1.601 1.602 5</xPoints>
                <yPoints>0 0 0 0 0.035 0.12 0.26 0.55 1.17 2 2 2 2</yPoints>
            </curve>
            <curve>
                <name>forceVelocity</name>
                <xPoints>-10 -1 -0.6 -0.3 -0.1 0 0.1 0.3 0.6 0.8 10</xPoints>
                <yPoints>0 0 0.08 0.2 0.55 1 1.4 1.6 1.7 1.75 1.75</yPoints>
            </curve>
        </mtuDefault>
        <mtuSet>
            <mtu>
                <name>add_brev_r</name>
                <c1>0.768956</c1>
                <c2>-0.990048</c2>
                <shapeFactor>0.101198</shapeFactor>
                <optimalFibreLength>0.152726160798647</optimalFibreLength>
                <pennationAngle>0</pennationAngle>
                <tendonSlackLength>0.0229663399697213</tendonSlackLength>
                <maxIsometricForce>429</maxIsometricForce>
                <strengthCoefficient>0.5</strengthCoefficient>
            </mtu>

                    ...

            <mtu>
                <name>vas_med_r</name>
                <c1>0.768956</c1>
                <c2>-0.990048</c2>
                <shapeFactor>0.101198</shapeFactor>
                <optimalFibreLength>0.1021279880827</optimalFibreLength>
                <pennationAngle>0.08726646</pennationAngle>
                <tendonSlackLength>0.144585690993485</tendonSlackLength>
                <maxIsometricForce>1294</maxIsometricForce>
                <strengthCoefficient>1.01759</strengthCoefficient>
            </mtu>
        </mtuSet>
        <dofSet>
            <dof>
                <name>hip_flexion_r</name>
                <mtuNameSet>add_brev_r add_long_r add_mag1_r add_mag2_r add_mag3_r bifemlh_r glut_max1_r glut_max2_r glut_max3_r glut_med1_r glut_med2_r glut_med3_r glut_min1_r glut_min2_r glut_min3_r grac_r iliacus_r psoas_r rect_fem_r sar_r semimem_r semiten_r tfl_r</mtuNameSet>
            </dof>
            <dof>
                <name>hip_adduction_r</name>
                <mtuNameSet>add_brev_r add_long_r add_mag1_r add_mag2_r add_mag3_r bifemlh_r glut_max1_r glut_max2_r glut_max3_r glut_med1_r glut_med2_r glut_med3_r glut_min1_r glut_min2_r glut_min3_r grac_r iliacus_r psoas_r rect_fem_r sar_r semimem_r semiten_r tfl_r</mtuNameSet>
            </dof>
            <dof>
                <name>hip_rotation_r</name>
                <mtuNameSet>add_brev_r add_long_r add_mag1_r add_mag2_r add_mag3_r bifemlh_r glut_max1_r glut_max2_r glut_max3_r glut_med1_r glut_med2_r glut_med3_r glut_min1_r glut_min2_r glut_min3_r grac_r iliacus_r psoas_r rect_fem_r sar_r semimem_r semiten_r tfl_r</mtuNameSet>
            </dof>
            <dof>
                <name>knee_angle_r</name>
                <mtuNameSet>bifemlh_r bifemsh_r lat_gas_r med_gas_r grac_r rect_fem_r sar_r semimem_r semiten_r tfl_r vas_int_r vas_lat_r vas_med_r</mtuNameSet>
            </dof>
            <dof>
                <name>ankle_angle_r</name>
                <mtuNameSet>lat_gas_r med_gas_r per_brev_r per_long_r per_tert_r soleus_r tib_ant_r</mtuNameSet>
            </dof>
        </dofSet>
        <calibrationInfo>
            <calibrated>
                <startSubjectFile>subject.xml</startSubjectFile>
                <calibrationSequence/>
            </calibrated>
        </calibrationInfo>
    </subject>


A subject description file consists of a root element named ``subject`` that contains the following elements:

- ``mtuDefault`` that contains the parameters that describe muscle (or, to be more precise, muscle-tendon-unit, MTU) properties common to all muscles (see `mtuDefault`_)
- ``mtuSet`` a list of ``mtu`` elements, each describing a muscle-tendon unit that actuates the model (see `mtu`_)
- ``dofSet`` a list of ``dof`` elements, each describing a degree of freedom of the model (see `dof`_)
- ``calibrationInfo`` that reports whether the subject has been calibrated (see `calibrationInfo`_)

mtuDefault
==========

The parameters common to all muscles are:

- ``emDelay`` the electromechanical delay (:math:`d` in :ref:`introActivationDynamics`)
- ``percentageChange`` the percentage change in optimal fibre length depending on activation (:math:`\gamma` in :ref:`introContractionDynamics`)
- ``damping`` the muscle damping coefficient (:math:`d_m` in :ref:`introContractionDynamics`)
- three ``curve`` elements, describing the normalized force/length and force/velocity curves shown in the :ref:`active and passive force curves <forcelengthcurves>`) figure in section :ref:`introContractionDynamics`. Each ``curve`` has a name, a list of y-coordinates, and the corresponding x-coordinates that can be interpolated to provide the complete curves.

mtu
===

Each muscle tendon unit is further described by the following, muscle-specific properties:

- ``name`` the identifier for the MTU
- ``c1`` and ``c2`` are the constants :math:`C_1` and :math:`C_2` that describe the recursive coefficients in :ref:`introActivationDynamics`
- ``shapeFactor`` is the non-linear shape factor that describes the relation between neural excitation and the muscle activation (:math:`A_j` in :ref:`introActivationDynamics`)
- ``optimalFibreLength`` is the optimal fibre length at maximum activation, (:math:`L_m^0` in :ref:`introContractionDynamics`)
- ``pennationAngle`` is the pennation angle of the fibre at its optimal length (:math:`\varphi_0` in :ref:`introContractionDynamics`)
- ``tendonSlackLength`` is the tendon slack length (:math:`l_{ts}` in :ref:`introContractionDynamics`)
- ``maxIsometricForce`` is the maximum isometric muscle force (:math:`F^{max}` in :ref:`introContractionDynamics`)
- ``strengthCoefficient`` is a multiplicative factor for ``maxIsometricForce``; the rationale for its inclusion is that, instead of calibrating the ``maxIsometricForce`` parameter for each muscle, you may want to keep it fixed and calibrate a reduced number of ``strength coefficient`` parameters instead, that can be shared by multiple muscles (e.g., belonging to the same muscle group - this is under the assumption that muscles in the same group develop in the same way). 


.. _subjectDof:

dof
===

Each degree of freedom (broadly speaking, a *joint* of the model) is thus defined:

- ``name`` the identifier for the degree of freedom
- ``mtuNameSet`` a list of the muscles that act on the joint


calibrationInfo
===============

If the file represents an uncalibrated subject, ``calibrationInfo`` will contain an ``uncalibrated`` element, made of:

- a ``subjectID`` element, containing a literal identifier for the subject
- an ``additionalInfo`` element, containing any additional note or information on the "origin" of the file (i.e., any pre-processing or scaling step that was performed)

If the file represents a calibrated subject (i.e, it is an output of the :ref:`CEINMScalibrate software <using_ceinmscalibrate>`), ``calibrationInfo`` will contain a ``calibrated`` element, made of:

- ``startSubjectFile`` the original XML subject file that was used as starting point of calibration
- ``calibrationSequence`` a list of the calibration steps that were performed (see :ref:`ceinmscalibrateConfigurationFile`)


XSD grammar
------------------------

.. literalinclude:: ../../../cfg/XSD/subject.xsd
   :language: xml
   :encoding: latin-1
