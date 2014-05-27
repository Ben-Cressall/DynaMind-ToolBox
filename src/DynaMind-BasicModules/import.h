/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @author  Markus Sengthaler <m.sengthaler@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012-2015  Christian Urich, Markus Sengthaler

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

#ifndef IMPORT_H
#define IMPORT_H
#include <dmmodule.h>
#include <dm.h>
#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include <QHash>
#include <QString>
#include "simplecrypt.h"


using namespace DM;

typedef std::map<std::string, std::string> StringMap;

class DM_HELPER_DLL_EXPORT Import : public Module
{
	DM_DECLARE_NODE(Import)

public:
	// parameters
	std::string FileName;

	std::string WFSServer;
	std::string WFSUsername;
	std::string WFSPassword;

	int		epsgcode;

	bool	append;
	bool	linkWithExistingView;
	bool	flip_wfs;

	double	offsetX;
	double	offsetY;

private:
	// backup states
	bool		append_old;
	std::string FileName_old;
	std::string ViewName_old;
	std::string WFSServer_old;
	std::string WFSUsername_old;
	std::string WFSPassword_old;

	// internal settings
	double	devider;
	double	tol;

	QHash<QString, std::vector<DM::Node* > > nodeList;

	enum DRIVERTYPE {
		DataError,
		ShapeFile,
		WFS,
		RasterData,
	}driverType;

public:
	// public for ui
	void initViews();

	void reloadFile();

	void updatePorts();

	SimpleCrypt crypto;
private:
	// main methods
	static bool ExtractLayers(OGRDataSource* dataSource, StringMap& viewConfig, 
		std::map<std::string, int>& viewConfigTypes, StringMap& viewEPSGConfig,
		int targetEPSG);

	static bool ExtractLayers(GDALDataset* dataSource, StringMap& newViewConfig, 
		std::map<std::string, int>& newViewConfigTypes);

	void adoptViewConfig(StringMap& newViewConfig, std::map<std::string, int>& newViewConfigTypes);

	void loadVectorData(const std::string& path);

	bool loadRasterData();

	// OGR-geom. loading methods
	void loadLayer(OGRLayer* layer, System* sys);

	void loadPoint(System *sys, OGRPoint *point, OGRCoordinateTransformation *poCT,
		OGRFeatureDefn* featureDef, OGRFeature* curFeature, const DM::View& view);

	void loadLineString(System *sys, OGRLineString *lineString, OGRCoordinateTransformation *poCT,
		OGRFeatureDefn* featureDef, OGRFeature* curFeature, const DM::View& view);

	void loadPolygon(System *sys, OGRPolygon *polygon, OGRCoordinateTransformation *poCT,
		OGRFeatureDefn* featureDef, OGRFeature* curFeature, const DM::View& view);

	// helper methods
	DM::Node* addNode(DM::System* sys, double x, double y, OGRCoordinateTransformation *poCT,
		const View* view = NULL);

	std::vector<Node*> addFaceNodes(System* sys, const OGRLineString *ring,
		OGRCoordinateTransformation *poCT);

	void appendAttributes(DM::Component * cmp, OGRFeatureDefn *poFDefn, OGRFeature *poFeature, const View& view);

	// geo helpers
	void initPointList(DM::System * sys);

	QString createHash(double x, double y);

	bool transform(double *x, double *y, OGRCoordinateTransformation *poCT);

	std::string getServerPath();
public:
	// parameter map: layername[.attribute], new-view-name
	StringMap viewConfig;

	// layer[.attribute], type; integer to combine attributes and views
	std::map<std::string, int> viewConfigTypes;

	// layername, epsg (as string, for param.)
	StringMap viewEPSGConfig;
public:
	void run();
	void init();
	void reset();
	virtual bool  createInputDialog();
	string getHelpUrl();
	Import();
	~Import();
};

#endif // IMPORT_H
