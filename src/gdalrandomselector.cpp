#include "gdalrandomselector.h"

DM_DECLARE_NODE_NAME(GDALRandomSelector, GDALModules)

GDALRandomSelector::GDALRandomSelector()
{
	GDALModule = true;

	srand (time(NULL));

	this->viewName = "";
	this->addParameter("view_name", DM::STRING, &this->viewName);

	this->attribute = "";
	this->addParameter("attribute", DM::STRING, &this->attribute);

	this->elements = 0;
	this->addParameter("elements", DM::INT, &this->elements);

	//dummy to get the ports
	std::vector<DM::ViewContainer> data;
	data.push_back(  DM::ViewContainer ("dummy", DM::SUBSYSTEM, DM::MODIFY) );
	this->addGDALData("city", data);

}

void GDALRandomSelector::init()
{
	initFailed = true;
	if (viewName.empty() || attribute.empty())
		return;
	this->vc = DM::ViewContainer(viewName, DM::FACE, DM::READ);
	this->vc.addAttribute(attribute, DM::Attribute::INT, DM::WRITE);

	std::vector<DM::ViewContainer*> data_stream;
	data_stream.push_back(&vc);
	this->registerViewContainers(data_stream);
	initFailed = false;
}

void GDALRandomSelector::run()
{

	if (initFailed) {
		DM::Logger(DM::Error) << "Init Failed";
		this->setStatus(DM::MOD_EXECUTION_ERROR);
		return;
	}
	vc.resetReading();
	int total_number_of_featurers = vc.getFeatureCount();
	if (total_number_of_featurers == -1) {
		DM::Logger(DM::Error) << "Error feature counter";
		return;
	}
	std::vector<int> rand_elements;
	std::vector<int> ids;
	int elements_max = (elements < total_number_of_featurers) ? elements : total_number_of_featurers;

	OGRFeature * f;

	DM::Logger(DM::Error) << elements_max << " "<< total_number_of_featurers;

	int counter = 0;
	while(f = vc.getNextFeature()) {
		rand_elements.push_back(counter++);
		ids.push_back(f->GetFID());
	}
	vc.syncAlteredFeatures();
	vc.syncReadFeatures();

	vc.resetReading();
	for (int i = 0; i < elements_max; i++) {
		int r = rand() %total_number_of_featurers;
		rand_elements[i] = rand_elements[r];
	}
	for (int i = 0; i < elements; i++) {
		f = vc.getFeature(ids[rand_elements[i]]);
		f->SetField(attribute.c_str(), 1);
	}

}


