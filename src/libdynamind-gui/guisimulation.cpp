/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011  Christian Urich
 
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

#include "guisimulation.h"
#include <dmmoduleregistry.h>
#include <modelnode.h>
#include <guiport.h>
#include <groupnode.h>
#include <rootgroupnode.h>
#include <dmlogger.h>

#include <guilink.h>
#include <dmsimulationreader.h>
#include <simulationio.h>

#ifndef PYTHON_EMBEDDING_DISABLED
#include <dmpythonenv.h>
#endif

GUISimulation::GUISimulation() : Simulation()
{

}
/*
void GUISimulation::changeGroupName(GroupNode * g) {
    emit GroupNameChanged(g);
}

GroupNode * GUISimulation::getGroupNode(DM::Group * g) {

    foreach (GroupNode * gn, this->groupNodes) {
        if (gn->getDMModel() == g)
            return gn;
    }

    return 0;
}*/
/*
void GUISimulation::GUIaddModule( DM::Module * m, QPointF pos)
{
	
    //Create Visual Representation of the Module
    if (!m->isGroup()) {
        ModelNode * node = new ModelNode(m, this);
        this->modelNodes.append(node);
        node->setPos(pos);
        connect(node, SIGNAL(showHelp(std::string, std::string)), this, SLOT(showHelp(std::string, std::string)));
        emit addedModule(node);
    }
    if (m->isGroup()) {
        GroupNode * node = new GroupNode(m, this);
        this->groupNodes.append(node);
        node->setPos(pos);
        connect(node, SIGNAL(showHelp(std::string, std::string)), this, SLOT(showHelp(std::string, std::string)));
        emit addedGroup(node);
    }

    this->updateSimulation();
	
}*/

void GUISimulation::registerRootNode() {
    //this->GUIaddModule(this->getRootGroup(), QPointF(0,0));
}
/*
void GUISimulation::GUIaddModule(QString name, QPointF pos, DM::Module *group)
{
    //Create Module in DynaMind
	
    DM::Module * m = this->addModule(name.toStdString());
    if (!m) {
        DM::Logger(DM::Error) << "Couldn't Create Module " << name.toStdString();
        return;
    }
    m->setGroup((DM::Group*)group);
    this->GUIaddModule(m, pos);
	
}
*/

/*
void GUISimulation::updateSimulation()
{    
    //this->startSimulation(true);
}

void GUISimulation::resetSimulation()
{
	this->resetSimulation();
    this->reloadModules();
    this->updateSimulation();
}*/

void GUISimulation::clearSimulation() 
{
	Simulation::reset();
	mforeach(ModelNode* m, modelNodes)
		m->deleteModelNode();

	modelNodes.clear();

	clear();

	/*
    RootGroupNode * rg = this->getGroupNode((DM::Group*)this->getRootGroup())->getRootGroupNode();

    foreach (ModelNode * m, rg->getChildNodes()) {
        rg->removeModelNode(m);
        m->deleteModelNode();
    }

    this->setSimulationStatus(DM::SIM_OK);
	*/
}

void GUISimulation::showHelp(string classname) 
{
    //emit showHelpForModule(classname);
}
/*
QVector<QDir> defaultModuleDirectories()
{
	QVector<QDir> cpv;
	cpv.push_back(QDir::currentPath() + "/Modules");
	cpv.push_back(QDir::currentPath() + "/bin/Modules");
#if defined DEBUG || _DEBUG
	cpv.push_back(QDir::currentPath() + "/../Modules/Debug");
#else
	cpv.push_back(QDir::currentPath() + "/../Modules/Release");
	cpv.push_back(QDir::currentPath() + "/../Modules/RelWithDebInfo");
	cpv.push_back(QDir::currentPath() + "/../../../output/Modules/Release");
	cpv.push_back(QDir::currentPath() + "/../../../output/Modules/RelWithDebInfo");
#endif

#ifndef PYTHON_EMBEDDING_DISABLED
	cpv.push_back(QDir::currentPath() + "/bin/PythonModules/scripts");
	cpv.push_back(QDir::currentPath() + "/PythonModules/scripts");
#endif
	return cpv;
}

void GUISimulation::loadModulesFromDefaultLocation()
{
	foreach (QDir cp, defaultModuleDirectories())  
		registerModulesFromDirectory(cp);
}
*/

ModelNode* GUISimulation::guiAddModule(QString moduleName)
{
	addModule(moduleName.toStdString());
	return lastAddedModuleNode;
}
DM::Module* GUISimulation::addModule(std::string moduleName, bool callInit)
{
	DM::Module* m = Simulation::addModule(moduleName, callInit);
	lastAddedModuleNode = new ModelNode(m, this);
	//lastAddedModuleNode->setPos(-100, -50);
	rootTab->addItem(lastAddedModuleNode);
	modelNodes[m] = lastAddedModuleNode;
	
	return m;
}

/*bool GUISimulation::addLink(PortNode* out, PortNode* in)
{
	return Simulation::addLink(out->getModule(), out->getPortName().toStdString(),
								in->getModule(), in->getPortName().toStdString());
}*/

bool GUISimulation::addLink(DM::Module* source, std::string outPort, 
							DM::Module* dest, std::string inPort)
{
	if(!Simulation::addLink(source, outPort, dest, inPort))
		return false;

	PortNode* inPortNode = getPortNode(dest, inPort, INPORT);
	PortNode* outPortNode = getPortNode(source, outPort, OUTPORT);

	if( !inPortNode || !outPortNode )
		return false;

	GUILink* gl = new GUILink();
	
	if( !gl )
		return false;

	inPortNode->scene()->addItem(gl);
	gl->setInPort(inPortNode);
	gl->setOutPort(outPortNode);
	
	// backlink for modelNode position update
	inPortNode->addLink(gl);
	outPortNode->addLink(gl);

	return true;
}

PortNode* GUISimulation::getPortNode(DM::Module* m, std::string portName, PortType type)
{
	ModelNode* mn;
	if(map_contains(&modelNodes, m, mn))
			return mn->getPort(portName, type);

	return NULL;
}

bool GUISimulation::removeLink(PortNode* out, PortNode* in)
{
	return Simulation::removeLink(out->getModule(), out->getPortName().toStdString(),
								in->getModule(), in->getPortName().toStdString());
}
/*
SimulationTab* GUISimulation::addTab(QWidget *parent)
{
	SimulationTab* tab = new SimulationTab(parent, this);
	tabs.append(tab);
	return tab;
}
void GUISimulation::closeTab(int i)
{
	if(i >= tabs.size() || i < 0)
		return;

	delete tabs[i];
	tabs.removeAt(i);
}
SimulationTab* GUISimulation::getTab(int i)
{
	if(i >= tabs.size() || i < 0)
		return NULL;

	return tabs[i];
}*/

/*void GUISimulation::guiUpdatePorts(ModelNode* node)
{
	DM::Module* m;
	if(map_contains(&moduleGuiMap, node, m))
	{
		// reset ports
		node->inPorts.clear();
		node->outPorts.clear();
		// get names from DM::Module
		foreach(std::string portName, m->getInPortNames())
			node->inPorts.push_back(QString::fromStdString(portName));

		foreach(std::string portName, m->getOutPortNames())
			node->outPorts.push_back(QString::fromStdString(portName));
	}
}*/



bool GUISimulation::loadSimulation(std::string filename) 
{
    SimulationReader simreader(QString::fromStdString(filename));
	SimulationIO simio(QString::fromStdString(filename));

	std::map<QString, ModuleExEntry> moduleExInfo = simio.getEntries();

	std::map<QString, DM::Module*> modMap;
	
	// load modules
	foreach(ModuleEntry me, simreader.getModules())
	{
		if(DM::Module* m = addModule(me.ClassName.toStdString()))
		{
			modMap[me.UUID] = m;
			// load parameters
			for(QMap<QString, QString>::iterator it = me.ParemterList.begin(); it != me.ParemterList.end(); ++it)
				m->setParameterValue(it.key().toStdString(), it.value().toStdString());

			ModuleExEntry* eex = &moduleExInfo[me.UUID];
			this->modelNodes[m]->setPos(eex->posX, eex->posY);
		}
		else
			DM::Logger(DM::Error) << "could not create module '" << me.ClassName.toStdString() << "'";
	}

	// load links
	foreach(LinkEntry le, simreader.getLinks())
	{
		DM::Module *src = modMap[le.OutPort.UUID];
		DM::Module *dest = modMap[le.InPort.UUID];
		std::string outPort = le.OutPort.PortName.toStdString();
		std::string inPort = le.InPort.PortName.toStdString();

		if(!src || !dest)
			DM::Logger(DM::Error) << "corrupt link";
		else
		{
			if(!addLink(src, outPort, dest, inPort))
				DM::Logger(DM::Error) << "could not establish link between "
				<< src->getClassName() << ":" << outPort << " and "
				<< dest->getClassName() << ":" << inPort;
		}
	}

	return true;
}
