/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @author  Michael Mair <abroxos@gmail.com>
 * @version 1.0
 * @section LICENSE
 * This file is part of DynaMite
 *
 * Copyright (C) 2011  Christian Urich, Michael Mair

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

#include "compilersettings.h"

#include <iostream>
#include <simulation.h>
#include <dmdatabase.h>
#include <vibe_log.h>
#include <datamanagement.h>
#include <QThreadPool>
#include <module.h>
using namespace std;
using namespace vibens;

int main(int argc, char *argv[], char *envp[]) {

    //Init Logger
    ostream *out = &cout;
    vibens::Log::init(new OStreamLogSink(*out), vibens::Debug);
    vibens::Logger(vibens::Debug) << "Start";
    QThreadPool::globalInstance()->setMaxThreadCount(1);
    DataManagement::init();
    DMDatabase * db = new DMDatabase();
    DataManagement::getInstance().registerDataBase(db);   //Init Logger
    Simulation * sim = new Simulation;
    sim->registerNativeModules("dmtestmodule");
    vibens::Module * in = sim->addModule("TestModule");
    vibens::Module * outm =sim->addModule("InOut");

    sim->addLink(in->getOutPort("SomeImport"), outm->getInPort("Inport"));
    sim->run();

    QThreadPool::globalInstance()->waitForDone();
    delete sim;
    vibens::Logger(vibens::Debug) << "End";

}
