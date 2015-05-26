.. _using_ceinmscalibrate:

Using CEINMScalibrate
#####################

The CEINMScalibrate executable implements the subject calibration procedure presented in :ref:`introCalibration`. It is aimed at refining muscle parameters, that determine how muscles generate force, using an optimization algorithm that minimizes a cost function. Currently, the cost function that is implemented is the error between the estimated and the measured joint moments during a set of tasks.

To summarize, before running the CEINMScalibrate software, you should:

- prepare your experimental data, as explained in :ref:`prepareExperimentalData`. Remember that you need the *joint moments* files for the trials that you want to use for calibration.
- prepare your setup files: the :ref:`main CEINMScalibrate setup file <ceinmscalibrateConfigurationFile>` and the files it includes.

Most of the setup files are related to the model definition and the interpretation of input data; please refer to the related sections in :ref:`ceinmsConfigFiles` for complete information. The file that is most critical and needs further explanations is the :ref:`calibConfigFile`. Indeed, modifying this file might yield to very different results in terms of calibrated parameters: extra care must be taken when defining calibration properties, as they depend both on the subject description and the application you are interested in.

Parameters of the optimization algorithm can be modified to reduce computation time (e.g., for *simulated annealing*, increasing the value for ``epsilon`` or ``rT``, or reducing ``noEpsilon``, ``NT`` or ``NS``), but convergence to the global minimum becomes less likely.

As for parameter ranges specification, one must always keep in mind that increasing the number of variables in an optimization problem yields to increased computational costs and less reliable solutions (the so-called *curse of dimensionality*). Therefore, while it is technically possible to calibrate all muscular parameters that describe the subject, one should give some consideration to which parameters can be estimated (and how reliably) with other techniques (i.e., scaling generic models appropriately, with imaging techniques, and so on) and which are to be adjusted through the calibration process.


Executing the software
=======================

Once you have installed the software (see :ref:`installCeinms`) and prepared the above-mentioned setup and data files, you can run the software by typing in your terminal (or command prompt in Windows)


    ``CEINMScalibrate -S <path to main XML setup file>``

If setup and data files can be correctly found, calibration will start after the number of total parameters is printed to screen. Calibration time may vary depending on the number of parameters you are calibrating and the computing power of your computer, so it may take several minutes before any information is printed to screen.

.. important:: Please make sure that the output subject file specified in the :ref:`main setup XML file <ceinmscalibrateConfigurationFile>` is to be written into a folder that already exists.


Output
======

When software execution finishes, an XML description of the calibrated subject will be written to the file that was specified as ``outputSubjectFile`` in the :ref:`main setup XML file <ceinmscalibrateConfigurationFile>`. We encourage you to check that calibrated parameters are still consistent with any information you have on the subject's physiology. In particular, we suggest that you check if any parameter hit the boundary of the calibration range, which may suggest that either the boundaries are incorrect, or that the experimental data you provided was not consistent with the model.
