.. _excitationsDescFile:

Excitation mappings description file
######################################

.. only:: html

   .. contents::


XML example
------------------------
.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <excitationGenerator xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                         xsi:noNamespaceSchemaLocation="excitationGenerator.xsd">

    <inputSignals type="EMG">addmag_r bicfemlh_r gaslat_r gasmed_r
                gmax_r gmed_r gra_r perlong_r
                recfem_r sar_r semimem_r sol_r
                tfl_r tibant_r vaslat_r vasmed_r
    </inputSignals>

    <mapping>
        <excitation id="add_brev_r">
            <input weigth="1"> addmag_r </input>
        </excitation>
        <excitation id="add_long_r">
            <input weigth="1"> addmag_r </input>
        </excitation>
        <excitation id="glut_min1_r">
            <input weigth="0.5"> gmed_r </input>
            <input weigth="0.5"> gmax_r </input>
        </excitation>
        <excitation id="psoas_r"/>
     </mapping>

    </excitationGenerator>

An excitations mapping description file consists of a root element named ``excitationGenerator`` that contains the following elements:

- ``inputSignals``: a list of identifiers for the input signals that are selected or combined to generate the excitation patterns for the model's MTUs. The identifiers need to match the labels that will be present in the excitations data file ( see :ref:`prepareDataInputData` ). Optionally, a ``type`` attribute can be added to specify the nature of these input signals (e.g., *EMGenvelopes*, *synergies*, ...)
- ``mapping``: a list of `excitation`_ elements

excitation
==========

An ``excitation`` element is identified by an ``id`` literal attribute, that must correspond to an MTU name in the subject's model.
It contains a set of ``input`` elements, each being the name of one of the abovementioned ``inputSignals``, with a corresponding ``weight`` numeral attribute. The excitation value for the MTU will be given by a linear combination of the listed input signals, each multiplied by its weight.

Examples of most common cases:

- one-to-one mapping is represented as a single input signal with weight 1
- the mean of two signals is represented as a list of the two signals, each with weight 0.5
- an unknown excitation is represented as an empty element.


XSD grammar
------------------------

.. literalinclude:: ../../../cfg/XSD/excitationGenerator.xsd
   :language: xml
   :encoding: latin-1
