Standard Modules
================

A range of standard modules that can be used for versatile use cases.


Data Import and Export
----------------------
- :doc:`GDALHotStarter </DynaMind-GDALModules/gdalhotstarter>` Hot start simulation
- :doc:`GDALImportData </DynaMind-GDALModules/gdalimportdata>` Import data
- :doc:`GDALPublishResults </DynaMind-GDALModules/gdalpublishresults>` Export GIS data

Linking
-------
- :doc:`GDALSpatialLinking </DynaMind-GDALModules/gdalspatiallinking>` Spatially link data
- :doc:`NonSpatialLinking </DynaMind-GDALModules/nonspatiallinking>` Link data sets with identifier
- :doc:`GDALJoinNearestNeighbour </DynaMind-GDALModules/gdaljoinnearestneighbour>` Link nearest neighbour
- :doc:`GDALCreateNeighbourhoodTable </DynaMind-GDALModules/gdalcreateneighbourhoodtable>` Link Adjacent Faces

Data Handling
-------------

- :doc:`GDALCopyElementToView </DynaMind-GDALModules/gdalcopyelementtoview>` Copy features
- :doc:`GDALRemoveComponets </DynaMind-GDALModules/gdalremovecomponents>` Delete components
- :doc:`GDALAttributeCalculator </DynaMind-GDALModules/gdalattributecalculator>` Calculate attributes
- :doc:`GDALGeometricAttributes </DynaMind-GDALModules/gdalgeometricattributes>` Calculate area
- :doc:`GDALCalculateLength </DynaMind-GDALModules/gdalcalculatelength>` Calculate length
- :doc:`DM_CalculateDistance </DynaMind-GDALModules/dm_calculatedistance>` Calculate distance

Geometry Processing
-------------------

- :doc:`GDALCreateCentroids </DynaMind-GDALModules/gdalcalculatecentroids>` Create centroids
- :doc:`GDALErase </DynaMind-GDALModules/gdalerase>` Subtract features
- :doc:`GDALOffset </DynaMind-GDALModules/gdaloffset>` Offset faces
- :doc:`GDALParceling </DynaMind-GDALModules/gdalparceling>` Parcel faces
- :doc:`GDALParcelSplit </DynaMind-GDALModules/gdalparcelsplit>` Subdivide parcel
- :doc:`GDALCreateBuilding </DynaMind-GDALModules/gdalcreatebuilding>` Generate simple buildings

Groups
------

- :doc:`LoopGroup </DynaMind-GDALModules/loopgroup>` Repeatedly execute modules in the group to iteratively manipulate data


Infrastructure Networks
=======================

A range of modules to help with data cleaning to prepare and analysis urban infrastructure networks. E.g. modules to integrate urban drainage networks into SWMM.

Data Preparation
----------------

- :doc:`GDALExtractNodes </DynaMind-GDALModules/gdalextractnodes>` Creates start and end nodes for a given drainage network
- :doc:`GDALClusterNetwork </DynaMind-GDALModules/gdalclusternetwork>` Identify clusters of connected lines
- :doc:`DM_DirectGraph </DynaMind-GDALModules/dm_direct_graph>` Direct graph
- :doc:`DM_ValueFromRaster</DynaMind-GDALModules/dm_value_from_raster>` Copy raster data values onto nodes

Network Analysis
----------------
- :doc:`DM_Strahler </DynaMind-GDALModules/dm_strahler>` Strahler ordering
- :doc:`Segmentation </DynaMind-GDALModules/segmentation>` Create segments of equal attributes
- :doc:`Dual Mapping </DynaMind-GDALModules/dualmapping>` Export dual mapped edge list

SWMM
----

- :doc:`GDALDMSWMM </DynaMind-GDALModules/gdaldmswmm>` SWMM integration