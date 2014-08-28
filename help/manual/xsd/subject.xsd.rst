Subject description file
###############################

.. only:: html or latex

   .. contents::


XML example
------------------------
.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <subject xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="subject.xsd">
      <mtuDefault>
            ...
      </mtuDefault>
      <mtuSet>
            ...
      </mtuSet>
      <dofSet>
            ...
      </dofSet>
      <calibrationInfo>
            ...
      </calibrationInfo>
    </subject>

A subject description file consists of a root element named ``subject`` that contains the following elements:

.. todo::

    TO BE COMPLETED

mtuDefault
==========

mtuSet
======

dofSet
======

calibrationInfo
===============

XSD grammar
------------------------

.. literalinclude:: ../../../cfg/XSD/subject.xsd
   :language: xml
   :encoding: latin-1
