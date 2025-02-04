[![Build Status](https://travis-ci.org/iut-ibk/DynaMind-ToolBox.svg?branch=master)](https://travis-ci.org/iut-ibk/DynaMind-ToolBox)
DynaMind-ToolBox 
================

DynaMind is an open-source GIS modelling toolbox similar to the ArcGIS - model builder. The idea is that small reusable modules can be linked together to describe a complex workflow in an urban environment. E.g. the evolution of a small city

<a href="http://htmlpreview.github.io/?https://github.com/christianurich/DynaMind-Analysis/blob/master/viewer/viewer.html"><img src="https://github.com/christianurich/DynaMind-Analysis/blob/master/viewer.png?raw=true" border="0"></a> 

The heart of the DynaMind-Toolbox is [DynaMind](https://github.com/iut-ibk/DynaMind) a small efficient core written in C++. The core provides easy to use interfaces to develop new modules in C++ or Python and to access the data in the data stream. 

The DyanMind-Toolbox comes with:


- a model build to set up and edit simulations

<img src="https://github.com/iut-ibk/DynaMind-Gui/blob/master/doc/images/gui_with_view.png?raw=true" border="0">

- a set of modules to import, edit, create and export data
- an easy to use Python interface to create new simulation or build new modules.

## [Documentation](http://iut-ibk.github.io/DynaMind-ToolBox)
  - [Overview](http://iut-ibk.github.io/DynaMind-ToolBox/introduction.html)
  - [Modules](http://iut-ibk.github.io/DynaMind-ToolBox/DynaMind-GDALModules/index.html)
  - [Development](http://iut-ibk.github.io/DynaMind-ToolBox/DynaMind/index.html)

## Windows Installer
- [DynaMind-Toolbox v0.12.11](https://www.dropbox.com/s/hvugj26ogxd1hey/DynaMind-Toolbox-0.12.11-win32.exe?dl=0)

## Install on Mac using the Dynamind [tap](https://github.com/christianurich/homebrew-dynamind)

```
brew tap christianurich/dynamind
brew install dynamind --HEAD
```
To run dynamind start it form the comandline and export the path to python-gdal2

```
export PYTHONPATH=/usr/local/opt/gdal2-python/lib/python2.7/site-packages/:${PYTHONPATH}
dynamind-gui
```

