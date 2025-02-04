/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @author  Markus Sengthaler <m.sengthaler@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011-2012  Christian Urich, Markus Sengthaler

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

#include <QString>
#include <QFileInfo>

#include <iostream>
#include <map>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <queue>

#include "dmsimulation.h"
#include "dmsimulationreader.h"
#include "dmmodule.h"
#include "dmmoduleregistry.h"
#include "dmmoduleregistry.h"
#include "dmsimulationobserver.h"
#include <dmmodule.h>
#include <dmmoduleregistry.h>
#include <dmgroup.h>
#include <dmsimulationwriter.h>
#include <dmlogger.h>

#ifndef PYTHON_EMBEDDING_DISABLED
#include <dmpythonenv.h>
#endif

#include <QSettings>
#include <QDir>

#include <QtConcurrentRun>

/*
#ifdef _OPENMP
#include <omp.h>
#endif
*/

using namespace DM;

std::string Core::getCoreVersion()
{
	return DYNAMIND_VERSION_CORE;
}

std::string Core::getAuthor()
{
	return "Christian Urich, Michael Mair";
}

std::string Core::getLicense()
{
	std::string license =	"Copyright (C) 2011-2014  " + Core::getAuthor() + "\n\n" +
							"This program is free software; you can redistribute it and/or modify it under the " +
							"terms of the GNU General Public License as published by the Free Software Foundation;" +
							"either version 2 of the License, or (at your option) any later version.\n\n" +
							"This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; " +
							"without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. " +
							"See the GNU General Public License for more details.\n\n" +
							"You should have received a copy of the GNU General Public License along with this program; " +
							"if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.";
	return license;
}

std::string Core::getContact()
{
	std::string contact =	"Christian Urich <christian.urich@gmail>\n"\
							"Michael Mair <michael.mair@uibk.ac.at>";
	return contact;
}

Simulation::Simulation()
{
	status = SIM_OK;
	moduleRegistry = new ModuleRegistry();

	withStatusUpdates = false;
}

Simulation::~Simulation()
{
	clear();
	// TODO: cleanup lost systems
	delete moduleRegistry;

	foreach(SimulationObserver* obs, observers)
		delete obs;

	if (withStatusUpdates)
		statusFile.close();
}

void Simulation::installStatusUpdater(const std::string& path)
{
	withStatusUpdates = true;
	statusFile.open(path.c_str());
}

Module* Simulation::addModule(const std::string ModuleName, Module* parent, bool callInit, const std::string uuid)
{

	std::string uuid_new = uuid;
	foreach (Module * m, modules){
		if (m->uuid.compare(uuid) == 0)
			uuid_new = "";
	}
	Module *module = this->moduleRegistry->createModule(ModuleName);

	if(!module)
		return NULL;
	if (!uuid_new.empty()){
		module->setUUID(uuid_new);
	}
	module->setOwner(parent);
	module->setSimulation(this);


	if(module->isGroup())
		dynamic_cast<Group*>(module)->sim = this;

	modules.push_back(module);
	Logger(Debug) << "Added module" << ModuleName;
	if (callInit)
		module->init();

	return module;
}

void Simulation::removeModule(Module* m)
{
	// TODO check if systems are lost
	Logger(Debug) << "Removing module" << m->getName();

	std::vector<Link*> toDelete;

	foreach(Link* l, links)
		if(l->dest == m || l->src == m)
			toDelete.push_back(l);

	foreach(Link* l, toDelete)
	{
		links.remove(l);
		delete l;
	}

	modules.remove(m);
	delete m;
}

void Simulation::clear()
{
	this->reset();
	foreach(Module* m, modules)
		removeModule(m);
}

bool Simulation::registerModule(const std::string& filepath)
{
	//Logger(Standard) << "Loading native module " << filepath;

	QString qfilepath = QString::fromStdString(filepath);

	if (!qfilepath.contains('.'))
		return false; // not a file
	else if(qfilepath.endsWith(".py"))
	{
#ifndef PYTHON_EMBEDDING_DISABLED
		QFileInfo fi = qfilepath;
		std::stringstream ss;
		ss << fi.absolutePath().toStdString() <<  "/" <<  fi.fileName().remove(".py").toStdString();
		DM::PythonEnv::getInstance()->addPythonPath(fi.absolutePath().toStdString());
		DM::PythonEnv::getInstance()->addPythonPath(ss.str());
		bool success = true;

		try
		{
			success = DM::PythonEnv::getInstance()->registerNodes(moduleRegistry, fi.fileName().remove(".py").toStdString());
		}
		catch (...)
		{
			success = false;
		}

		if (success)
			Logger(Debug) << "successfully loaded python module " << filepath;
		else
			Logger(Error) <<  "failed loading python module " << filepath;

		return success;
#endif
	}
	else if(qfilepath.endsWith(".dll") || qfilepath.endsWith(".so") || qfilepath.endsWith(".dylib"))
	{
		if(moduleRegistry->addNativePlugin(filepath))
		{
            DM::Logger(Standard) <<  "successfully loaded native module " << filepath;
			return true;
		}
		else
		{
			Logger(Debug) <<  "failed loading native module " << filepath;
			return false;
		}
	}

    Logger(Debug) << "not recognized filename ending " << filepath;
	return false;
}

void Simulation::registerModulesFromDirectory(const QDir& dir)
{
	QStringList modulesToLoad = dir.entryList();
	foreach (QString module, modulesToLoad)
	{
		if (module == ".." || module == "." )
			continue;
		std::string modulepath = (dir.absolutePath() +"/" + module).toStdString();
		registerModule(modulepath);
	}
}

void Simulation::registerModulesFromDefaultLocation()
{
//	QVector<QDir> cpv;
//	cpv.push_back(QDir::currentPath() + "/Modules");
//	cpv.push_back(QDir::currentPath() + "/bin/Modules");
//#if defined DEBUG || _DEBUG
//	cpv.push_back(QDir::currentPath() + "/../Modules/Debug");
//	cpv.push_back(QDir::currentPath() + "/../../../output/Debug");	// win32 unit-tests
//	cpv.push_back(QDir::currentPath() + "/../../../output/Modules/Debug");
//#else
//	cpv.push_back(QDir::currentPath() + "/../Modules/Release");
//	cpv.push_back(QDir::currentPath() + "/../Modules/RelWithDebInfo");
//	cpv.push_back(QDir::currentPath() + "/../../../output/Modules/Release");
//	cpv.push_back(QDir::currentPath() + "/../../../output/Modules/RelWithDebInfo");
//#endif

//#ifndef PYTHON_EMBEDDING_DISABLED
//	cpv.push_back(QDir::currentPath() + "/bin/PythonModules/scripts");
//	cpv.push_back(QDir::currentPath() + "/PythonModules/scripts");
//#endif

	QVector<QDir> cpv;
	cpv.push_back( QDir(QString::fromStdString(this->getSimulationConfig().getDefaultModulePath() + "/PythonModules/scripts") ) );
	cpv.push_back(  QDir(QString::fromStdString(this->getSimulationConfig().getDefaultModulePath() + "/Modules" ) ) );
	foreach (QDir cp, cpv) {
		DM::Logger(DM::Standard) << "Load modules from: " << cp.absolutePath().toStdString();
		registerModulesFromDirectory(cp);
	}
}

bool Simulation::isLinkingValid(Module* source, std::string outPort, Module* dest, std::string inPort,
								bool logOutput)
{
	if(!source || !dest)
	{
		if(logOutput)	Logger(Error) << "invalid module (NULL pointer)";
		return false;
	}
	if(source == dest)
	{
		if(logOutput)	Logger(Error) << "cannot link module itself";
		return false;
	}

	// group stuff
	if(dest->owner == source)	// into group link
	{
		if(!source->hasInPort(outPort))
		{
			if(logOutput)	Logger(Error) << "accessing not existing inner group in-port";
			return false;
		}
	}
	else if(!source->hasOutPort(outPort))
	{
		if(logOutput)	Logger(Error) << "accessing not existing out-port";
		return false;
	}

	if(source->owner == dest)	// out of group link
	{
		if(!dest->hasOutPort(inPort))
		{
			if(logOutput)	Logger(Error) << "accessing not existing inner group out-port";
			return false;
		}
	}
	else if(!dest->hasInPort(inPort))
	{
		if(logOutput)	Logger(Error) << "accessing not existing in-port";
		return false;
	}

	// check if the same link exists already
	foreach(Link* l, links)
	{
		if(l->dest == dest && l->inPort == inPort)
		{
			if(l->src == source && l->outPort == outPort)
			{
				if(logOutput)	Logger(Warning) << "Link already exists: " << outPort << "to" << inPort;
				return false;
			}
			if(!l->dest->isGroup())
			{
				if(logOutput)	Logger(Warning) << "Double linking an in-Port is not possible by now: " << outPort << "to" << inPort;
				return false;
			}
		}
	}
	return true;
}

bool Simulation::addLink(Module* source, std::string outPort, Module* dest, std::string inPort, bool checkStream)
{
	if(!isLinkingValid(source, outPort, dest, inPort, true))
		return false;

	Link* l = new Link();
	l->src = source;
	l->outPort = outPort;
	l->dest = dest;
	l->inPort = inPort;
	l->isIntoGroupLink = (dest->owner == source);
	l->isOutOfGroupLink = (source->owner == dest);
	links.push_back(l);
	// stream check
	Logger(Debug) << "Added link from module '" << l->src->getClassName() << "' port '" << outPort
		<< "' to module '" << l->dest->getClassName() << "' port '" << inPort << "'";

	if(checkStream)
	{
		if(checkModuleStreamForward(l))
			Logger(Debug) << "checking stream successfull";
		else
			Logger(Warning) << "stream incomplete" ;


		if (l->isIntoGroupLink || l->isOutOfGroupLink)
		{
			DM::Logger(DM::Warning) << "Relinking groups resets the simulation";
			this->reset();
		}
		else if (getOutgoingLinks(source, outPort).size() > 1)
		{
			DM::Logger(DM::Warning) << "Branching resets the simulation";
			this->reset();
		}
		else
		{
			shiftData(l);
			l->src->setOutPortData(outPort, NULL);
		}
	}

	return true;
}
bool Simulation::removeLink(Module* source, std::string outPort, Module* dest, std::string inPort)
{
	Link* toDelete = NULL;
	foreach(Link* l, links)
	{
		if(	l->src == source &&
			l->outPort == outPort &&
			l->dest == dest &&
			l->inPort == inPort)
		{
			// reset resets all stream views
			// lets keep the ones not affected by this link
			std::map<std::string, std::map<std::string,View> > streamViews = l->dest->streamViews;
			streamViews.erase(l->inPort);
			l->dest->reset();
			l->dest->streamViews = streamViews;

			toDelete = l;
			break;
		}
	}
	if(toDelete)
	{
		links.remove(toDelete);
		Logger(Debug) << "Deleted link from port " << outPort << "to" << inPort;
		checkModuleStreamForward(dest);
		return true;
	}
	return false;
}

std::vector<Link*> Simulation::getIngoingLinks(const Module* dest, const std::string& inPort) const
{
	std::vector<Link*> ls;
	foreach(Link* l, links)
		if(	l->dest == dest && l->inPort == inPort && !l->isOutOfGroupLink)
			ls.push_back(l);

	return ls;
}
std::vector<Link*> Simulation::getOutgoingLinks(const Module* src, const std::string& outPort) const
{
	std::vector<Link*> ls;
	foreach(Link* l, links)
		if(	l->src == src && l->outPort == outPort && !l->isIntoGroupLink)
			ls.push_back(l);

	return ls;
}
std::vector<Link*> Simulation::getIntoGroupLinks(const Module* src, const std::string& inPort) const
{
	std::vector<Link*> ls;
	foreach(Link* l, links)
		if(	l->src == src && l->outPort == inPort && l->isIntoGroupLink)
			ls.push_back(l);

	return ls;
}
std::vector<Link*> Simulation::getOutOfGroupLinks(const Module* dest, const std::string& outPort) const
{
	std::vector<Link*> ls;
	foreach(Link* l, links)
		if(	l->dest == dest && l->inPort == outPort && l->isOutOfGroupLink)
			ls.push_back(l);


	return ls;
}

std::string Simulation::serialise()
{
	QBuffer dest;
	writeSimulation(&dest, "<clipboard>");

	return QString(dest.data()).toStdString();
}

bool Simulation::checkGroupStreamForward(Group* g, std::string streamName, bool into)
{
	if(!g)
	{
		DM::Logger(DM::Error) << "critical error: group == NULL at checkGroupStreamForward";
		return false;
	}

	std::vector<Link*> nextLinks;
	std::map<std::string, DM::View>* curStreamViews;

	if(into)
	{
		DM::Logger(DM::Debug) << "initializing group '" << g->getClassName() << "'";
		g->init();

		nextLinks = getIntoGroupLinks(g, streamName);
		curStreamViews = &g->streamViews[streamName];
	}
	else
	{
		nextLinks = getOutgoingLinks(g, streamName);
		curStreamViews = &g->outStreamViews[streamName];
	}

	bool success = true;
	foreach(Link* l, nextLinks)
	{
		if(!l->isOutOfGroupLink)
			l->dest->streamViews[l->inPort] = *curStreamViews;
		else
			((Group*)l->dest)->outStreamViews[l->inPort] = *curStreamViews;

		if(!l->dest->isGroup())
		{
			if(!checkModuleStreamForward(l->dest))
				success = false;
		}
		else
			if(!checkGroupStreamForward((Group*)l->dest, l->inPort, !l->isOutOfGroupLink))
				success = false;
	}

	if (!into) // if we followed outside the group, the inner part must have been successful
		g->setStatus(MOD_CHECK_OK);

	return success;
}

bool IsWriteOnly(const std::map<std::string, View>& stream)
{
	mforeach(const View& v, stream)
		if(v.getAccessType() != DM::WRITE)
			return false;
	return true;
}

bool Simulation::checkModuleStreamForward(Link* link)
{
	if(link->src->isGroup())
		return checkGroupStreamForward((Group*)link->src, link->outPort, link->isIntoGroupLink);
	else
		return checkModuleStreamForward(link->src);
}

bool Simulation::checkModuleStreamForward(Module* m)
{
	bool success = true;

	// check if all streams are set
	// = all pre-modules have been initialized and transmitted view data
	// this can exclusivly be the case on modules with multiple in-ports
	foreach(std::string portName, m->getInPortNames())
	{
		if(m->streamViews.find(portName) == m->streamViews.end())
		{
			DM::Logger(DM::Debug) << "module '" << m->getClassName() << "' waiting for other streams";
			m->setStatus(MOD_UNTOUCHED);
			return true;
		}
	}

	// updated stream consist of input stream + written streams in this module
	std::map<std::string, std::map<std::string, View> > updatedStreams;

	if (m->getStatus() != MOD_CHECK_OK && m->getStatus() != MOD_EXECUTION_OK)
	{
		updatedStreams = m->streamViews;

		DM::Logger(DM::Debug) << "initializing module '" << m->getClassName() << "'";
		m->init();

		for (std::map<std::string, std::map<std::string, View> >::iterator it = m->accessedViews.begin();
			it != m->accessedViews.end(); ++it)
		{
			const std::string& streamName = it->first;
			DM::Logger(DM::Debug) << "checking stream '" << streamName << "' in module '" << m->getClassName() << "'";

			mforeach(const View& v, it->second)
			{
				if (v.getName() == "dummy")	// TODO: this is ugly, horrible, terrible and awful
					continue;				// but for backwards compatibility its necessary

				DM::ACCESS ac = v.getAccessType();

				if(ac==WRITE && map_contains(&m->streamViews[streamName], v.getName()))
					ac=MODIFY;

				if (ac < WRITE)
				{
					// check if we can access the desired view
					if (!map_contains(&m->streamViews[streamName], v.getName()))
					{
						std::string ownerString = m->getOwner() ? "in '" + m->getOwner()->getName() + "'": "";

						DM::Logger(DM::Error) << "module '" << m->getName()
							<< "' of type '" << m->getClassName()
							<< ownerString
							<< "' tries to access the nonexisting view '" << v.getName()
							<< "' from stream '" << streamName << "'";
						m->setStatus(MOD_CHECK_ERROR);
						success = false;
					}
					else
					{
						View& existingView = updatedStreams[streamName][v.getName()];
						// check if all attributes are ready existing
						std::vector<std::string> existingAttributes = existingView.getAllAttributes();
						foreach(std::string attributeName, v.getAllAttributes())
						{
							DM::ACCESS a = v.getAttributeAccessType(attributeName);

							if(a==WRITE && vector_contains(&existingAttributes, attributeName))
								a = MODIFY;

							if (a < WRITE)
							{
								// check if existing
								if (!vector_contains(&existingAttributes, attributeName))
								{
									DM::Logger(DM::Error) << "module '" << m->getClassName()
										<< "' tries to access the nonexisting attribute '" << attributeName
										<< "' in view '" << v.getName()
										<< "' from stream '" << streamName << "'";
									m->setStatus(MOD_CHECK_ERROR);
									success = false;
								}
							}
							else
							{
								// check if existing
								if (vector_contains(&existingAttributes, attributeName))
								{
									DM::Logger(DM::Warning) << "module '" << m->getClassName()
										<< "' overwrites attribute '" << attributeName
										<< "' in view '" << v.getName()
										<< "' from stream '" << streamName << "'";
								}
								// add attribute
								Attribute::AttributeType type = v.getAttributeType(attributeName);
								if (type != Attribute::LINK)
									existingView.addAttribute(attributeName, type, READ);
								else
									existingView.addAttribute(attributeName, v.getNameOfLinkedView(attributeName), READ);
							}

						}
					}
				}

				if (ac == WRITE)
				{
					// it may be, that a view already exists
					if (map_contains(&m->streamViews[streamName], v.getName()))
					{
						DM::Logger(DM::Warning) << "module '" << m->getClassName()
							<< "' overwrites view '" << v.getName()
							<< "' from stream '" << streamName << "'";
					}
					updatedStreams[streamName][v.getName()] = v;
				}

				if (ac == DEL)
					updatedStreams[streamName].erase(v.getName());
			}
		}
		if (!success)
			return success;

		m->setStatus(MOD_CHECK_OK);

		m->outStreamViews = updatedStreams;
	}
	else
		updatedStreams = m->outStreamViews;

	// shift views to next module
	// loop thought all out ports
	//for(std::map<std::string, std::map<std::string,View> >::iterator it = updatedStreams.begin();
	//	it != updatedStreams.end(); ++it)
	foreach(std::string outPortName, m->getOutPortNames())
	{
		//const std::string& streamName = it->first;
		// get all out ports, all assigned links and push forward
		std::vector<Link*> outLinks;
		//foreach(std::string outPortName, m->getOutPortNames())
		foreach(Link* outLink, getOutgoingLinks(m, outPortName))
			outLinks.push_back(outLink);

		foreach(Link* l, outLinks)
		{
			std::map<std::string,View> outStream = updatedStreams[outPortName];
			if(l->outPort != outPortName)
			{
				const std::map<std::string,View>& streamAccess = m->accessedViews[l->outPort];
				if(IsWriteOnly(streamAccess))
					outStream = m->accessedViews[l->outPort];
				else
					continue;
			}

			if(outStream.size() == 0)
				continue;

			if(!l->isOutOfGroupLink)
				l->dest->streamViews[l->inPort] = outStream;
			else
				((Group*)l->dest)->outStreamViews[l->inPort] = outStream;

			if(!l->dest->isGroup())
			{
				if(!checkModuleStreamForward(l->dest))
					success = false;
			}
			else
				if(!checkGroupStreamForward((Group*)l->dest, l->inPort, !l->isOutOfGroupLink))
					success = false;
		}
	}
	return success;
}

bool Simulation::checkStream()
{
	DM::Logger(DM::Standard) << ">> Check Streams";
	foreach(Module* m, this->modules)
		m->streamViews.clear();

	bool success = true;
	QList<QFuture<bool>*> results;
	foreach(Module* m, modules)
		if(m->getInPortNames().size() == 0) {
			if (!checkModuleStreamForward(m))
				return false;
		}

	return success;
}

void Simulation::setSimulationConfig(SimulationConfig config)
{
	this->config = config;
}

SimulationConfig Simulation::getSimulationConfig()
{
	return config;
}

void Simulation::emitStatusUpdate(float progress, const std::string& msg)
{
	std::string tStamp = QDateTime::currentDateTime().toString("ddMMyyhhmmss").toStdString();
	statusFile << tStamp << " " << (size_t) (progress * 100) << " " << status << " '" << msg << "'" << endl;
}

class Worklist: public std::list<Module*>
{
public:
	void unique_insert(Module* m)
	{
		if(find(begin(), end(), m) == end())
			push_back(m);
	}
};

void Simulation::run()
{
    srand((unsigned)time(NULL));
	canceled = false;
	// notify progress
	foreach(SimulationObserver* obs, observers)
		obs->update(0);

	if (withStatusUpdates) {
		emitStatusUpdate(0., "initializing simulation");
	}

	QElapsedTimer simtimer;
	simtimer.start();

	Logger(Standard) << ">> checking simulation";
	if(!checkStream())
	{
		Logger(Error) << ">> checking simulation failed";
		this->status = SIM_FAILED;

		if (withStatusUpdates) {
			emitStatusUpdate(0., "checking simulation failed");
		}

		return;
	}
	Logger(Standard) << ">> checking simulation succeeded (took " << (long)simtimer.elapsed() << "ms)";
	simtimer.restart();
	Logger(Standard) << ">> starting simulation";
	// get modules with no imput - beginning modules list
	Worklist worklist;
	foreach(Module* m, modules)
		if(m->inPortsSet() && m->getStatus() != MOD_EXECUTION_OK)
			worklist.unique_insert(m);

	// the domain in which we are currently executing, starting with root = NULL
	Group* currentGroupDomain = NULL;
	// progress stuff
	int cntModulesFinished = 0;
	int numModulesToFinish = modules.size();
	// run modules
	// if not started decoupled, the state of the future is canceled, started and finished
	while(worklist.size() && !canceled)
	{
		// get first element
		Worklist::iterator it = worklist.begin();
		Module* m = *it;
		worklist.remove(m);

		// check if we stay in our domain
		if(m->getOwner() != currentGroupDomain)
		{
			// we are going into or out of the group
			// - check if we have already finished the current one
			bool groupFinished = true;
			foreach(Module* n, worklist)
			{
				if(n->getOwner() == currentGroupDomain)
				{
					groupFinished = false;
					break;
				}
			}
			if(!groupFinished)
			{
				// still modules left in this group
				worklist.unique_insert(m);
				continue;
			}
		}

		if(!m->isGroup())
		{
			// if we execute a module more than once, our total module count increases
			if(m->getStatus() == MOD_EXECUTION_OK)
				numModulesToFinish++;
			// execute module
			Logger(Standard) << "running module '" << m->getClassName() << " "<< m->getName() << "'";
			QElapsedTimer modTimer;
			modTimer.start();
			m->setStatus(MOD_EXECUTING);

			float progress = (float)cntModulesFinished/numModulesToFinish;
			if (withStatusUpdates) {
				emitStatusUpdate(progress, "running module " + m->getName());
			}

			//Run PreRun to init GDALSytem before module is executed
			m->preRun();
			//Run Module
			QtConcurrent::run(m, &Module::moduleRun).waitForFinished();
			// check for errors
			//Run AfterRun to clean up GDALModules
			m->afterRun();

			cntModulesFinished++;
			if(m->getStatus() == MOD_EXECUTION_ERROR)
			{
				if(Module* owner = m->getOwner())
					owner->setStatus(MOD_EXECUTION_ERROR);

				Logger(Error) << "module '" << m->getName() << "' failed after " << (long)modTimer.elapsed() << "ms";
				this->status = DM::SIM_FAILED;

				if (withStatusUpdates) {
					emitStatusUpdate(0., "module " + m->getName() + " failed");
				}

				return;
			}
			else
			{
				Logger(Standard)	<< "module '" << m->getName() << "' executed successfully (took "
					<< (long)modTimer.elapsed() << "ms)";
				m->setStatus(MOD_EXECUTION_OK);

				// notify progress
				foreach(SimulationObserver* obs, observers)
					obs->update(progress);
			}
			// shift data from out port to next inport
			foreach(Module* nextModule, shiftModuleOutput(m))
				worklist.unique_insert(nextModule);
		}
		else
		{
			Group* g = (Group*)m;
			// we are now operating in domain g
			currentGroupDomain = g;

			Logger(Standard) << "running group '" << g->getName() << "'";

			if(g->condition())
			{
				Logger(Standard) << "condition fulfilled for group '" << g->getName() << "'";
				// to ensure loop in loops are working properly, we init all modules of a
				// group before starting it - resetting all internal counters
				Logger(Debug) << "resetting modules in group";
				foreach(Module* m, modules)
					if(m->getOwner() == g)
					{
						// FIX: module::getData also searches output if the system is already created
						for(std::map<std::string, ISystem*>::iterator it = m->outPorts.begin(); it != m->outPorts.end(); ++it)
							it->second = NULL;

						m->init();
					}

				// execute group
				g->setStatus(MOD_EXECUTING);
				// instead of m::run() we simply shift the data to the first internal module
				foreach(Module* nextModule, shiftGroupInput(g))
					worklist.unique_insert(nextModule);
			}
			else
			{
				Logger(Standard) << "finishing group '" << g->getName() << "'";
				// finish and shift data
				g->setStatus(MOD_EXECUTION_OK);
				foreach(Module* nextModule, shiftModuleOutput(g))
					worklist.unique_insert(nextModule);

				// reset domain
				currentGroupDomain = (Group*)g->getOwner();
			}
		}
	}
	if(canceled)
	{
		Logger(Standard) << ">> canceled simulation (time elapsed " << (long)simtimer.elapsed() << "ms)";
		Logger(Debug) << ">> cleaning up";
		//reset();
		// notify progress
		foreach(SimulationObserver* obs, observers)
			obs->update(0);
	}
	else
	{
		Logger(Standard) << ">> finished simulation (took " << (long)simtimer.elapsed() << "ms)";
		// notify progress
		foreach(SimulationObserver* obs, observers)
			obs->update(1);
	}

	if (withStatusUpdates) {
		emitStatusUpdate(canceled ? 0. : 1., "finished simulation");
	}
}

void Simulation::cancel()
{
	Logger(Standard) << ">> canceling simulation";
	canceled = true;
}

ISystem *Simulation::getData(Link* l)
{
	if(l->src->isGroup() && l->dest->getOwner() == l->src)
		// into group link
		return l->src->getInPortData(l->outPort);
	else
		return l->src->getOutPortData(l->outPort);
}

void Simulation::shiftData(Link* l, bool successor)
{
	ISystem * data = getData(l);
	if(!data)
		return;
	if(successor)
		data = data->createSuccessor();
	// shift pointer
	if(l->dest->isGroup() && l->src->getOwner() == l->dest)
		// out of group link
		l->dest->setOutPortData(l->inPort, data);
	else
	{
		l->dest->setInPortData(l->inPort, data);
		// FIX: modules which won't call getData(...)
		// won't get data on the out port
		if(l->dest->hasOutPort(l->inPort) && !l->dest->isGroup())
			l->dest->setOutPortData(l->inPort, data);
	}
}

std::set<Module*> Simulation::shiftModuleOutput(Module* m)
{
	std::set<Module*> nextModules;
	for(std::map<std::string, ISystem*>::iterator it = m->outPorts.begin();
		it != m->outPorts.end();	++it)
	{
		// first get all links starting at the given module
		std::list<Link*> branches;
		std::vector<Link*> outLinks = getOutgoingLinks(m, it->first);
		foreach(Link* l, outLinks)
			if(getData(l))
				branches.push_back(l);

		if(branches.size() > 0)
		{
			bool createSuccessor = branches.size() > 1 || m->isSuccessorMode();
			foreach(Link* l, branches)
			{
				shiftData(l, createSuccessor);
				if(l->dest->inPortsSet())
					nextModules.insert(l->dest);
			}
			// reset out port
			if(!createSuccessor)
				it->second = NULL;
		}
		else if(outLinks.size())
			Logger(Warning) << "module '" << m->getClassName() << "' does not write data to outport '" << it->first << "'";
	}

	// check if this module is read only
	bool readOnly = true;
	typedef std::map<std::string, View> viewmap;
	mforeach(viewmap views, m->getAccessedViews())
		mforeach(const View& view, views)
			if(view.writes())
				readOnly = false;

	// do not reset inport if there is no outport and the module is read only
	if(!(m->outPorts.size() == 0 && readOnly))
		// reset in ports
		for(std::map<std::string, ISystem*>::iterator it = m->inPorts.begin();
			it != m->inPorts.end();	++it)
			it->second = NULL;

	return nextModules;
}

std::set<Module*> Simulation::shiftGroupInput(Group* g)
{
	std::set<Module*> nextModules;
	std::vector<std::string> inPorts = g->getInPortNames();
	foreach(std::string inPort, inPorts)
	{
		if(g->getInPortData(inPort))
		{
			std::vector<Link*> intoGroupLinks = getIntoGroupLinks(g, inPort);
			if(intoGroupLinks.size() != 0)
			{
				foreach(Link* l, intoGroupLinks)
				{
					shiftData(l, inPorts.size() > 1);
					if(l->dest->inPortsSet())
						nextModules.insert(l->dest);
				}
			}
			else
			{
				Logger(Warning) << "no link into group found, system lost (destructed)";
				delete g->getInPortData(inPort);
				g->setInPortData(inPort, NULL);
			}
		}
	}
	return nextModules;
}

void Simulation::reset()
{
	Logger(Standard) << ">> Reset Simulation";
	// gather all root systems, then delete them
	// if done at once, deleting the root will cause corrupt predecessor methods
	std::set<ISystem*> systems;
	foreach(Module* m, this->modules)
	{
		foreach(std::string portName, m->getInPortNames())
			if(ISystem* sys = m->getInPortData(portName))
				if(!sys->getPredecessor())
					systems.insert(sys);
		foreach(std::string portName, m->getOutPortNames())
			if(ISystem* sys = m->getOutPortData(portName))
				if(!sys->getPredecessor())
					systems.insert(sys);
	}
	// delete them
	foreach(ISystem* sys, systems) {
		delete sys;
	}

	// reset modules
	foreach(Module* m, this->modules)
	{
		m->reset();
		m->setStatus(MOD_UNTOUCHED);
	}
	checkStream();
}

bool Simulation::registerModulesFromSettings()
{
	QSettings settings;
	QString text;
	QStringList list;

#ifndef PYTHON_EMBEDDING_DISABLED
	//Init Python
	QStringList pythonhome = settings.value("pythonhome",QStringList()).toString().replace("\\","/").split(",");
	for (int index = 0; index < pythonhome.size(); index++)
		DM::PythonEnv::getInstance()->addPythonPath(pythonhome.at(index).toStdString());

	QDir pythonDir;
	text = settings.value("pythonModules").toString();
	list = text.replace("\\","/").split(",");
	foreach (QString s, list)
		registerModulesFromDirectory(s);
#endif

	// Native Modules
	text = settings.value("nativeModules").toString();
	list = text.replace("\\","/").split(",");
	foreach (QString s, list) {
		if (s.isEmpty())
			continue;
		registerModule(s.toStdString());
	}

	return true;
}

// for old versions
void LoopGroupAdaptor(	QVector<LinkEntry>& links,
					  QVector<ModuleEntry>& modules,
					  ModuleEntry& loopGroup)
{
	// check if it is an old version
	QMap<QString, QString>::iterator itInView = loopGroup.parameters.find("nameOfInViews");
	QMap<QString, QString>::iterator itOutView = loopGroup.parameters.find("nameOfOutViews");

	if(itInView == loopGroup.parameters.end() || itOutView == loopGroup.parameters.end())
		return;

	// get modules in group for searching backlinks
	QMap<QString, ModuleEntry> inGroupModules;
	foreach(const ModuleEntry& m, modules)
		if(m.GroupUUID == loopGroup.UUID)
			inGroupModules[m.UUID] = m;

	// get backlinks
	QList<LinkEntry> backLinks;
	foreach(const LinkEntry& l, links)
		if(l.backlink && inGroupModules.find(l.InPort.UUID) != inGroupModules.end())
			backLinks.push_back(l);

	// remove backlinks
	foreach(const LinkEntry& bl, backLinks)
	{
		int index = -1;
		int i = 0;
		foreach(const LinkEntry& l, links)
		{
			if(		l.InPort.PortName == bl.InPort.PortName
				&&	l.InPort.UUID == bl.InPort.UUID
				&&	l.OutPort.PortName == bl.OutPort.PortName
				&&	l.OutPort.UUID == bl.OutPort.UUID)
			{
				index = i;
				break;
			}
			else
				i++;
		}
		if(index>0)
			links.remove(index); // TODO check
	}

	QList<QString> nameOfInPorts = itInView.value().split("*|*", QString::SkipEmptyParts);
	QList<QString> nameOfOutPorts = itOutView.value().split("*|*", QString::SkipEmptyParts);

	// remove old params
	loopGroup.parameters.remove("nameOfInViews");
	loopGroup.parameters.remove("nameOfOutViews");

	// add write streams
	QString writeStreams;
	foreach(const LinkEntry& backLink, backLinks)
	{
		// get link into group, connecting loop entry with backlink end
		QString inPortName;
		QString outPortName;
		foreach(const LinkEntry& refl, links)
		{
			if(refl.InPort.PortName == backLink.InPort.PortName
				&& refl.InPort.UUID == backLink.InPort.UUID
				&& refl.OutPort.UUID == loopGroup.UUID)
			{
				inPortName = refl.OutPort.PortName;
			}
		}
		foreach(const LinkEntry& refl, links)
		{
			if(refl.OutPort.PortName == backLink.OutPort.PortName
				&& refl.OutPort.UUID == backLink.OutPort.UUID
				&& refl.InPort.UUID == loopGroup.UUID)
			{
				outPortName = refl.InPort.PortName;
			}
		}
		if(inPortName.size() == 0)	// cant resolve link
		{
			DM::Logger(Error) << "Backlink could not be converted from old loopgroup format";
			return;
		}
		// add new write stream
		writeStreams += inPortName + "*|*";
		// remove from in ports, as the port is added with the write stream parameter
		nameOfInPorts.removeAll(inPortName);
		nameOfOutPorts.removeAll(outPortName);
		// reconnect links, as the outport from the loopgroup has been renamed
		// from 'backLink.OutPort.PortName' to 'portName'
		for(QVector<LinkEntry>::iterator innerLink = links.begin(); innerLink != links.end(); ++innerLink)
		{
			if(innerLink->OutPort.PortName == backLink.OutPort.PortName
				&& innerLink->OutPort.UUID == backLink.OutPort.UUID
				&& innerLink->InPort.UUID == loopGroup.UUID)
			{
				// this link goes to an outport
				// change all outgoing links on the outside
				for(QVector<LinkEntry>::iterator outerLink = links.begin(); outerLink != links.end(); ++outerLink)
				{
					if(outerLink->OutPort.UUID == innerLink->InPort.UUID
						&& outerLink->OutPort.PortName == innerLink->InPort.PortName)
					{
						outerLink->OutPort.PortName = inPortName;
					}
				}
				innerLink->InPort.PortName = inPortName;
			}
		}
	}
	// add read streams
	QString readStreams;
	foreach(QString portName, nameOfInPorts)
		readStreams += portName + "*|*";

	// TRICKY: we assume that if out ports are left, one read stream is meant to be a write stream
	// that must fulfill following conditions:
	// just a single out and a single in port, no write streams so far
	if(nameOfOutPorts.size() == 1 && writeStreams.size() == 0 && nameOfInPorts.size() == 1)
	{
		// get names
		QString oldOutPortName = nameOfOutPorts.front();
		QString newOutPortName = nameOfInPorts.at(0);
		// exchange
		writeStreams = readStreams;
		readStreams.clear();
		// relink
		for(QVector<LinkEntry>::iterator it = links.begin(); it != links.end(); ++it)
		{
			if(it->OutPort.UUID == loopGroup.UUID && it->OutPort.PortName == oldOutPortName)
				it->OutPort.PortName = newOutPortName;
			else if(it->InPort.UUID == loopGroup.UUID && it->InPort.PortName == oldOutPortName)
				it->InPort.PortName = newOutPortName;
		}
		DM::Logger(DM::Debug) << "exchanged port '" << oldOutPortName << "' of module '"
			<< loopGroup.ClassName << "' with port '" << newOutPortName << "'";
	}

	loopGroup.parameters["writeStreams"] = writeStreams;
	loopGroup.parameters["readStreams"] = readStreams;
}

void UpdateVersion(QVector<LinkEntry>& links, QVector<ModuleEntry>& modules)
{
	for(QVector<ModuleEntry>::iterator it = modules.begin(); it != modules.end(); ++it)
		if(it->ClassName == "LoopGroup")
			LoopGroupAdaptor(links, modules, *it);
}

bool Simulation::loadSimulation(QIODevice* source, QString filepath,
								std::map<std::string, DM::Module*>& modMap,
								DM::Module* overwrittenOwner, bool overwriteGroupOwner)
{


	QDir simFileDir = QFileInfo(filepath).absoluteDir();	// for param corr.
	Logger(Standard) << ">> loading simulation file '" << filepath << "'";

	SimulationReader simreader(source);

	this->setSimulationConfig(simreader.getSettings());

	QVector<ModuleEntry> moduleEntries = simreader.getModules();
	QVector<LinkEntry> linkEntries = simreader.getLinks();
	UpdateVersion(linkEntries, moduleEntries);

	std::set<std::string> uuid_in_loadsim;
	foreach(ModuleEntry m, moduleEntries) {
		uuid_in_loadsim.insert(m.UUID.toStdString());
	}



	int waitingForGroup = 0;
	// load modules
	while(moduleEntries.size() > 0 && moduleEntries.size() >= waitingForGroup)
	{
		ModuleEntry me = moduleEntries.first();
		moduleEntries.pop_front();
		DM::Module* owner = modMap[me.GroupUUID.toStdString()];


		if(me.GroupUUID.size() && me.GroupUUID != simreader.getRootGroupUUID() && (uuid_in_loadsim.find(me.GroupUUID.toStdString()) != uuid_in_loadsim.end()))
		{
			if(!owner)
			{
				moduleEntries.push_back(me);
				waitingForGroup++;
				continue;
			}
			else
				waitingForGroup--;
		}
		else if(overwriteGroupOwner)	// its a root member, should we overwrite it?
			owner = overwrittenOwner;

		// do not init module - we first have to set parameters and links as well as checking the stream!
		if(DM::Module* m = addModule(me.ClassName.toStdString(), owner, false, me.UUID.toStdString()))
		{
			modMap[me.UUID.toStdString()] = m;
			// load parameters
			for(QMap<QString, QString>::iterator it = me.parameters.begin(); it != me.parameters.end(); ++it)
			{
				std::string value = it.value().toStdString();
				if(DM::Module::Parameter* p = m->getParameter(it.key().toStdString()))
				{
					if(p->type == DM::FILENAME && filepath != "<clipboard>")
					{
						if (!it.value().contains("host=")){
							QDir path(simFileDir.absoluteFilePath(it.value()));
                            DM::Logger(DM::Standard) << "converted relative path '" << value << "'";
                            value = QDir::cleanPath(simFileDir.absoluteFilePath(it.value())).toStdString();
                            DM::Logger(DM::Standard) << "to absolute path '" << value << "'";
						}
					}
				}
				m->setParameterValue(it.key().toStdString(), value);
			}
			// set successor mode
			m->setSuccessorMode(me.DebugMode);
			m->setName(me.Name.toStdString());

			// set filter
			std::vector<DM::Filter> filters;

			foreach (FilterEntry fe, me.filterEntries) {
				DM::Filter f(fe.view_name.toStdString(),
							FilterArgument(fe.attribtue_filter.toStdString()),
							FilterArgument(fe.spatial_filter.toStdString()));
				filters.push_back(f);
			}
			m->setFilter(filters);
		}
		else
			DM::Logger(Error) << "creating module '" << me.ClassName.toStdString() << "' failed";
	}
	if(waitingForGroup>0)
		foreach(ModuleEntry me, moduleEntries)
		DM::Logger(Error) << "could not find the group for module '" << me.ClassName.toStdString() << "'";

	// load links
	foreach(LinkEntry le, linkEntries)
	{
		DM::Module *src = modMap[le.OutPort.UUID.toStdString()];
		DM::Module *dest = modMap[le.InPort.UUID.toStdString()];
		std::string outPort = le.OutPort.PortName.toStdString();
		std::string inPort = le.InPort.PortName.toStdString();

		if(!src || !dest)
			DM::Logger(Error) << "corrupt link";
		else
		{
			// we may want to init for initializing ports
			if(!src->hasOutPort(outPort))
				src->init();
			if(!dest->hasInPort(inPort))
				dest->init();

			if(!addLink(src, outPort, dest, inPort, false))
				DM::Logger(Error) << "could not establish link between "
				<< src->getClassName() << ":" << outPort << " and "
				<< dest->getClassName() << ":" << inPort;
		}
	}
	Logger(Standard) << ">> checking stream";
	if(checkStream())
		Logger(Standard) << ">> checking stream finished successfully";
	else
		Logger(Error) << ">> error checking stream";

	Logger(Standard) << ">> loading simulation file finished";
	return true;
}

bool Simulation::loadSimulation(std::string filePath)
{
	Q_ASSERT(QFile::exists(QString::fromStdString(filePath)));
	QFile file(QString::fromStdString(filePath));
	std::map<std::string, DM::Module*> modMap;
	return loadSimulation(&file, QString::fromStdString(filePath), modMap);
}

void Simulation::writeSimulation(QIODevice* dest, QString filePath)
{
    SimulationWriter::writeSimulation(	dest, filePath, this->getSimulationConfig(),
		getModules(), getLinks());
}

void Simulation::writeSimulation(std::string filePath)
{
	QString qFilePath = QString::fromStdString(filePath);
	QFile file(qFilePath);
	writeSimulation(&file, qFilePath);
}

void Simulation::addObserver(SimulationObserver *obs)
{
	observers.push_back(obs);
}
void Simulation::removeObserver(SimulationObserver *obs)
{
	std::vector<SimulationObserver*>::iterator it = find(observers.begin(), observers.end(), obs);
	if(it != observers.end())
		observers.erase(it);
}



