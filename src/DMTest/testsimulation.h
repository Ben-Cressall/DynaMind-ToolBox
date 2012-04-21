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



#ifndef TESTSIMULATION_H
#define TESTSIMULATION_H

#include <QObject>
class TestSimulation : public QObject
{
    Q_OBJECT


private slots:
    /** @brief Test adding Module */
    void addModuleToSimulationTest();
    /** @brief Test if the a native module is loaded correctly */
    void loadModuleNativeTest();
    /** @brief Test repeated Simulation execution Module */
    void repeatedRunTest();
    /** @brief Test Linked Modules Module */
    void linkedModulesTest();
    /** @brief Test DynamicModuleLinkage
     *
     * Model setup TestModule - InOut - DynamicInOut - InOut2
     */
    void linkedDynamicModules();

    /** @brief Test DynamicModuleLinkage
     *
     * Model setup TestModule - InOut - |DynamicInOut| - InOut2
     */
    void linkedDynamicModulesOverGroups();

};

#endif // TESTSIMULATION_H
