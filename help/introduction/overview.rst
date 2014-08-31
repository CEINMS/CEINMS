Overview
========

CEINMS is the result of an interdisciplinary collaboration among the biomechanics and the computer science worlds.
Inspired by early EMG-driven methods used in the biomechanical community, Lloyd and colleagues developed algorithms and software to calibrate EMG-driven, or now called EMG-informed, neuromusculoskeletal models to an individual :cite:`buchanan2004neuromusculoskeletal,gerus2013subject,lloyd2003emg,lloyd1996model,lloyd2001strategies,lloyd2008neuromusculoskeletal,sartori2013hybrid,sartori2012emg,winby2013correlation,winby2009muscle`

Fundamental to these calibrated methods was ability to validate the outputs to other data not used for calibration. All these algorithms and software were collected and integrated together to create CEINMS.
However, the complexity of these algorithms and software, and the growing breadth of research and applications it could be used for, CEINMS required highly specialised programming skills that are the domain of individuals with dedicated information technology background. Thus, using solid software programming patterns, along with the use of advanced programming techniques, the candidate solved the configuration and the modularity problems required to produce physiologically accurate EMG-informed neuromusculoskeletal models that are i) easy to implement in OpenSim, and ii) can execute in real-time.
Above all, CEINMS was designed and written to be flexible and generic software, that is, given the appropriate anatomical and physiological data, it can operate with any number of musculotendon units and any number of DOF. Moreover, the modular design allows the independent selection of different operation modes:

#. Full-predictive open-loop mode. The experimentally recorded EMG signals and 3D joint angles are used as input to a neuromusculoskeletal model to directly drive the computations of the musculotendon forces :cite:`gerus2013subject,lloyd2003emg,lloyd1996model,lloyd2001strategies,lloyd2008neuromusculoskeletal,sartori2012emg,winby2013correlation,winby2009muscle` (:num:`Fig. #figoverview`).
#. Hybrid mode. The excitation patterns muscles (e.g. deep muscles) from which it is not practical to routinely collect EMG signals, are constructed using optimization algorithms. Then, the constructed excitations, experimental EMGs, and 3D joint angles are used as input for the neuromusculoskeletal model :cite:`sartori2013hybrid`.
#. EMG-assisted mode. This mode is a more generalizable form of the Hybrid mode. It uses optimization to adjust both the excitations determined from experimentally recorded EMG signals and determine the excitations of muscles without experimental EMG. Then the muscle excitations, coupled with 3D joint angles, are used as input to the neuromusculoskeletal model.
#. Full optimization-driven closed-loop mode. In this mode, without the aid of experimental EMG data, an optimization algorithm is used to construct all the muscle excitations to drive the neuromusculoskeletal model :cite:`erdemir2007model,tsirakos1997inverse`. Importantly, the different operation modes can be executed on the same neuromusculoskeletal model, allowing a consistent comparison among the different neural solutions. Fundamental to the current EMG-informed methods and the above modes of operation, CEINMS can be calibrated to the individual subject (:num:`Fig. #calibration`) :cite:`lloyd2003emg`. This is a selected operation procedure in CEINMS, so it can run un-calibrated or calibrated state.





The aim of calibration is to determine the values for a set of parameters for each musculotendon unit. The first parameter set defines the musculotendon unit\u2019s activation dynamics (:num:`Fig. #calibration` and :num:`Fig. #processingflow`), which characterise the transformation of muscle excitation to muscle activation. The second parameter set define the musculotendon contraction dynamics (:num:`Fig. #calibration` and :num:`Fig. #processingflow`), which transforms the muscle activation and musculotendon kinematics to force :cite:`buchanan2004neuromusculoskeletal,gerus2013subject,lloyd2003emg,lloyd1996model,lloyd2001strategies,lloyd2008neuromusculoskeletal,sartori2013hybrid,sartori2012emg,winby2013correlation`.

.. _figoverview:

.. figure:: ../images/ceinmsOverview.png
   :align: center
   :width: 50%
   :alt: The schematic structure of CEINMS - open-loop full predictive mode
   :figclass: align-center

   The schematic structure of CEINMS using the open-loop full predictive mode.

.. _calibration:

.. figure:: ../images/calibration.png
   :align: center
   :width: 50%
   :alt: Schematic representation of the calibration procedure implemented in CEINMS
   :figclass: align-center

   Schematic representation of the calibration procedure implemented in CEINMS. The neuromusculoskeletal (NMS) model is started with an initial set of un-calibrated parameters taken from literature. The parameters are refined using an optimization algorithm to minimize the error between the estimated and the measured joint moments.

These parameters may change non-linearly across individuals, so simulated annealing :cite:`goffe1994global` is employed to alter the values of these various parameters to enable close tracking of the experimental joint moments and/or excitations derived from EMG signal, which are estimated from data collected during the execution of different motor tasks :cite:`buchanan2004neuromusculoskeletal,gerus2013subject,lloyd2003emg,lloyd1996model,lloyd2001strategies,lloyd2008neuromusculoskeletal,sartori2013hybrid,sartori2012emg,winby2013correlation,winby2009muscle`. Various calibration control functions (e.g. minimize maximum activation, minimize maximum joint contact forces etc.) can also be implemented to direct the final set of model parameters :cite:`gerus2013subject,sartori2013hybrid`. Finally, during calibration, the parameters are also constrained to vary within predefined boundaries to ensure muscles operate in their physiological range.
The result of the calibration is a subject-specific neuromusculoskeletal (NMS) model, which reflects the musculotendon physiology, activation and contraction dynamics for an individual.
Finally, CEINMS can be validated with a novel set of input data, which has not been used for the calibration process, and run with any of the four execution modes.

.. _processingflow:

.. figure:: ../images/processingFlow.png
   :align: center
   :width: 80%
   :alt: Data processing flow
   :figclass: align-center

   General data processing flow showing activation dynamics and contraction dynamics.
