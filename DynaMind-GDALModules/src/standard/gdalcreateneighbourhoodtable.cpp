#include "gdalcreateneighbourhoodtable.h"
#include <dmviewcontainer.h>
#include <dmview.h>
#include <dmgdalsystem.h>
#include <dm.h>
#include <sstream>
#include <ogrsf_frmts.h>

DM_DECLARE_CUSTOM_NODE_NAME(GDALCreateNeighbourhoodTable, Link Adjacent Faces, Linking)

GDALCreateNeighbourhoodTable::GDALCreateNeighbourhoodTable()
{
	GDALModule = true;


	this->viewName = "";
	this->addParameter("view_name", DM::STRING, &viewName);

	this->neighName = "";
	this->addParameter("neigh_name", DM::STRING, &neighName);

	this->onlyShareEdge = false;
	this->addParameter("only_share_edge", DM::BOOL, &onlyShareEdge);

	//dummy to get the ports
	std::vector<DM::ViewContainer> data;
	data.push_back(  DM::ViewContainer ("dummy", DM::SUBSYSTEM, DM::MODIFY) );
	this->addGDALData("city", data);
}

void GDALCreateNeighbourhoodTable::init()
{
	if (this->viewName.empty() || this->neighName.empty()) {
		return;
	}

	std::map<std::string, DM::View> inViews = getViewsInStream()["city"];
	if (inViews.find(viewName) == inViews.end()) {
		DM::Logger(DM::Warning) << "view " << viewName << " not found in in stream";
		return;
	}

	name_id1 = this->viewName + "_id1";
	name_id2 = this->viewName + "_id2";

	leadingView = DM::ViewContainer(this->viewName, DM::FACE, DM::READ);
	ngView = DM::ViewContainer(this->neighName, DM::COMPONENT, DM::WRITE);

	ngView.addAttribute(name_id1, this->viewName, DM::WRITE);
	ngView.addAttribute(name_id2, this->viewName, DM::WRITE);

	std::vector<DM::ViewContainer*> data;
	data.push_back( &leadingView);
	data.push_back( &ngView);
	this->registerViewContainers(data);

}

void GDALCreateNeighbourhoodTable::run()
{
	leadingView.createSpatialIndex();
	DM::GDALSystem * sys = this->getGDALData("city");

	std::stringstream query;

	if (!onlyShareEdge) {
		query << "SELECT p1.OGC_FID as \"" << name_id1 << "\", p2.OGC_FID as \"" << name_id2 << "\" FROM " <<  viewName <<" as p1, " << viewName;
		query << " as p2 WHERE ST_Touches(p1.geometry, p2.geometry) AND  p2.ROWID IN ( SELECT ROWID FROM SpatialIndex WHERE f_table_name ='";
		query << viewName << "' AND search_frame = p2.Geometry ORDER BY ROWID)";
	} else {
		query << "SELECT p1.OGC_FID as \"" << name_id1 << "\", p2.OGC_FID as \"" << name_id2 << "\" FROM " <<  viewName <<" as p1, " << viewName;
		query << " as p2 WHERE ST_Touches(p1.geometry, p2.geometry) AND GeometryType(Intersection(p1.geometry, p2.geometry)) != 'POINT'  AND  p2.ROWID IN "
				 "( SELECT ROWID FROM SpatialIndex WHERE f_table_name ='";
		query << viewName << "' AND search_frame = p2.Geometry ORDER BY ROWID)";
	}


	DM::Logger(DM::Standard) << query.str();

	OGRLayer * l = sys->getDataSource()->ExecuteSQL(query.str().c_str(), 0, "SQLITE");

	if (!l) {
		DM::Logger(DM::Error) << "Error in sql query";
		return;
	}
	OGRFeature * f;
	while (f = l->GetNextFeature()) {
		int id1 = f->GetFieldAsInteger(name_id1.c_str());
		int id2 = f->GetFieldAsInteger(name_id2.c_str());

		OGRFeature * f_l = ngView.createFeature();
		f_l->SetField(name_id1.c_str(), id1);
		f_l->SetField(name_id2.c_str(), id2);

		OGRFeature::DestroyFeature(f);

	}
	sys->getDataSource()->ReleaseResultSet(l);
}

string GDALCreateNeighbourhoodTable::getHelpUrl()
{
	return "/DynaMind-GDALModules/gdalcreateneighbourhoodtable.html";
}

