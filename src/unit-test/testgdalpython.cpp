#include "testgdalpython.h"

#include <dmsimulation.h>
#include <dmlogger.h>
#include <dmlog.h>
#include <dmlogsink.h>
#include <ogrsf_frmts.h>
#include <dmviewcontainer.h>
#include <dmsystem.h>
#include <dmmoduleregistry.h>
#include <dmpythonenv.h>

#define LOADPYTHON
#define ADVANCEDAPI
#define READAPI
#define MODIFYAPI

#define FEATURES "100000"
#define FEATURES_2 "200000"

#ifdef LOADPYTHON
TEST_F(TestGDALPython,LoadPython) {
	ostream *out = &cout;
	DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
	DM::Logger(DM::Standard) << "Create Simulation";
	//std::cout << QDir::currentPath().toStdString() << std::endl;
	DM::PythonEnv::getInstance()->addPythonPath(QDir::currentPath().toStdString());

	DM::Simulation sim;

	sim.registerModulesFromDefaultLocation();
	DM::Logger(DM::Debug) << "Loaded Modules";
	foreach (std::string m, sim.getModuleRegistry()->getRegisteredModules() ) {
		DM::Logger(DM::Debug) << m;
	}

	ASSERT_EQ(NULL,!sim.addModule("CreateGDALComponents"));
	sim.run();

}
#endif

#ifdef ADVANCEDAPI
TEST_F(TestGDALPython,PythonConnectionTest) {
	ostream *out = &cout;
	DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
	DM::Logger(DM::Standard) << "Create Simulation";
	//std::cout << QDir::currentPath().toStdString() << std::endl;
	DM::PythonEnv::getInstance()->addPythonPath(QDir::currentPath().toStdString());

	DM::Simulation sim;

	sim.registerModulesFromDefaultLocation();

	DM::Logger(DM::Debug) << "Loaded Modules";

	DM::Module * m1 = sim.addModule("CreateGDALComponentsAdvanced");
	m1->setParameterValue("elements", FEATURES);
	m1->init();

	DM::Module * m2 = sim.addModule("CreateGDALComponentsAdvanced");
	m2->setParameterValue("elements", FEATURES);
	m2->setParameterValue("append", "1");
	m2->init();
	sim.addLink(m1, "city", m2, "city");
	sim.run();

	DM::GDALSystem * sys = (DM::GDALSystem*) m2->getOutPortData("city");
	DM::ViewContainer components = DM::ViewContainer("component", DM::NODE, DM::READ);
	components.setCurrentGDALSystem(sys);

	QString s_number = QString(FEATURES);
	int number = s_number.toInt();
	ASSERT_EQ(components.getFeatureCount(), 2*number);
}
#endif

#ifdef READAPI
TEST_F(TestGDALPython,PythonReadTest) {
	ostream *out = &cout;
	DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
	DM::Logger(DM::Standard) << "Create Simulation";
	DM::PythonEnv::getInstance()->addPythonPath(QDir::currentPath().toStdString());

	DM::Simulation sim;

	sim.registerModulesFromDefaultLocation();

	DM::Logger(DM::Debug) << "Loaded Modules";

	DM::Module * m1 = sim.addModule("CreateGDALComponentsAdvanced");
	m1->setParameterValue("elements", FEATURES);
	m1->init();

	DM::Module * m2 = sim.addModule("ReadGDALComponentsAdvanced");
	m2->setParameterValue("elements", "0");
	m2->init();
	sim.addLink(m1, "city", m2, "city");
	sim.run();

	DM::GDALSystem * sys = (DM::GDALSystem*) m1->getOutPortData("city");
	DM::ViewContainer components = DM::ViewContainer("component", DM::NODE, DM::READ);
	components.setCurrentGDALSystem(sys);

	QString s_number = QString(QString::fromStdString(m2->getParameterAsString("elements")));
	int number = s_number.toInt();

	QString s_number1 = QString(FEATURES);
	int number1 = s_number1.toInt();
	ASSERT_EQ(number1, number);
}
#endif

#ifdef MODIFYAPI
TEST_F(TestGDALPython,PythonModifyTest) {
	ostream *out = &cout;
	DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
	DM::Logger(DM::Standard) << "Create Simulation";
	DM::PythonEnv::getInstance()->addPythonPath(QDir::currentPath().toStdString());

	DM::Simulation sim;

	sim.registerModulesFromDefaultLocation();

	DM::Logger(DM::Debug) << "Loaded Modules";

	DM::Module * m1 = sim.addModule("CreateGDALComponentsAdvanced");
	m1->setParameterValue("elements", FEATURES);
	m1->init();

	DM::Module * m2 = sim.addModule("ModifyGDALComponentsAdvanced");
	m2->init();

	DM::Module * m3 = sim.addModule("ReadGDALComponentsAdvanced");
	m3->setParameterValue("elements", "0");
	m3->setParameterValue("readValue", "1");
	m3->init();


	sim.addLink(m1, "city", m2, "city");
	sim.addLink(m2, "city", m3, "city");
	sim.run();

	DM::GDALSystem * sys = (DM::GDALSystem*) m1->getOutPortData("city");
	DM::ViewContainer components = DM::ViewContainer("component", DM::NODE, DM::READ);
	components.setCurrentGDALSystem(sys);

	QString s_number = QString(QString::fromStdString(m3->getParameterAsString("sumValue")));
	int number = s_number.toInt();

	QString s_number1 = QString(FEATURES);
	int number1 = s_number1.toInt();
	ASSERT_EQ(3*number1, number);
}
#endif

