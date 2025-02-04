======================
Urban Metabolism Model
======================

Overview
========

wb_lot_streams combines the internal streams to a global stream as outflow on the node.


Internal streams on lot
-----------------------

+-------------------------+------------------------+------------------------------------------------------------------------+
|        Stream           |          ID            |       Description                                                      |
+=========================+========================+========================================================================+
| potable_demand          |       1                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| non_potable_demand      |       2                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| outdoor_demand          |       3                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| black_water             |       4                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| grey_water              |       5                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| roof_runoff             |       6                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| impervious_runoff       |       7                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| pervious_runoff         |       8                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| evapotranspiration      |       9                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| infiltration            |       10               |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+

Global streams
--------------

+-------------------------+------------------------+------------------------------------------------------------------------+
|        Stream           |          ID            |       Description                                                      |
+=========================+========================+========================================================================+
| potable_demand          |       1                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| non_potable_demand      |       2                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| outdoor_demand          |       3                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| sewerage                |       4                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| grey_water              |       5                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| stormwater_runoff       |       6                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| evapotranspiration      |       7                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+
| infiltration            |       8                |                                                                        |
+-------------------------+------------------------+------------------------------------------------------------------------+


Data-stream
-----------

+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|        View         |          Attribute       |       Type                  |Access |    Description                           |
+=====================+==========================+=============================+=======+==========================================+
| wb_lot_streams      |                          | COMPONENT                   | read  |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | wb_lot_template_id       | LINK                        | read  | Link to lot template                     |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | outflow_stream_id        | INT                         | read  | global stream id                         |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | lot_stream_id            | INT                         | read  | internal stream                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     |                          |                             |       |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
| wb_storages         |                          | COMPONENT                   | read  |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | wb_lot_template_id       |                             | read  |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | inflow_stream_id         |                             | read  |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | demand_stream_id         |                             | read  |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | volume                   |                             | read  |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     |                          |                             |       |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
| wb_sub_catchment    |                          | COMPONENT                   | read  |                                          |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | stream                   | INT                         | read  | Stream ID                                |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | annual_flow              | DOUBLE                      | write | Annual flow                              |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+
|                     | daily_flow               | DOUBLEVECTOR                | write | Daily flow                               |
+---------------------+--------------------------+-----------------------------+-------+------------------------------------------+