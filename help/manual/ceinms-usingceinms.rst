.. _using_ceinms:

Using CEINMS
############

The CEINMS executable allows to run neuromusculoskeletal simulations in different operation modes, as was discussed in the :ref:`introduction <ceinmsOverview>`, depending on the content of the ``NMSmodel`` element in :ref:`executionConfigFile`.

Within this element, the ``openLoop`` type means that the execution will run in *full-predictive open-loop*, therefore no joint moments file (see :ref:`prepareDataInputData`) is required for the trial that is being processed.
The ``hybrid`` execution modality is more complex, as it allows to select the objective function weightings alpha, beta and gamma, the muscles considered in the optimization, and to select the optimization algorithm.
Depending on the choice of ``adjustMTUs``, ``synthMTUs`` and weightings, we label the algorithm differently (see also :ref:`introHybrid`):

*Hybrid mode*

    alpha = 1, beta > 0, gamma = 0

    synthMTUs = every muscle without experimental excitations

    adjustMTUs = none

*EMG-assisted mode*

    alpha = 1, beta > 1, gamma > 1

    synthMTUs = every muscle without experimental excitations

    adjustMTUs = every muscle with experimental excitations

*Static optimisation mode*

    alpha = 1, beta > 0, gamma = 0

    synthMTUs = every muscle

    adjustMTUs = none

As for the ``tendon`` element, the ``equilibriumElastic`` option usually provides more accurate results, while using a ``stiff`` tendon reduces computation time.


Executing the software
=======================

Once you have installed the software (see :ref:`installCeinms`) and prepared the above-mentioned setup and data files, you can run the software by typing in your terminal (or command prompt in Windows)


    ``CEINMS -S <path to main XML setup file>``

If setup and data files can be correctly found, the execution will run and some information on current results will be reported to screen.
Upon completion, you will find, inside the output folder you specified in the setup file, a set of storage files containing all the quantities that are calculated during the simulation of each muscle's behaviour, such as activation, lenght and contraction velocity of the fibres, and ultimately muscle forces. Furthermore, joint moments are computed, that can be compared to experimental ones. While it is generally difficult to compare single muscles' quantities against experimental data, these files are useful to get a better insight at how each muscle is behaving, and to verify that there are no errors or artifacts in the input data or in the model parameter.
