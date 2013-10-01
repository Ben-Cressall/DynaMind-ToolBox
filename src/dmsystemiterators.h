/**
 * @file
 * @author  Gregor Burger <burger.gregor@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012  Gregor Burger
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef SYSTEM_ITERATORS_H
#define SYSTEM_ITERATORS_H

#include "dmsystem.h"
#include "dmview.h"
#include "dmnode.h"
#include "dmedge.h"
#include "dmface.h"
#include "dmcomponent.h"
#include "dmrasterdata.h"
#include "dmattribute.h"
#include "tbvectordata.h"
#include "cgalgeometry.h"
//#include "cgaltriangulation.h"
#include "triangulaterasterdata.h"

#include <dmlogger.h>

#include <QtGlobal>

enum iterator_pos {
	before,
	after,
	in_between
};

template<typename CB>
void iterate_components(DM::System *system, DM::View v, CB &callback = CB()) 
{
	foreach(DM::Component *cmp, system->getAllComponentsInView(v))
	{
		std::vector<DM::LinkAttribute> links = cmp->getAttribute("Geometry")->getLinks();
		/*std::vector<std::string> uuids;

		foreach (DM::LinkAttribute link, links)
			uuids.push_back(link.uuid);

		if (v.getType() == DM::FACE)
			uuids.push_back(cmp_uuid);*/

		callback(system, v, cmp, 0, 0, before);
		//foreach (std::string uuid, uuids) 
		//{
		//	DM::Face * f = system->getFace(uuid);
		if(DM::Face* f = (DM::Face*)cmp)
		{
			std::vector<double> c = f->getAttribute("color")->getDoubleVector();

			DM::Vector3 color;
			if (c.size() > 2)
				color = DM::Vector3(c[0],c[1],c[2]);

			// std::vector<DM::Node> nodes = DM::CGALGeometry::FaceTriangulation(system, f);
			std::vector<DM::Node> nodes;
			DM::CGALGeometry::FaceTriangulation(system, f, nodes);

			DM::Vector3 vec;
			foreach (const DM::Node &n, nodes) 
			{
				n.get(&vec.x);
				callback(system, v, cmp, &vec, &color, in_between);
			}
		}
		callback(system, v, cmp, 0, 0, after);
	}
}

template<typename CB>
void iterate_mesh(DM::System *system, DM::View v, CB &callback = CB()) 
{
	foreach(DM::Component *cmp, system->getAllComponentsInView(v))
	{
		DM::Face* f = (DM::Face*)cmp;

		callback(system, v, f, 0, 0, before);

		DM::Vector3 vec;
		foreach(DM::Node* n, f->getNodePointers())
		{
			n->get(&vec.x);
			callback(system, v, n, &vec, 0, in_between);
		}
		callback(system, v, f, 0, 0, after);
	}
}


template<typename CB>
void iterate_nodes(DM::System *system, DM::View v, CB &callback = CB()) 
{
	DM::Vector3 vec;
	foreach(DM::Component *cmp, system->getAllComponentsInView(v))
	{
		DM::Node *n = (DM::Node*)cmp;
		n->get(&vec.x);

		callback(system, v, n, 0, 0, before);
		callback(system, v, n, &vec, 0, in_between);
		callback(system, v, n, 0, 0, after);
	}

}

template<typename CB>
void rasterdata_triangluation_callback(DM::System *system, DM::RasterData * r, DM::View v, CB &callback)
{
	unsigned long Y = r->getHeight();
	unsigned long X = r->getWidth();

	unsigned long OX = r->getXOffset();
	unsigned long OY = r->getYOffset();

	double noData = r->getNoValue();
	double lX = r->getCellSizeX();
	double lY = r->getCellSizeY();

	double xp,xm,yp,ym;

	for (unsigned long  y = 0; y < Y; y++) 
	{
		for (unsigned long  x = 0; x < X; x++) 
		{
			double val = r->getCell(x,y);
			if (val == noData)
				continue;

			xp = (x+0.5) * lX + OX;
			xm = (x-0.5) * lX + OX;

			yp = (y+0.5) * lY + OY;
			ym = (y-0.5) * lY + OY;

			DM::Vector3 vec( xm , ym, val);
			callback(system, v, r, &vec, 0, in_between);
			vec.x = xp;
			vec.y = ym;
			callback(system, v, r, &vec, 0, in_between);
			vec.x = xm;
			vec.y = yp;
			callback(system, v, r, &vec, 0, in_between);
			//vec.x = xm;
			//vec.y = yp;
			callback(system, v, r, &vec, 0, in_between);
			vec.x = xp;
			vec.y = ym;
			callback(system, v, r, &vec, 0, in_between);
			vec.x = xp;
			vec.y = yp;
			callback(system, v, r, &vec, 0, in_between);
		}
	}
}

template<typename CB>
void iterate_rasterdata(DM::System *system, DM::View v, CB &callback = CB()) 
{
	//DM::ComponentMap cmp = system->getAllComponentsInView(v);
	DM::RasterData * r = 0;
	/*for (DM::ComponentMap::const_iterator it = cmp.begin();
		it != cmp.end();
		++it) {
			r = (DM::RasterData *) it->second;
	}*/
	std::vector<DM::Component*> cmps = system->getAllComponentsInView(v);
	if(cmps.size())
		r = (DM::RasterData*)cmps[0];

	callback(system, v, r, 0, 0, before);
	rasterdata_triangluation_callback(system, r, v, callback);
	callback(system, v, r, 0, 0, after);
}

template<typename CB>
void iterate_edges(DM::System *system, DM::View v, CB &callback = CB()) 
{
	DM::Vector3 vecStart;
	DM::Vector3 vecEnd;
	foreach(DM::Component *cmp, system->getAllComponentsInView(v))
	{
		DM::Edge *e = (DM::Edge*)cmp;
		DM::Node* n = e->getStartNode();
		n->get(&vecStart.x);
		n = e->getEndNode();
		n->get(&vecEnd.x);

		callback(system, v, e, 0, 0, before);
		callback(system, v, e, &vecStart, 0, in_between);
		callback(system, v, e, &vecEnd, 0, in_between);
		callback(system, v, e, 0, 0, after);
	}
}

template<typename CB>
void iterate_faces(DM::System *system, DM::View v, CB &callback = CB()) 
{
	DM::Vector3 vec;
	foreach(DM::Component *cmp, system->getAllComponentsInView(v))
	{
		DM::Face *f = (DM::Face*)cmp;

		callback(system, v, f, 0, 0, before);
		std::vector<DM::Node*> nodes = f->getNodePointers();
		//nodes.pop_back();
		foreach(DM::Node* n, nodes)
		{
			n->get(&vec.x);
			callback(system, v, f, &vec, 0, in_between);
		}

		callback(system, v, f, 0, 0, after);
	}
}


#endif // SYSTEM_ITERATORS_H
