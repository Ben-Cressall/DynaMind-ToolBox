=========
GDALErase
=========

Intersects features from the *eraseViewName* with features from *leadingViewName* and removes intersecting faces from the
*leadingViewName*

Parameter
---------

+-------------------+------------------------+----------------------------------------------------------------------------------------------+
|        Name       |          Type          |       Description                                                                            |
+===================+========================+==============================================================================================+
|leadingViewName    | STRING (*FACE*)        | Name of view that is intersected with the eraseViewNames and interecting featers are removed.|
+-------------------+------------------------+----------------------------------------------------------------------------------------------+
|eraseViewName      | STRING (*FACE*)        | Name of view that provides the mask that is deleted from the *leadingViewName*.              |
+-------------------+------------------------+----------------------------------------------------------------------------------------------+






