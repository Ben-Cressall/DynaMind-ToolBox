/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of VIBe2
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

#ifndef SIMULATION_H
#define SIMULATION_H

#include "compilersettings.h"
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <QThread>
#include <rootgroup.h>


namespace vibens {
    struct SimulationPrivate;
    class DataObserver;
    class SimulationObserver;
    class Module;
    class Port;
    class DataBase;
    class ModuleRegistry;
    class ModuleLink;
    class Group;

    class VIBE_HELPER_DLL_EXPORT Simulation:public QThread {
    public:
        Simulation(std::string fileName, std::vector<std::string> pythonModules);
        Simulation();
        virtual ~Simulation();

        void run(bool check = true);
        void addDataObserver(DataObserver*  observer);
        void addSimulationObserver(SimulationObserver * simulationObserver);
        DataBase * getDataBase();
        Module * getRootGroup(){return &rootGroup;}

        ModuleLink * addLink(Port * OutPort,Port * InPort);
        void removeLink(ModuleLink * );
        Module * addModule(std::string ModuleName);
        void removeModule(std::string UUid);
        void registerDataBase(DataBase * database);
        ModuleRegistry * getModuleRegistry();

        std::vector<Group*> getGroups();
        void writeSimulation(std::string filename);
        std::vector<Module*> getModules() {return this->Modules;}
        std::map<std::string, std::string>  loadSimulation(std::string FileName);
        Module * getModuleWithUUID(std::string UUID);
        Module * getModuleByName(std::string name);
        std::vector<ModuleLink*> getLinks();
        Module * resetModule(std::string UUID);
        void resetModules();
        bool checkConnections() const;
        void reloadModules();
        std::vector<Module*> getModulesFromType(std::string name);
        void deregisterModule(std::string UUID);

    private:
        RootGroup rootGroup;
        std::vector<Module*> Modules;
        //std::vector<ModuleLink * > Links;
        SimulationPrivate *data;
        DataBase * database;
        ModuleRegistry * moduleRegistry;
        void removeLinksFromModule(Module *);



    };
}
#endif // SIMULATION_H
