#include "gdalmodules_unittests.h"
#include <dmsimulation.h>
#include <dmlogger.h>
#include <dmlog.h>
#include <dmlogsink.h>
#include <dmviewcontainer.h>
#include <dmsystem.h>

//#define GDALParcelSplit
#define GDALAttributeCaluclator

#ifdef GDALParcelSplit
TEST_F(GDALModules_Unittests, GDALParcelSplit) {
	ostream *out = &cout;
	DM::Log::init(new DM::OStreamLogSink(*out), DM::Standard);
	DM::Logger(DM::Standard) << "Create System";

	DM::Simulation sim;
	QDir dir("./");
	sim.registerModulesFromDirectory(dir);
	DM::Module * rect = sim.addModule("GDALCreateRectangle");
	rect->setParameterValue("width", "100");
	rect->setParameterValue("height", "200");
	rect->setParameterValue("view_name", "CITYBLOCK");
	rect->init();


	DM::Module * parceling = sim.addModule("GDALParcelSplit");
	sim.addLink(rect, "city",parceling, "city");


	sim.run();

	//Check M2
	DM::GDALSystem * sys = (DM::GDALSystem*) parceling->getOutPortData("city");
	DM::ViewContainer components = DM::ViewContainer("PARCEL", DM::FACE, DM::READ);
	components.setCurrentGDALSystem(sys);
	ASSERT_EQ(components.getFeatureCount(), 26);

	sim.clear();

	rect = sim.addModule("GDALCreateRectangle");
	rect->setParameterValue("width", "200");
	rect->setParameterValue("height", "100");
	rect->setParameterValue("view_name", "CITYBLOCK");
	rect->init();


	 parceling = sim.addModule("GDALParcelSplit");
	sim.addLink(rect, "city",parceling, "city");


	sim.run();

	//Check M2
	sys = (DM::GDALSystem*) parceling->getOutPortData("city");
	components = DM::ViewContainer("PARCEL", DM::FACE, DM::READ);
	components.setCurrentGDALSystem(sys);
	ASSERT_EQ(components.getFeatureCount(), 26);

}
#endif

#ifdef GDALAttributeCaluclator
TEST_F(GDALModules_Unittests, GDALAttributeCaluclator) {
	ostream *out = &cout;
	DM::Log::init(new DM::OStreamLogSink(*out), DM::Standard);
	DM::Logger(DM::Standard) << "Create System";

	DM::Simulation sim;
	QDir dir("./");
	sim.registerModulesFromDirectory(dir);
	DM::Module * rect = sim.addModule("GDALCreateRectangle");
	rect->setParameterValue("width", "800");
	rect->setParameterValue("height", "800");
	rect->setParameterValue("view_name", "CITYBLOCK");
	rect->init();


	DM::Module * parceling = sim.addModule("GDALParceling");

	parceling->setParameterValue("width", "100");
	parceling->setParameterValue("height", "100");

	sim.addLink(rect, "city",parceling, "city");


	DM::Module * linking = sim.addModule("GDALSpatialLinking");

	linking->setParameterValue("leadingViewName", "CITYBLOCK");
	linking->setParameterValue("linkViewName", "PARCEL");

	sim.addLink(parceling, "city",linking, "city");
	linking->init();

	DM::Module * attr_calc = sim.addModule("GDALAttributeCalculator");

	attr_calc->setParameterValue("attribute", "PARCEL.counter");
	attr_calc->setParameterValue("equation", "1");

	sim.addLink(linking, "city",attr_calc, "city");
	attr_calc->init();


	DM::Module * attr_calc_counter = sim.addModule("GDALAttributeCalculator");

	attr_calc_counter->setParameterValue("attribute", "CITYBLOCK.sum");
	attr_calc_counter->setParameterValue("equation", "a");
	attr_calc_counter->setParameterValue("variables", "*||*a*|*CITYBLOCK.PARCEL.counter");

	sim.addLink(attr_calc, "city",attr_calc_counter, "city");
	attr_calc_counter->init();

	sim.run();


}
#endif


