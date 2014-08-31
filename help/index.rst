.. title:: CEINMS Reference Documentation


CEINMS
******

.. only:: html or latex

   .. contents::


Introduction
############

CEINMS is a toolbox for EMG-driven simulation of musculoskeletal models.

Background and Development of CEINMS
====================================

.. toctree::
   :maxdepth: 1

      CEINMS Overview </introduction/overview>
      Neuromusculoskeletal models </introduction/models>
      Bibliography </introduction/bibliography>

CEINMS installation instructions
################################

Sadly, nothing here yet, but maybe we could start creating packages using cpack (only executables and, if needed, shared libraries - dev is still too messy, and most of the implementation is exposed because of the templates)

CEINMS configuration files
##########################

CEINMS is entirely configured through XML files. Each file type is described by an `XML schema (XSD) <http://www.w3schools.com/schema/>`_. This allows to check the *formal* correctness of an XML file without even launching CEINMS, using any XML-validating software (for example, `XML Copy Editor <http://xml-copy-editor.sourceforge.net/>`_). XSD files are provided together with the software.

Configuration files are presented in the following sections. An example of each file is provided and explained, and the corresponding grammar scheme is reported.
Files are divided into three categories:

.. toctree::
   :maxdepth: 1

    Execution (configuration parameters for a CEINMS simulation) <manual/ceinms-setups-execution>
    Calibration (configuration parameters for a CEINMS calibration) <manual/ceinms-setups-calibration>
    Data description (representation of the subject model and experimental data) <manual/ceinms-setups-commondata>


Preparing your experimental data
################################

- to use ceinms you need: emg, lmt, ma, external torques (see conf files and ceinms/ceinms-calibrate)
- Collect data (suggested: static pose to scale subject, mandatory for now: MVCs)
- Export in c3d
- Use MoToNMS
- [ Specify here the folder structure that we intend to use ]
- Scale subject (provide sample setup.xml or link to Opensim docs and, when the time is ripe, to the muscleOptimizer)
- convert subject using osimToXML (need a template with the model you are studying?)
- use claudio's scripts to:
    - perform ik with opensim
    - perform id with opensim
    - perform muscle analysis with opensim
    - prepare inputData.xml

**Don't forget to sanity-check your results!**

.. _using_ceinms:

Using CEINMS
############

- what you use ceinms for ( see the theory)
- recap of what you need: see config files, see prepare your data
- explain the actual meaning of some configs we didn't go into detail in the config files section
- explain what the ouput files are and what you can do with them (no profanities)


.. _using_ceinmscalibrate:

Using CEINMScalibrate
#####################

- what you use ceinms-calibrate for ( see the theory)
- recap of what you need: see config files, see prepare your data
- explain the actual meaning of some configs we didn't go into detail in the config files section
- some considerations about time vs accuracy tradeoff and the curse of dimensionality (old but gold)
- explain that all params can be calibrated, but we only suggest to calibrate some - which and why (give the impression we understand what we are doing)
- output file: give some hints for sanity checks (are the parameters close to the boundaries? - maybe we could add some check in the code for that)


CEINMS FAQ
##########

.. todo: Find some categories here, or remove this section entirely

Acknowledgements
################

CEINMS is being developed by the `Rehabilitation Engineering Group`_, `Dipartimento di Tecnica e Gestione dei Sistemi Industriali`_,
`Universita' degli Studi di Padova`_, and the `Centre For Musculoskeletal Research`_, `Griffith University`_.

.. list-table::

  * - .. image:: images/reglogo.jpg
        :height: 100pt
        :alt: DTG logo
        :target: `Rehabilitation Engineering Group`_
    - .. image:: images/unipd.png
        :height: 100pt
        :alt: UNIPD logo
        :target: `Universita' degli Studi di Padova`_
    - .. image:: images/griffith.png
        :height: 100pt
        :alt: Griffith logo
        :target: `Griffith University`_

.. only:: html or text

 Release Notes
 #############

 .. toctree::
    :maxdepth: 1

    /release/index


.. only:: html

 Index and Search
 ################

 * :ref:`search`
.. * :ref:`genindex`

