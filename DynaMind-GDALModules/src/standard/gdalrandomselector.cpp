#include "gdalrandomselector.h"
#include <sstream>

DM_DECLARE_CUSTOM_NODE_NAME(GDALRandomSelector,Randomly Select Features, Data Handling)

GDALRandomSelector::GDALRandomSelector()
{
	GDALModule = true;

	valueFromView = false;

	srand (time(NULL));

	this->viewName = "";
	this->addParameter("view_name", DM::STRING, &this->viewName);

	this->attribute = "";
	this->addParameter("attribute", DM::STRING, &this->attribute);

	this->elements = 0;
	this->addParameter("elements", DM::INT, &this->elements);

	this->units = "";
	this->addParameter("units", DM::STRING, &this->units);

	this->viewNameFrom = "";
	this->addParameter("view_from", DM::STRING, &this->viewNameFrom);

	this->attributeNameFrom= "";
	this->addParameter("attribute_from", DM::STRING, &this->attributeNameFrom);

	//dummy to get the ports
	std::vector<DM::ViewContainer> data;
	data.push_back(  DM::ViewContainer ("dummy", DM::SUBSYSTEM, DM::MODIFY) );
	this->addGDALData("city", data);

}

void GDALRandomSelector::init()
{
	valueFromView = false;

	if (viewName.empty() || attribute.empty())
		return;
	this->vc = DM::ViewContainer(viewName, DM::FACE, DM::READ);
	this->vc.addAttribute(attribute, DM::Attribute::INT, DM::WRITE);
	if (!this->units.empty())
		vc.addAttribute(units, DM::Attribute::INT, DM::READ);

	std::vector<DM::ViewContainer*> data_stream;
	data_stream.push_back(&vc);

	if (!this->viewNameFrom.empty() && !this->attributeNameFrom.empty()) {
		this->view_from = DM::ViewContainer(this->viewNameFrom, DM::COMPONENT, DM::READ);
		this->view_from.addAttribute(this->attributeNameFrom, DM::Attribute::INT, DM::READ);
		data_stream.push_back(&view_from);
		valueFromView = true;
	}
	this->registerViewContainers(data_stream);
}

void GDALRandomSelector::mark_parcels(int id, int elements_max)
{
	DM::Logger(DM::Standard) << "place " << elements_max;

	std::stringstream ss;
	ss << this->viewNameFrom << "_id=" << QString::number(id).toStdString();
	vc.setAttributeFilter(ss.str());
	vc.resetReading();

	int total_number_of_featurers = vc.getFeatureCount();
	if (total_number_of_featurers == -1) {
		DM::Logger(DM::Error) << "Error feature counter";
		return;
	}
	std::vector<int> rand_elements;
	std::vector<int> ids;

	OGRFeature * f;

	int counter = 0;
	while(f = vc.getNextFeature()) {
		rand_elements.push_back(counter);
		ids.push_back(f->GetFID());
		counter++;
	}
	if (ids.size() == 0) {
		DM::Logger(DM::Warning) << "No elements found";
		return;
	}
	vc.syncAlteredFeatures();
	vc.syncReadFeatures();

	vc.resetReading();

	//total_number_of_featurers = rand_elements.size();
	DM::Logger(DM::Debug) << "start mixing " << total_number_of_featurers;
	for (int i = 0; i < total_number_of_featurers; i++) {
		int r = rand() % total_number_of_featurers;
		int tmp_value = rand_elements[i];
		rand_elements[i] = rand_elements[r];
		rand_elements[r] = tmp_value;
	}
	DM::Logger(DM::Debug) << "end mixing " << total_number_of_featurers;
	counter = 0;
	bool has_unit = true;
	if (this->units.empty())
		has_unit = false;
	while (elements_max > 0) {
		if (counter > rand_elements.size()-1) {
			break;
		}
		int pos = rand_elements[counter];
		if (pos > ids.size()-1) {
			DM::Logger(DM::Warning) << "invalid index " << pos;
			DM::Logger(DM::Debug) << ids.size();
			DM::Logger(DM::Debug) << rand_elements.size();
			DM::Logger(DM::Debug) << counter-1;
			break;
		}
		f = vc.getFeature(ids[pos]);
		counter++;
		if (!f) {
			DM::Logger(DM::Debug) << ids.size();
			DM::Logger(DM::Debug) << rand_elements.size();
			DM::Logger(DM::Debug) << counter-1;
			DM::Logger(DM::Debug) << rand_elements[counter-1];
			DM::Logger(DM::Warning) << "Feature " << ids[rand_elements[counter-1]] << " not valid";
			continue;
		}
		if (!has_unit) {
			elements_max--;
			f->SetField(attribute.c_str(), 1);
			continue;
		}

		int e = f->GetFieldAsInteger(units.c_str());
		if (e < 1) {
			continue;
		}

		f->SetField(attribute.c_str(), 1);
		elements_max-=e;
	}
	DM::Logger(DM::Standard) << "Rest "<< elements_max;
}

void GDALRandomSelector::run()
{
	if (this->valueFromView) {
		view_from.resetReading();
		OGRFeature * from;
		while(from = view_from.getNextFeature()) {
			this->mark_parcels(from->GetFID(),from->GetFieldAsInteger(this->attributeNameFrom.c_str()));
		}
	} else {
		this->mark_parcels(0, this->elements);
	}
}


