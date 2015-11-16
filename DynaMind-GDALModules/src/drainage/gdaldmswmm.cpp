/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011-2014 Christian Urich

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
#include "gdaldmswmm.h"
#include "dmgdalhelper.h"

#include <fstream>

#include <QDir>
#include <QUuid>
#include <QProcess>
#include <QTextStream>
#include <QSettings>
#include <math.h>
#include <algorithm>
#include "swmmwriteandread.h"
#include "swmmreturnperiod.h"
#include "drainagehelper.h"
#include "ogrsf_frmts.h"


DM_DECLARE_CUSTOM_NODE_NAME(GDALDMSWMM, SWMM Integration, Performance Assessment)

using namespace DM;
GDALDMSWMM::GDALDMSWMM()
{
	GDALModule = true;

	GLOBAL_Counter = 1;
	internalTimestep = 0;


	this->FileName = "/tmp/swmm";
	this->climateChangeFactor = 1;
	this->RainFile = "";

	this->writeResultFile = false;
	this->climateChangeFactorFromCity = false;
	this->calculationTimestep = 1;
	this->deleteSWMM = true;
	this->rainfile_from_vector = "";
	years = 0;

	this->isCombined = false;
	this->addParameter("Folder", DM::STRING, &this->FileName);
	this->addParameter("RainFile", DM::FILENAME, &this->RainFile);
	this->addParameter("ClimateChangeFactor", DM::DOUBLE, & this->climateChangeFactor);
	exportSubCatchmentShape = false;
	this->addParameter("export_subcatchment_shape", DM::BOOL, &this->exportSubCatchmentShape);

	explictly_consider_WSUD = false;
	this->addParameter("explictly_consider_WSUD", DM::BOOL, &this->explictly_consider_WSUD);
	//this->addParameter("combined system", DM::BOOL, &this->isCombined);

	this->addParameter("rainfile_from_vector", DM::STRING, & this->rainfile_from_vector);

	this->addParameter("writeResultFile", DM::BOOL, &this->writeResultFile);
	this->addParameter("climateChangeFactorFromCity", DM::BOOL, &this->climateChangeFactorFromCity);
	this->addParameter("calculationTimestep", DM::INT, & this->calculationTimestep);
	this->addParameter("deleteSWMM", DM::BOOL, & this->deleteSWMM);

	counterRain = 0;

	unique_name = QUuid::createUuid().toString().toStdString();

}

void GDALDMSWMM::init() {
	hasWeir = false;
    hasRaintanks = false;
	conduit = DM::ViewContainer("conduit", DM::EDGE, DM::READ);
	conduit.addAttribute("start_id", "node", DM::READ);
	conduit.addAttribute("end_id", "node", DM::READ);
	conduit.addAttribute("diameter", DM::Attribute::DOUBLE, DM::READ);
	conduit.addAttribute("type", DM::Attribute::STRING, DM::READ);
	conduit.addAttribute("capacity", DM::Attribute::DOUBLE, DM::WRITE);
	conduit.addAttribute("velocity", DM::Attribute::DOUBLE, DM::WRITE);
	conduit.addAttribute("inlet_offset",  DM::Attribute::DOUBLE, DM::READ);
	conduit.addAttribute("outlet_offset",  DM::Attribute::DOUBLE, DM::READ);

	inlet = DM::ViewContainer("inlet", DM::NODE, DM::READ);
	inlet.addAttribute("node_id", "node", DM::READ);
	inlet.addAttribute("sub_catchment_id", "sub_catchment", DM::READ);

	junctions = DM::ViewContainer("junction", DM::NODE, DM::READ);
	junctions.addAttribute("node_id", "node", DM::READ);
	junctions.addAttribute("d", DM::Attribute::DOUBLE, DM::READ);
	junctions.addAttribute("invert_elevation", DM::Attribute::DOUBLE, DM::READ);

	junctions.addAttribute("flooding_volume", DM::Attribute::DOUBLE, DM::WRITE);
	junctions.addAttribute("node_depth", DM::Attribute::DOUBLE, DM::WRITE);

	catchment = DM::ViewContainer("sub_catchment", DM::FACE, DM::READ);
	catchment.addAttribute("area", DM::Attribute::DOUBLE, DM::READ);
	catchment.addAttribute("impervious_fraction", DM::Attribute::DOUBLE, DM::READ);

	outfalls= DM::ViewContainer("outfall", DM::NODE, DM::READ);
	outfalls.addAttribute("node_id", "node", DM::READ);
	outfalls.addAttribute("invert_elevation", DM::Attribute::DOUBLE, DM::READ);

	nodes= DM::ViewContainer("node", DM::NODE, DM::READ);

	city = DM::ViewContainer("city", DM::COMPONENT, DM::READ);
	city.addAttribute("SWMM_ID", DM::Attribute::STRING, DM::WRITE);
	city.addAttribute("v_p", DM::Attribute::DOUBLE, DM::WRITE);
	city.addAttribute("v_r", DM::Attribute::DOUBLE, DM::WRITE);
	city.addAttribute("v_wwtp", DM::Attribute::DOUBLE, DM::WRITE);
	city.addAttribute("v_outfall", DM::Attribute::DOUBLE, DM::WRITE);
	city.addAttribute("continuity_error", DM::Attribute::DOUBLE, DM::WRITE);
	city.addAttribute("average_capacity", DM::Attribute::DOUBLE, DM::WRITE);

	std::map<std::string, DM::View> inViews = getViewsInStream()["city"];
	if (inViews.find("weir") != inViews.end()) {
		this->hasWeir = true;
		weir = DM::ViewContainer("weir", DM::EDGE, DM::READ);
		weir.addAttribute("start_id", "node", DM::READ);
		weir.addAttribute("end_id", "node", DM::READ);
		// weir.addAttribute("diameter", DM::Attribute::DOUBLE, DM::READ);
		weir.addAttribute("crest_height", DM::Attribute::DOUBLE, DM::READ);
		weir.addAttribute("discharge_coefficient", DM::Attribute::DOUBLE, DM::READ);
		weir.addAttribute("end_coefficient", DM::Attribute::DOUBLE, DM::READ);
	}

	if (inViews.find("rwht") != inViews.end() && explictly_consider_WSUD) {
        this->hasRaintanks = true;
        rwhts = DM::ViewContainer("rwht", DM::COMPONENT, DM::READ);
        rwhts.addAttribute("sub_catchment_id", DM::Attribute::LINK, DM::READ);
        rwhts.addAttribute("volume", DM::Attribute::DOUBLE, DM::READ);
        rwhts.addAttribute("connected_area", DM::Attribute::DOUBLE, DM::READ);
    }



	if (this->climateChangeFactorFromCity)
		city.addAttribute("climate_change_factor",DM::Attribute::DOUBLE, DM::READ);

	if (!this->rainfile_from_vector.empty())
		city.addAttribute(this->rainfile_from_vector,DM::Attribute::DOUBLEVECTOR, DM::READ);

	std::vector<DM::ViewContainer*> data_stream;

	data_stream.push_back(&inlet);
	data_stream.push_back(&junctions);
	data_stream.push_back(&catchment);
	data_stream.push_back(&outfalls);
	data_stream.push_back(&conduit);
	data_stream.push_back(&nodes);
	if (this->hasWeir)
		data_stream.push_back(&weir);
    if (this->hasRaintanks)
        data_stream.push_back(&rwhts);
	data_stream.push_back(&city);


	data_map["inlet"] = &this->inlet;
	data_map["junction"] = &this->junctions;
	//data_map["endnode"] = &this->endnodes;
	data_map["catchment"] = &this->catchment;
	data_map["outfall"]  = &this->outfalls;
	data_map["conduit"]  = &this->conduit;
	data_map["node"]  = &this->nodes;
	if (this->hasWeir)
		data_map["weir"]  = &this->weir;
    if (this->hasRaintanks)
        data_map["rwht"]  = &this->rwhts;
	data_map["city"]  = &this->city;

	this->registerViewContainers(data_stream);
}

string GDALDMSWMM::getHelpUrl()
{
	return "/DynaMind-GDALModules/gdaldmswmm.html";
}


void GDALDMSWMM::run() {

	if (!QDir(QString::fromStdString(this->FileName)).exists()){
		DM::Logger(DM::Standard) <<  this->FileName << "  does not exist but created";
		QDir::current().mkpath(QString::fromStdString(this->FileName));
		//return;
	}

	double cf = this->climateChangeFactor;
	std::vector<double> rainvec;

	if (this->climateChangeFactorFromCity) {
		this->city.resetReading();
		OGRFeature * city_f;
		while(city_f = this->city.getNextFeature()){
			cf = city_f->GetFieldAsDouble("climate_change_factor");
			if (!this->rainfile_from_vector.empty())
				DM::DMFeature::GetDoubleList(city_f, this->rainfile_from_vector, rainvec);
		}
	}



	this->years++;

	if (this->internalTimestep == this->calculationTimestep)
		this->internalTimestep = 0;
	this->internalTimestep++;

	if (this->internalTimestep != 1) {
		return;
	}

	SWMMWriteAndRead * swmm;
	swmm = new SWMMWriteAndRead(data_map, this->RainFile, this->FileName);
		if (!this->rainfile_from_vector.empty())
			swmm->setRainVec(rainvec);
	swmm->setDeleteSWMMWhenDone(this->deleteSWMM);

	if (!this->rainfile_from_vector.empty())
		city.addAttribute(this->rainfile_from_vector,DM::Attribute::DOUBLEVECTOR, DM::READ);

	//swmm->setBuildYearConsidered(this->consider_built_time);
	swmm->setExportSubcatchmentShape(exportSubCatchmentShape);
	swmm->setClimateChangeFactor(cf);
	swmm->setupSWMM();
	swmm->runSWMM();
	swmm->readInReportFile();

	std::map<int, double> flooded_nodes = swmm->getFloodedNodes();
	std::map<int, double> node_depths = swmm->getNodeDepthSummery();

	OGRFeature * junction;
	junctions.resetReading();
	while (junction = junctions.getNextFeature()) {
		int id =junction->GetFieldAsInteger("node_id");
		if (flooded_nodes.find(id) != flooded_nodes.end()) {
			junction->SetField("flooding_volume", flooded_nodes[id]);
		}
		if (node_depths.find(id) != node_depths.end()) {
			junction->SetField("node_depth", node_depths[id]);
		}
	}

	std::map<int, double> flow_cap = swmm->getLinkFlowSummeryCapacity();
	std::map<int, double> flow_vel = swmm->getLinkFlowSummeryVelocity();

	OGRFeature * c;
	this->conduit.resetReading();
	while (c = conduit.getNextFeature()) {
		int id = c->GetFID();
		if (flow_cap.find(id) != flow_cap.end()) {
			c->SetField("capacity", flow_cap[id]);
		}
		if (flow_vel.find(id) != flow_vel.end()) {
			c->SetField("velocity", flow_vel[id]);
		}
	}

	DM::Logger(DM::Standard) << "Start write output files";

	delete swmm;
}
