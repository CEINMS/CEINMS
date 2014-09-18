.. _prepareExperimentalData:

Preparing your experimental data
################################

The aim of CEINMS is to simulate muscle dynamics, taking into account information on muscle excitations and the subject's kinematics during a task. Joint kinematics data collected experimentally however is not directly used as input for the CEINMS software: preprocessing steps are required to compute how the subject's motion reflects on the *geometrical* state of each muscle. The length of the muscle-tendon unit is a geometric property that affects force generation in Hill-type muscle models, while the moment arm of a muscle insertion point with respect to a joint rotation axis determines how that forces contributes to the joint total moment. This preprocessing step can be done through an anatomical modeling tool such as `OpenSim <http://simtk.org/home/opensim>`_.
In this section, an explanation of the input data files required for CEINMS execution will be provided, together with some indications on how to collect experimental data for EMG-driven muscle simulations.

.. _prepareDataInputData:

Input data
===========

The input quantities to run a CEINMS simulation from experimental data are:

- *musculo-tendon lengths* for the muscles in the model, calculated by means of an anatomical musculoskeletal model of the subject
- *moment arms* for the muscles that insist on each joint, calculated by means of an anatomical musculoskeletal model of the subject
- *muscle excitations* usually estimated from sEMG signals

To calibrate the subject's model on experimental data, for each of the involved trials one also needs

- *joint moments* at the joints of interest (usually estimated by means of inverse dynamics, see `Exploiting OpenSim`_)

Since these quantities are usually computed with different tools and in different steps, as will be shown in the next sections, they are stored in multiple files. In order to let the software know which files correspond to each trial, a configuration file for each trial has to be prepared, according to :ref:`inputDataDescFile`.

Storage file format
~~~~~~~~~~~~~~~~~~~~
File formats such `OpenSim motion (.mot) files <http://simtk-confluence.stanford.edu:8080/display/OpenSim/Motion+(.mot)+Files>`_ or `OpenSim storage (.sto) files <http://simtk-confluence.stanford.edu:8080/display/OpenSim/Storage+(.sto)+Files>`_ are accepted for input files. Output files are written as OpenSim storage.

The specifications for CEINMS input files however are more relaxed than OpenSim ones:

- there is an header section, that ends with a ``endheader`` line, followed by a data section
- within the header section, a line must indicate the number of total columns which is the first one) that will be found in the data section, for example 15, in one of these two formats:

    - nColumns=15
    - dataColumns 15

- within the header section, a line must indicate the number of data rows (samples) that will be found in the data section, for example 1264, in one of these two formats:

    - nRows=1264
    - dataRows 1264

- the data section begins with with a line containing tab-delimited or space-delimited labels for each column. The first column is *time*, followed by the list of muscle names or degrees of freedom that identify the data (note that these labels must match the names written in the XML configuration files, i.e., degrees of freedom and muscle names must match the :ref:`subjectDescFile`, and muscle activations must match the *inputSignals* in :ref:`excitationsDescFile`. The rows below this line of column labels must be the corresponding values of each of these quantities at the time represented by the first number in each row.


Additional acquisitions
=======================

Static acquisition
~~~~~~~~~~~~~~~~~~

When acquiring exponential data, we suggest that,  **for each subject**, you acquire a static pose of the subject with your motion capture system. This allows to create a more subject-specific model, starting from generic models, as explained in `Scaling`_.

Maximum voluntary contractions (MVC) acquisitions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since your subject's model should be independent from the electrode configuration you use during an acquisition session, you need to normalize each muscle's EMG data to a repeatable reference value.
Currently, the most common way among CEINMS users to acquire these reference values is by having the subject perform, **for each acquisition session**, a set of tasks in which one or more muscles are maximally contracted isometrically (no motion occurring), according to a pre-defined protocol.


Exporting and converting data
=============================

In order to elaborate the experimental data acquired with the motion analysis equipment available in your laboratory, we suggest that first of all you export your acquisitions into a standard `C3D format <http://www.c3d.org/>`_, which is supported by almost all manufacturers and vendors.

Secondly, we suggest that you use a conversion tool such as `MOtoNMS <https://simtk.org/home/motonms>`_ to extract and convert the data relevant for NMS modeling into appropriate file formats.

MOtoNMS
~~~~~~~

MOtoNMS is an open source `MATLAB <http://www.mathworks.com/products/matlab/>`_ toolbox that allows to pre-process data in a transparent and repeatable way. Moreover, it can be configured via XML files (or user-friendly GUIs) to adapt to different laboratory setups, without requiring any custom coding.

Key outputs of MOtoNMS are:

- from *static elaboration*:

    - marker traces to use to scale the subject (see `Scaling`_). Might include additional traces of calculated points, such as joint centers (see :ref:`motonms:StaticElaboration`)

- from *dynamic elaboration*:

    - marker traces for the trial (see `Inverse Kinematics`_)
    - external loads for the trial (see `Inverse Dynamics`_)
    - normalized EMG envelopes (see MOtoNMS manual's :ref:`motonms:DataProcessing` page), that can be used as *muscle excitations* (see `Input Data`_)


Data organization
~~~~~~~~~~~~~~~~~

MOtoNMS requires that input C3D files are located inside an ``InputData`` directory, or any of its subdirectory; processed data will be saved into an ``ElaboratedData`` directory, keeping the same folder organization, as explained in the MOtoNMS manual :ref:`motonms:DataOrganization` page. 

We suggest to enforce the same data organization for dealing with OpenSim and CEINMS processing: inside the ``ElaboratedData``, along with ``sessionData``, ``staticElaborations``, and ``dynamicElaborations``, we suggest you create a folder for each of the processing steps involved (e.g., Inverse Kinematics, CEINMS). This allows to automatize the most time-consuming tasks (i.e., preparing configuration files and pre-processing data) as will be discussed in `Batch processing scripts`_).



Exploiting OpenSim
==================

In this section, a brief explanation is given of the preprocessing steps through which it is possible to obtain the input files listed in the `Input Data`_ section. The "meaning" of each step is discussed, so that it should be easier to obtain the same quantities with any other musculoskeletal modeling tool you may want to use.

Scaling
~~~~~~~

In the absence of subject-specific neuromusculoskeletal models obtained, e.g., from imaging techiques (see :cite:`JMRI:JMRI20805` for an overview on the subject), it is common practice to generate subject-specific muscluloskeletal models starting from generic ones. This means that the anthropometry of a generic model is altered so that it matches a particular subject as closely as possible.

For additional information on scaling procedures, please refer to the `OpenSim manual Scaling page <http://simtk-confluence.stanford.edu:8080/display/OpenSim/Scaling>`_.

Inverse Kinematics
~~~~~~~~~~~~~~~~~~~

Starting from markers' positions, the values of the generalized coordinates (generally corresponding to joint angles) that describe the motion of body segments are estimated. This is needed both to determine muscle kinematics during the task, through `Muscle Analysis`_, and to determine how the external loads are applied on the subject, which is needed for `Inverse Dynamics`_.

For additional information on inverse kinematics in OpenSim, please refer to the `OpenSim manual Inverse Kinematics page <http://simtk-confluence.stanford.edu:8080/display/OpenSim/Inverse+Kinematics>`_.

Inverse Dynamics
~~~~~~~~~~~~~~~~

Inverse dynamics is aimed at estimating the net joint moments that are responsible for the given movement (i.e., the kinematics estimated through `Inverse Kinematics`_), given the kinetics information recorded on the environment (i.e., ground reaction forces measured through force platforms, the *external loads* mentioned in `MOtoNMS`_).
This step is actually optional: it provides the optional *joint moments* file (see `Input Data`_) that can be used for calibration or for validation of the model.

For additional information on inverse dynamics in OpenSim, please refer to the `OpenSim manual Inverse Dynamics page <http://simtk-confluence.stanford.edu:8080/display/OpenSim/Inverse+Dynamics>`_.

Muscle Analysis
~~~~~~~~~~~~~~~

Muscle analysis reports the state of the muscles during the execution of a given movement. We use it to obtain the lengths of muscle-tendon units, and the moment arms of each muscle with respect to the degrees of freedom it acts upon.

Muscle analysis can be performed using OpenSim's `Analyze Tool <http://simtk-confluence.stanford.edu:8080/display/OpenSim/Analyses>`_. The muscle-tendon unit lengths are written to a ``<prefix>_Length.sto`` file, while moment arms file names follow this convention: ``<prefix>_MomentArm_<dof_name>.sto``.
Since OpenSim documentation on this particular analysis is scarce, we include a template setup file in the following. Fields that must be filled in are marked by text within \*\* symbols.
Most important fields within the ``MuscleAnalysis`` block are:

- ``compute_moments`` that must be set to true (default is false)
- ``muscle_list`` list of muscles for which perform the analysis: you can specify a subset of muscles instead of ``all`` to reduce computation time and the number of output files, but be sure to include all muscles that belong to your CEINMS model
- ``moment_arm_coordinate_list`` list of degrees of freedom for which to compute the moment arms: as with the muscle list, you can specify a subset of degrees of freedom instead of all to reduce computation time and the number of ouput files, but be sure to include all the degrees of freedom of your CEINMS model.



.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" ?>
    <OpenSimDocument Version="30000">
        <AnalyzeTool name="MuscleAnalysisTool">
            <!--Name of the .osim file used to construct a model.-->
            <model_file> ** MODEL FILE **</model_file>
            <!--Replace the model's force set with sets specified in <force_set_files>? If false, the force set is appended to.-->
            <replace_force_set>false</replace_force_set>
            <!--List of xml files used to construct an force set for the model.-->
            <force_set_files />
            <!--Directory used for writing results.-->
            <results_directory>.</results_directory>
            <!--Output precision.  It is 8 by default.-->
            <output_precision>8</output_precision>
            <!--Initial time for the simulation.-->
            <initial_time> ** INITIAL TIME ** </initial_time>
            <!--Final time for the simulation.-->
            <final_time> ** FINAL TIME ** </final_time>
            <!--Flag indicating whether or not to compute equilibrium values for states other than the coordinates or speeds.  For example, equilibrium muscle fiber lengths or muscle forces.-->
            <solve_for_equilibrium_for_auxiliary_states>false</solve_for_equilibrium_for_auxiliary_states>
            <!--Maximum number of integrator steps.-->
            <maximum_number_of_integrator_steps>20000</maximum_number_of_integrator_steps>
            <!--Maximum integration step size.-->
            <maximum_integrator_step_size>1</maximum_integrator_step_size>
            <!--Minimum integration step size.-->
            <minimum_integrator_step_size>1e-008</minimum_integrator_step_size>
            <!--Integrator error tolerance. When the error is greater, the integrator step size is decreased.-->
            <integrator_error_tolerance>1e-005</integrator_error_tolerance>
            <!--Set of analyses to be run during the investigation.-->
            <AnalysisSet name="Analyses">
                <objects>
                    <MuscleAnalysis name="MuscleAnalysis">
                        <!--Flag (true or false) specifying whether whether on. True by default.-->
                        <on>true</on>
                        <!--Start time.-->
                        <start_time> ** INITIAL TIME, AS ABOVE ** </start_time>
                        <!--End time.-->
                        <end_time> ** FINAL TIME, AS ABOVE ** </end_time>
                        <!--Specifies how often to store results during a simulation. More specifically, the interval (a positive integer) specifies how many successful integration steps should be taken before results are recorded again.-->
                        <step_interval>1</step_interval>
                        <!--Flag (true or false) indicating whether the results are in degrees or not.-->
                        <in_degrees>true</in_degrees>
                        <!--List of muscles for which to perform the analysis. Use 'all' to perform the analysis for all muscles.-->
                        <muscle_list> all</muscle_list>
                        <!--List of generalized coordinates for which to compute moment arms. Use 'all' to compute for all coordinates.-->
                        <moment_arm_coordinate_list> all</moment_arm_coordinate_list>
                        <!--Flag indicating whether moments should be computed.-->
                        <compute_moments>true</compute_moments>
                    </MuscleAnalysis>
                </objects>
                <groups />
            </AnalysisSet>
            <!--Controller objects in the model.-->
            <ControllerSet name="Controllers">
                <objects />
                <groups />
            </ControllerSet>
            <!--XML file (.xml) containing the forces applied to the model as ExternalLoads.-->
            <external_loads_file> ** EXTERNAL LOADS FILE (optional) ** </external_loads_file>
            <!--Storage file (.sto) containing the time history of states for the model. This file often contains multiple rows of data, each row being a time-stamped array of states. The first column contains the time.  The rest of the columns contain the states in the order appropriate for the model. In a storage file, unlike a motion file (.mot), non-uniform time spacing is allowed.  If the user-specified initial time for a simulation does not correspond exactly to one of the time stamps in this file, inerpolation is NOT used because it is sometimes necessary to an exact set of states for analyses.  Instead, the closest earlier set of states is used.-->
            <states_file />
            <!--Motion file (.mot) or storage file (.sto) containing the time history of the generalized coordinates for the model. These can be specified in place of the states file.-->
            <coordinates_file> ** OUTPUT FILE FROM INVERSE KINEMATICS** </coordinates_file>
            <!--Storage file (.sto) containing the time history of the generalized speeds for the model. If coordinates_file is used in place of states_file, these can be optionally set as well to give the speeds. If not specified, speeds will be computed from coordinates by differentiation.-->
            <speeds_file />
            <!--Low-pass cut-off frequency for filtering the coordinates_file data (currently does not apply to states_file or speeds_file). A negative value results in no filtering. The default value is -1.0, so no filtering.-->
            <lowpass_cutoff_frequency_for_coordinates>-1</lowpass_cutoff_frequency_for_coordinates>
        </AnalyzeTool>
    </OpenSimDocument>


Batch processing scripts
~~~~~~~~~~~~~~~~~~~~~~~~

If you have many experimental trials to process, we warmly suggest that you automatize this pre-processing pipeline (from inverse kinematics onward). A simple way is to use scripts that customize template setup files with the actual paths of data files to use. This is most easy when you enforce a clear data organization scheme, as suggested in section `Data organization`_.

Examples of such scripts for inverse kinematics and inverse dynamics are available at `<https://github.com/RehabEngGroup/OpenSimProcessingScripts>`_.

.. important::

    Performing these steps automatically does not ensure that the results are correct. You should **always** check the results and, if they do not look plausible, check that that the template setup files are correct for your type of data/experiments, and that the setup files are generated correctly.

You may also want to automatize the creation of :ref:`trial (input) data description files <inputDataDescFile>`.


.. only:: html

    .. rubric:: Bibliography

.. bibliography:: litManuals.bib
    :cited:

