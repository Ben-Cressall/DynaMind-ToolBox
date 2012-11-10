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
#include "dmattribute.h"
#include "tbvectordata.h"
#include "cgalgeometry.h"

#include <QtGlobal>

enum iterator_pos {
    before,
    after,
    in_between
};

template<typename CB>
void iterate_components(DM::System *system, DM::View v, CB &callback = CB()) {
    foreach(std::string cmp_uuid, system->getUUIDsOfComponentsInView(v)) {
        DM::Component *cmp = system->getComponent(cmp_uuid);

        std::vector<DM::LinkAttribute> links = cmp->getAttribute("Geometry")->getLinks();

        callback(system, v, cmp,0, before);
        foreach (DM::LinkAttribute link, links) {
            DM::Face * f = system->getFace(link.uuid);
            std::vector<double> c = f->getAttribute("color")->getDoubleVector();
            int size_c = c.size();
            DM::Node color;
            if (c.size() > 2) {
                color.setX(c[0]);
                color.setY(c[1]);
                color.setZ(c[2]);
            }
            std::vector<DM::Node> nodes = DM::CGALGeometry::FaceTriangulation(system, f);
            foreach (DM::Node n, nodes) {
                n.addAttribute("r", color.getX());
                n.addAttribute("g", color.getY());
                n.addAttribute("b", color.getZ());
                callback(system, v, cmp, &n, in_between);
            }
        }
        callback(system, v, cmp, 0, after);
    }
}

template<typename CB> 
void iterate_nodes(DM::System *system, DM::View v, CB &callback = CB()) {
    
    foreach(std::string node_uuid, system->getUUIDsOfComponentsInView(v)) {
        DM::Node *n = system->getNode(node_uuid);
        callback(system, v, n, 0, before);
        callback(system, v, n, n, in_between);
        callback(system, v, n, 0, after);
    }
    
}

template<typename CB> 
void iterate_edges(DM::System *system, DM::View v, CB &callback = CB()) {
    foreach(std::string edge_uuid, system->getUUIDsOfComponentsInView(v)) {
        DM::Edge *e = system->getEdge(edge_uuid);
        
        callback(system, v, e, 0, before);
        
        DM::Node *n = system->getNode(e->getStartpointName());
        callback(system, v, e, n, in_between);
        
        n = system->getNode(e->getEndpointName());
        callback(system, v, e, n, in_between);
        
        callback(system, v, e, 0, after);
    }
}

template<typename CB> 
void iterate_faces(DM::System *system, DM::View v, CB &callback = CB()) {
    foreach(std::string face_uuid, system->getUUIDsOfComponentsInView(v)) {
        DM::Face *f = system->getFace(face_uuid);

        std::vector<std::string> nodes = f->getNodes();
        nodes.pop_back();
        
        callback(system, v, f, 0, before);
        
        foreach(std::string node_uuid, nodes) {
            DM::Node *n = system->getNode(node_uuid);
            callback(system, v, f, n, in_between);
        }
        
        callback(system, v, f, 0, after);
    }
}

#endif // SYSTEM_ITERATORS_H
