#include "gdalpublishresults.h"

#include "ogr_geometry.h"
#include <dmsimulation.h>

#include <ogrsf_frmts.h>
#include <ogr_api.h>


#include <dmgroup.h>
#include <sstream>

DM_DECLARE_CUSTOM_NODE_NAME(GDALPublishResults, Export GIS Data , Data Import and Export)

GDALPublishResults::GDALPublishResults()
{
	GDALModule = true;
	OGRRegisterAll();

	this->sink = "";
	this->addParameter("sink", DM::STRING, &sink);

	this->driverName = "";
	this->addParameter("driver_name", DM::STRING, &driverName);
	this->viewName = "";
	this->addParameter("view_name", DM::STRING, &viewName);

	this->layerName = "";
	this->addParameter("layer_name", DM::STRING, &layerName);

	this->steps = 1;
	this->addParameter("steps", DM::INT, &steps);

	this->targetEPSG = 0;
	this->addParameter("targetEPSG", DM::INT, &targetEPSG);

	this->overwrite = false;
	this->addParameter("overwrite", DM::BOOL, &overwrite);

	this->append = false;
	this->addParameter("append", DM::BOOL, &append);


	this->scenario_id_from_city = false;
	this->addParameter("scenario_id_as_prefix", DM::BOOL, &this->scenario_id_from_city);

	DM::ViewContainer v("dummy", DM::SUBSYSTEM, DM::MODIFY);


	std::vector<DM::ViewContainer> datastream;
	datastream.push_back(v);
	this->addGDALData("city", datastream);
}

void GDALPublishResults::init()
{
	if (this->viewName.empty()) {
		return;
	}

	std::map<std::string, DM::View> inViews = getViewsInStream()["city"];
	if (inViews.find(viewName) == inViews.end()) {
		DM::Logger(DM::Error) << "view " << viewName << " not found in in stream";
		return;
	}
	this->components = DM::ViewContainer(viewName, inViews[viewName].getType(), DM::READ);
	this->dummy = DM::ViewContainer(viewName, inViews[viewName].getType(), DM::MODIFY);


	std::vector<DM::ViewContainer *> data_stream;

	data_stream.push_back(&components);
	data_stream.push_back(&dummy);

	if (this->scenario_id_from_city) {
		if (inViews.find("city") == inViews.end()) {
			DM::Logger(DM::Error) << "view " << "city" << " not found in in stream";
			return;
		}
		this->city = DM::ViewContainer("city", inViews["city"].getType(), DM::READ);
		this->city.addAttribute("scenario_id", DM::Attribute::INT, DM::READ);
		data_stream.push_back(&this->city);
	}

	this->registerViewContainers(data_stream);
}

void GDALPublishResults::updateAttributes(OGRLayer* lyr)
{
	foreach(std::string attribute_name, components.getAllAttributes()) {
		//Feature already in layer
		if (lyr->GetLayerDefn()->GetFieldIndex(attribute_name.c_str()) >= 0)
			continue;
		if (components.getAttributeType(attribute_name) == DM::Attribute::INT){
			OGRFieldDefn oField ( attribute_name.c_str(), OFTInteger );
			lyr->CreateField(&oField);
			continue;
		}
		if (components.getAttributeType(attribute_name) == DM::Attribute::STRING){
			OGRFieldDefn oField ( attribute_name.c_str(), OFTString );
			lyr->CreateField(&oField);
			continue;
		}
		if (components.getAttributeType(attribute_name) == DM::Attribute::DOUBLE){
			OGRFieldDefn oField ( attribute_name.c_str(), OFTReal );
			lyr->CreateField(&oField);
			continue;
		}
		if (components.getAttributeType(attribute_name) == DM::Attribute::STRINGVECTOR){
			OGRFieldDefn oField ( attribute_name.c_str(), OFTStringList );
			lyr->CreateField(&oField);
			DM::Logger(DM::Error) << "Attribute typer STRINGVECTOR is currently not supported";
			continue;
		}
		if (components.getAttributeType(attribute_name) == DM::Attribute::DOUBLEVECTOR){
			OGRFieldDefn oField ( attribute_name.c_str(), OFTString );
			lyr->CreateField(&oField);
			continue;
		}
		if (components.getAttributeType(attribute_name) == DM::Attribute::DATE){
			OGRFieldDefn oField ( attribute_name.c_str(), OFTDate );
			lyr->CreateField(&oField);
			continue;
		}
		if (components.getAttributeType(attribute_name) == DM::Attribute::LINK){
			OGRFieldDefn oField ( attribute_name.c_str(), OFTInteger );
			lyr->CreateField(&oField);
			continue;
		}
	}
}

OGRLayer * GDALPublishResults::openLayer(GDALDataset *poDS, OGRSpatialReference* oTargetSRS, char** options, std::string layer_name)
{
	OGRLayer * lyr = NULL;

	if (append && poDS->GetLayerByName(layer_name.c_str())){
		// Append Layer

		lyr = poDS->GetLayerByName(layer_name.c_str());
		updateAttributes(lyr);
		return lyr;
	}

	switch ( components.getType() ) {
	case DM::COMPONENT:
		lyr = poDS->CreateLayer(layer_name.c_str(), oTargetSRS, wkbNone, options );
		break;
	case DM::NODE:
		lyr = poDS->CreateLayer(layer_name.c_str(), oTargetSRS, wkbPoint, options );
		break;
	case DM::EDGE:
		lyr = poDS->CreateLayer(layer_name.c_str(), oTargetSRS, wkbLineString, options );
		break;
	case DM::FACE:
		lyr = poDS->CreateLayer(layer_name.c_str(), oTargetSRS, wkbPolygon, options );
		break;
	}

	return lyr;
}

void GDALPublishResults::run()
{
	//Init Data Source
	GDALDriver *poDriver;
	GDALDataset *poDS;

	DM::Group* lg = dynamic_cast<DM::Group*>(getOwner());
	int interal_counter = -1;
	if(lg) {
		interal_counter = lg->getGroupCounter();
	}
	DM::Logger(DM::Debug) << interal_counter;

	if (interal_counter % this->steps != 0 && interal_counter != -1) {
		return;
	}


	DM::Logger(DM::Debug) << "Start Inster";

	char ** options = NULL;
	options = CSLSetNameValue( options, "PG_USE_COPY", "YES" );

	//Set Overwrite
	if (overwrite)
		options = CSLSetNameValue( options, "OVERWRITE", "YES" );

	CPLSetConfigOption("PG_USE_COPY", "YES");
	OGRRegisterAll();

	//Extend string_stream with _xx;
	std::stringstream sink_name;

	if (!append) {
		if (!driverName.empty()) {
			int pos = this->sink.find(".");
			if (pos == -1) {
				sink_name << sink;
				sink_name << "_" << interal_counter;
			} else {
				sink_name<< sink.substr(0,pos);
				sink_name << "_" << interal_counter;
				sink_name<< sink.substr(pos, sink.size()-1);
			}
		}

	} else {
		sink_name << sink;
	}

	if (!driverName.empty()) {
		poDriver  = (GDALDriver*) GDALGetDriverByName(this->driverName.c_str() );// OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName( this->driverName.c_str() );
		if( poDriver == NULL )
		{
			DM::Logger(DM::Error) << this->driverName << "driver not available.";
			this->setStatus(DM::MOD_EXECUTION_ERROR);
			return;
		}
		DM::Logger(DM::Error) << "create " << sink_name.str().c_str();
		poDS = poDriver->Create( sink_name.str().c_str(), 0, 0, 0, GDT_Unknown, NULL );
		// poDS = poDriver->CreateDataSource(sink_name.str().c_str());
	} else {
		poDS = (GDALDataset*) GDALOpenEx( sink.c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, NULL, NULL, NULL );
		//poDS = OGRSFDriverRegistrar::Open(sink.c_str(), true);
	}
	if( poDS == NULL )
	{
		DM::Logger(DM::Error) <<  "problem creating source";
		this->setStatus(DM::MOD_EXECUTION_ERROR);
		return;
	}


	OGRSpatialReference* oSourceSRS;
	OGRSpatialReference* oTargetSRS;
	oSourceSRS = new OGRSpatialReference();
	oSourceSRS->importFromEPSG(this->getSimulation()->getSimulationConfig().getCoorindateSystem());

	oTargetSRS = new OGRSpatialReference();
	if (targetEPSG == 0)
		oTargetSRS->importFromEPSG(this->getSimulation()->getSimulationConfig().getCoorindateSystem());
	else
			oTargetSRS->importFromEPSG(targetEPSG);
	OGRCoordinateTransformation* trans = OGRCreateCoordinateTransformation(oSourceSRS, oTargetSRS);
	if (!trans) {
		DM::Logger(DM::Error) << "Init transformation failed, check EPSG codes";
		return;
	}
	std::stringstream layer_name;

	int scenario_id = -1;
	if(this->scenario_id_from_city) {
		this->city.resetReading();
		OGRFeature * f;
		while (f = this->city.getNextFeature()) {
			scenario_id = f->GetFieldAsInteger("scenario_id");
		}
		if(scenario_id > -1) {
			layer_name << scenario_id << "_";
		}
	}
	DM::Logger(DM::Standard) << "layer name "<< layer_name.str();

	layer_name << this->layerName;
	if (!append) {
		if (interal_counter != -1) {
			 layer_name << "_" << interal_counter;
		}
	}

	OGRLayer* lyr = openLayer(poDS, oTargetSRS, options, layer_name.str());

	if (!lyr) {
		DM::Logger(DM::Error) << "Layer not created " << components.getType();
		this->setStatus(DM::MOD_EXECUTION_ERROR);
		return;
	}

	OGRFeatureDefn * def =  (OGRFeatureDefn*)components.getFeatureDef();
	int c_fields = def->GetFieldCount();
	for (int i = 0; i < c_fields; i++)
		lyr->CreateField(def->GetFieldDefn(i));
	OGRFeature * feat;
	components.resetReading();
	int counter = 0;
	lyr->StartTransaction();
	while (feat = components.getNextFeature()) {
		counter++;
		if (counter % 1000000 == 0){
			lyr->CommitTransaction();
			lyr->StartTransaction();
		}
		OGRFeature * f_new = OGRFeature::CreateFeature( lyr->GetLayerDefn() );
		for (int i = 0; i < c_fields; i++)
			f_new->SetField(i, feat->GetRawFieldRef(i));
		if (components.getType() != DM::COMPONENT) {
			OGRGeometry * geo = feat->GetGeometryRef();
            if (trans && geo != 0)
				geo->transform(trans);
			f_new->SetGeometry(geo);
		}
		lyr->CreateFeature(f_new);

		OGRFeature::DestroyFeature(f_new);
	}
	DM::Logger(DM::Standard) << "published features " << counter;
	lyr->CommitTransaction();
	GDALClose(poDS);
}




std::string GDALPublishResults::getHelpUrl()
{
	return "/DynaMind-GDALModules/gdalpublishresults.html";
}
