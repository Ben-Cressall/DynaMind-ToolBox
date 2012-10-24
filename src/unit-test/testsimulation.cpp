/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012  Christian Urich
 
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

#include <ostream>
#include <testsimulation.h>
#include <dmmodule.h>
#include <dmsimulation.h>
#include <dmlog.h>
#include <dynamicinout.h>
#include <grouptest.h>
#include <dmporttuple.h>

namespace {
	
TEST_F(TestSimulation,testMemory){
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Add Module";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * m = sim.addModule("CreateNodes");
    std::string m_uuid = m->getUuid();
    ASSERT_TRUE(m != 0);
    sim.run();
    ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
    for (int i = 0; i < 5; i++) {
        sim.removeModule(m_uuid);
        m_uuid = sim.addModule("CreateNodes")->getUuid();
        sim.run();
    }
}

TEST_F(TestSimulation,addModuleToSimulationTest){
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Add Module";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * m = sim.addModule("TestModule");
    ASSERT_TRUE(m != 0);
}

TEST_F(TestSimulation,loadModuleNativeTest) {
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Load Native Module";
    DM::Simulation sim;
    ASSERT_TRUE(sim.registerNativeModules("dynamind-testmodules") == true);
}

TEST_F(TestSimulation,repeatedRunTest) {
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Test Repeatet Run";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * m = sim.addModule("TestModule");
    ASSERT_TRUE(m != 0);
    for (long i = 0; i < 1000; i++) {
        sim.run();
        ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
    }

}

TEST_F(TestSimulation,linkedModulesTest) {
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Test Linked Modules";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * m = sim.addModule("TestModule");

    ASSERT_TRUE(m != 0);
    DM::Module * inout  = sim.addModule("InOut");

    ASSERT_TRUE(inout != 0);
    DM::ModuleLink * l = sim.addLink(m->getOutPort("Sewer"), inout->getInPort("Inport"));

    ASSERT_TRUE(l != 0);
    for (long i = 0; i < 10; i++){
        sim.run();
        ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
    }
}

TEST_F(TestSimulation,linkedDynamicModules) {
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Test Linked Modules";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * m = sim.addModule("TestModule");
    ASSERT_TRUE(m != 0);
    DM::Module * inout  = sim.addModule("InOut");
    ASSERT_TRUE(inout != 0);
    DM::ModuleLink * l = sim.addLink(m->getOutPort("Sewer"), inout->getInPort("Inport"));
    ASSERT_TRUE(l != 0);
    DynamicInOut * dyinout  = (DynamicInOut *)sim.addModule("DynamicInOut");
    ASSERT_TRUE(dyinout != 0);
    dyinout->addAttribute("D");
    DM::ModuleLink * l1 = sim.addLink(inout->getOutPort("Inport"), dyinout->getInPort("Inport"));
    ASSERT_TRUE(l1 != 0);
    DM::Module * inout2  = sim.addModule("InOut2");
    ASSERT_TRUE(inout2 != 0);
    DM::ModuleLink * l2 = sim.addLink(dyinout->getOutPort("Inport"), inout2->getInPort("Inport"));
    ASSERT_TRUE(l2 != 0);
    sim.run();
    ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
}

TEST_F(TestSimulation,linkedDynamicModulesOverGroups) {
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Test Linked Modules";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * m = sim.addModule("TestModule");
    ASSERT_TRUE(m != 0);
    DM::Module * inout  = sim.addModule("InOut");
    ASSERT_TRUE(inout != 0);
    DM::ModuleLink * l = sim.addLink(m->getOutPort("Sewer"), inout->getInPort("Inport"));
    ASSERT_TRUE(l != 0);
    //Here comes the group
    GroupTest * g = (GroupTest * ) sim.addModule("GroupTest");
    g->addInPort("In");
    DM::ModuleLink * l_in = sim.addLink(inout->getOutPort("Inport"),g->getInPortTuple("In")->getInPort());
    ASSERT_TRUE(l_in != 0);
    g->addOutPort("Out");
    DynamicInOut * dyinout  = (DynamicInOut *)sim.addModule("DynamicInOut");
    ASSERT_TRUE(dyinout != 0);
    dyinout->setGroup(g);
    ASSERT_TRUE(dyinout != 0);
    dyinout->addAttribute("D");
    DM::ModuleLink * l1 = sim.addLink(g->getInPortTuple("In")->getOutPort(), dyinout->getInPort("Inport"));
    ASSERT_TRUE(l1 != 0);
    DM::ModuleLink * l_out = sim.addLink(dyinout->getOutPort("Inport"), g->getOutPortTuple("Out")->getInPort());
    ASSERT_TRUE(l_out != 0);
    DM::Module * inout2  = sim.addModule("InOut2");
    ASSERT_TRUE(inout2 != 0);
    DM::ModuleLink * l2 = sim.addLink(g->getOutPortTuple("Out")->getOutPort(), inout2->getInPort("Inport"));
    ASSERT_TRUE(l2 != 0);
    sim.run();
    ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
}



TEST_F(TestSimulation,validationtool) {
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Test validation tool";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * mcreator = sim.addModule("CreateAllComponenets");
    ASSERT_TRUE(mcreator != 0);
    DM::Module * mcheck  = sim.addModule("CheckAllComponenets");
    ASSERT_TRUE(mcheck != 0);
    DM::ModuleLink * l2 = sim.addLink(mcreator->getOutPort("sys"), mcheck->getInPort("sys"));
    ASSERT_TRUE(l2 != 0);
    sim.run();
    ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
}

/*
TEST_F(TestSimulation,simplesqltest) {
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Test Reallocation (SQL)";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * mcreator = sim.addModule("CreateAllComponenets");
    ASSERT_TRUE(mcreator != 0);
    DM::Module * mallocator  = sim.addModule("Reallocator");
    ASSERT_TRUE(mallocator != 0);
    DM::Module * mcheck  = sim.addModule("CheckAllComponenets");
    ASSERT_TRUE(mcheck != 0);
    DM::ModuleLink * l1 = sim.addLink(mcreator->getOutPort("sys"), mallocator->getInPort("sys"));
    ASSERT_TRUE(l1 != 0);
    DM::ModuleLink * l2 = sim.addLink(mallocator->getOutPort("sys"), mcheck->getInPort("sys"));
    ASSERT_TRUE(l2 != 0);
    sim.run();
    ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
}
TEST_F(TestSimulation,sqlsuccessortest) {
    ostream *out = &cout;
    DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
    DM::Logger(DM::Standard) << "Test Successor states (SQL)";
    DM::Simulation sim;
    sim.registerNativeModules("dynamind-testmodules");
    DM::Module * mcreator = sim.addModule("CreateAllComponenets");
    ASSERT_TRUE(mcreator != 0);
    DM::Module * msucc  = sim.addModule("SuccessorCheck");
    ASSERT_TRUE(msucc != 0);
    DM::ModuleLink * l1 = sim.addLink(mcreator->getOutPort("sys"), msucc->getInPort("sys"));
    ASSERT_TRUE(l1 != 0);
    sim.run();
    ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
}*/

#ifndef PYTHON_EMBEDDING_DISABLED
    TEST_F(TestSimulation,loadPythonModule) {
        ostream *out = &cout;
        DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
        DM::Logger(DM::Standard) << "Add Module";
        DM::Simulation sim;
        sim.registerPythonModules("PythonModules/scripts/");
        DM::Module * m = sim.addModule("PythonTestModule");
        ASSERT_TRUE(m != 0);
    }

    TEST_F(TestSimulation,runSinglePythonModule) {
        ostream *out = &cout;
        DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
        DM::Logger(DM::Standard) << "Add Module";
        DM::Simulation sim;
        sim.registerPythonModules("PythonModules/scripts/");
        DM::Module * m = sim.addModule("PythonTestModule");
        ASSERT_TRUE(m != 0);
        sim.run();
        ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);
    }

    TEST_F(TestSimulation,runRepeatedSinglePythonModule) {
            ostream *out = &cout;
            DM::Log::init(new DM::OStreamLogSink(*out), DM::Error);
            DM::Logger(DM::Standard) << "Add Module";
            DM::Simulation sim;
            sim.registerPythonModules("PythonModules/scripts/");
            DM::Module * m = sim.addModule("PythonTestModule");
            ASSERT_TRUE(m != 0);
            for (int i = 0; i < 100; i++)
                sim.run();
            ASSERT_TRUE(sim.getSimulationStatus() == DM::SIM_OK);

        }
#endif
}
