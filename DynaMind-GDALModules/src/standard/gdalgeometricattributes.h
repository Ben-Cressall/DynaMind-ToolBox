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

#ifndef GDALGEOMETRICATTRIBUTES_H
#define GDALGEOMETRICATTRIBUTES_H

#include <dmmodule.h>
#include <dm.h>

class DM_HELPER_DLL_EXPORT OGRPolygon;

class DM_HELPER_DLL_EXPORT GDALGeometricAttributes : public DM::Module
{
	DM_DECLARE_NODE(GDALGeometricAttributes)
private:
	std::string leadingViewName;
	bool isCalculateArea;
	bool isAspectRationBB;
	bool isPercentageFilled;
	bool useSQL;

	DM::ViewContainer vc;

public:
	GDALGeometricAttributes();
	void run();
	void init();
	void run_sql();
	std::string getHelpUrl();
	void run_sql_threaded();
};

#endif // GDALGEOMETRICATTRIBUTES_H
