/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2014  Christian Urich

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef IMPORTDATA_H
#define IMPORTDATA_H

#include <dmmodule.h>
#include <dm.h>
#include <dmviewcontainer.h>
#include <ogrsf_frmts.h>



class DM_HELPER_DLL_EXPORT GDALImportData: public DM::Module
{
	DM_DECLARE_NODE(GDALImportData)
private:
	DM::ViewContainer components;

	std::string source;
	std::string layername;
	std::string viewName;
	std::string driver_name;
	std::map<std::string, std::string> import_attribute_as;
	bool append;
	DM::ViewContainer *initShapefile();

	DM::ViewContainer *filterView;

	DM::ViewContainer *vc;

	DM::ViewContainer dummy;

	GDALDataset *poDS;

	OGRLayer *initLayer();

	bool checkIsFlat(int ogrFlat);

	int epsg_from; //Needed to set spatial filter


public:
	GDALImportData();
	void run();
	void init();
    void preRun();
	virtual ~GDALImportData();
	OGRCoordinateTransformation *getTrafo(int sourceEPSG, int targetEPSG);

	std::map<std::string, std::string> translator;

	std::string getHelpUrl();
};

#endif // IMPORTDATA_H
