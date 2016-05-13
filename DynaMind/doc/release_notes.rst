=============
Release Notes
=============

Release Notes v0.11
-------------------

Core
____

- Change form GDAL 1.xx to GDAL 2.0
- New handing of DOUBLEVECTOR. This

Modules
_______

- Bug and stability fixes





Release Notes v0.10
-------------------

Upgrade Information
-------------------
- changes in the hotstart database component require and rerun of the simulation to create a new simulation database

Core
____

- Bug and stability fixes
- Simulation database structure is now saved in database

Modules
_______

- Bug and stability fixes



Release Notes v0.9
------------------

Modules
_______

- Threading for
  - Parceling
  - Geometric Attributes

- New module  :doc:`SQL calculator </DynaMind-GDALModules/dm_sqlcalculator>`

  - DynaMind SQLite extensions

- :doc:`Attribute Calculator </DynaMind-GDALModules/gdalattributecalculator>`

  - Enabling vector arithmetic

- :doc:`Export GIS data </DynaMind-GDALModules/gdalpublishresults>`

  - Append data to existing layer

- New module to :doc:`generate grids </DynaMind-GDALModules/dm_creategrid>`

- New module to :doc:`add a single component to a simulation </DynaMind-GDALModules/dm_createcomponent>`

- DAnCE platform integration



Release Notes v0.8
------------------

Core
____

- Increased OGC conformity
- Proper linking of tables
- Improved documentation
- Improved installation (proper directories) and OSX build with homebrew
- Additional Simulation parameter to set epsg codes, working directories

Model Builder
_____________

- export of simulation database directly from GUI
- improved model view
- start screen
- improved model tree and better naming
- proper help integration

Modules
_______

- new modules to calculate length :doc:`GDALCalculateLength </DynaMind-GDALModules/gdalcalculatelength>`
- speed improvement for spatial linking (10 times and more) using native spatialite
- lots of bug fixes and stability
- doc, doc, doc

Release Notes v0.7
------------------

- Integration of data base backend enabling big simulation. Note: Modules using the GDAL based data stream are not compatible with older modules
- Support for spatial and attribute filters for modules [1]_
- Python wrapper for simulation class
- And lots of bug fixes
- Initial set of modules working with the new GDAL data stream backend


.. [1] only for modules using the GDAL data stream