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

#include "grouptest.h"
#include <guigrouptest.h>
#include <algorithm>
#include <dmporttuple.h>



DM_DECLARE_GROUP_NAME(GroupTest, Loops)
GroupTest::GroupTest() {
    i = 0;
    Runs = 1;

    this->addParameter("Runs", DM::INT, &Runs);
    this->addParameter("nameOfInViews", DM::STRING_LIST, &nameOfInViews);
    this->addParameter("nameOfOutViews", DM::STRING_LIST, &nameOfOutViews);



}

void GroupTest::run() {
    this->Steps = Runs;



    Group::run();
}

void GroupTest::init() {
    foreach (std::string s, nameOfInViews) {
        this->addTuplePort(s, DM::INTUPLESYSTEM);
    }
    foreach (std::string s, nameOfOutViews) {
        this->addTuplePort(s, DM::OUTTUPLESYSTEM);
    }

    //Remove Ports
     std::vector<PortTuple * > tuple_in_ports = this->getInPortTuples();
     std::vector<PortTuple * > tuple_remove;
     foreach (PortTuple * pt, tuple_in_ports) {
         bool exists = false;
         foreach (std::string s, nameOfInViews) {
         if (pt->getName().compare(s) == 0)
             exists = true;
         }
         if (!exists)
             tuple_remove.push_back(pt);
     }
     foreach (PortTuple * pt, tuple_remove) {
         this->removeTuplePort(pt);
     }

}

void GroupTest::addInPort(std::string n) {

    if (n.empty())
        return;
    //CheckIfPortAlreadyExists
    if (std::find(nameOfInViews.begin(), nameOfInViews.end(), n) != nameOfInViews.end())
        return;
    nameOfInViews.push_back(n);
    this->init();
}

void GroupTest::addOutPort(std::string n) {

    if (n.empty())
        return;
    //CheckIfPortAlreadyExists
    if (std::find(nameOfOutViews.begin(), nameOfOutViews.end(), n) != nameOfOutViews.end())
        return;
    nameOfOutViews.push_back(n);
    this->init();
}

void GroupTest::removeInPort(string port) {
    if (std::find(nameOfInViews.begin(), nameOfInViews.end(), port) == nameOfInViews.end())
        return;
    nameOfInViews.erase(std::find(nameOfInViews.begin(), nameOfInViews.end(), port) );
    this->init();
}

void GroupTest::removeOutPort(string port) {
    if (std::find(nameOfOutViews.begin(), nameOfOutViews.end(), port) == nameOfOutViews.end())
        return;
    nameOfOutViews.erase(std::find(nameOfOutViews.begin(), nameOfOutViews.end(), port) );
    this->init();
}

bool GroupTest::createInputDialog() {

    QWidget * w = new GUIGroupTest(this);
    w->show();
    return true;
}
