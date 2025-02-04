=====
Mixer
=====

:index:`Mixer <CD3Node; Mixer | Combining streams>` combines two or more.
Depending on the num_inputs parameter n inports are created.
The ports are number from 0 to n-1.

Flow

.. math::

    f_{out,0}= \sum f_{in,0}

..

Concentrations

.. math::

    f_{out,i} =  \frac{\sum f_{in,i} \cdot  f_{in,0} } { \sum f_{in,0}} \\
    i = 1 \dotsc n

..


Parameter
---------

+-----------------------+------------------------+------------------------------------------------------------------------+
|        Name           |          Type          |       Description                                                      |
+=======================+========================+========================================================================+
| num_inputs            | Integer                | Number of in ports default is 2                                        |
+-----------------------+------------------------+------------------------------------------------------------------------+


In and Out
----------

+--------------------+------------+----------------+
|        Name        | Type       |   Description  |
+====================+============+================+
| in_0               | in         |                |
+--------------------+------------+----------------+
| in_1               | in         |                |
+--------------------+------------+----------------+
| ...                | in         |                |
+--------------------+------------+----------------+
| in_n-1             | in         |                |
+--------------------+------------+----------------+
| out                | out        |                |
+--------------------+------------+----------------+


