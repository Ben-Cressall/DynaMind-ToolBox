/**
 * @file
 * @author  Michael Mair <michael.mair@uibk.ac.at>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012  Michael Mair

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

#ifndef EpanetDynamindConverter_H
#define EpanetDynamindConverter_H

#include <dm.h>
#include <watersupplyviewdef.h>
#include <epanetmodelcreator.h>

class EpanetDynamindConverter
{

private:
	DM::WS::ViewDefinitionHelper wsd;
	QMap<DM::Component*,uint> components;
	EPANETModelCreator creator;
	bool openedepanetfile;

public:
	EpanetDynamindConverter();
	EPANETModelCreator* getCreator();
	bool createEpanetModel(DM::System *sys, std::string inpfilepath);
	bool mapEpanetAttributes(DM::System *sys);
	bool mapPipeAttributes(DM::System *sys);
	bool mapJunctionAttributes(DM::System *sys);
	bool checkENRet(int ret);
	bool openEpanetModel(std::string inpfilepath, string rptfilepath);
	bool closeEpanetModel();
	int getEpanetNodeID(DM::Component* component);
	int getEpanetLinkID(DM::Component *component);

	//NODE COMPONENTS OF EPANET
	bool addJunction(DM::Node *junction);
	bool addReservoir(DM::Node *reservoir);
	bool addTank(DM::Node *tank);

	//LINK COMPONENTS OF EPANET
	bool addPipe(DM::Edge *pipe, bool cv);
	~EpanetDynamindConverter();

	//HELP FUNCTIONS
	std::vector<DM::Node*> getFlowNeighbours(DM::Node* junction);
	std::vector<DM::Node*> getInverseFlowNeighbours(DM::Node* junction);
	DM::Node* getNearestPressure(DM::Node* currentpressurepoint, std::vector<DM::Node*> &nodes);
	DM::Node* getNearestFlowPoint(DM::Node* currentpoint, std::vector<DM::Node*> &nodes);
	double calcDiameter(double k, double l, double q, double h, double maxdiameter,bool discretediameters,bool nearestdiscretediameter);
	double calcFrictionHeadLoss(double d, double k, double l, double q);
	double calcLambda(double k, double d, double q, double lambda = 0);
};

#endif // EpanetDynamindConverter_H
