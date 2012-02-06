/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011-2012  Christian Urich
 
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

#include "tbvectordata.h"
#include "DM.h"
#include <QtGlobal>

DM::Edge * TBVectorData::getEdge(DM::System * sys, DM::View & view, DM::Node * n1, DM::Node * n2, bool OrientationMatters) {

    DM::ComponentMap cmap = sys->getAllComponentsInView(view);
    for (DM::ComponentMap::const_iterator it = cmap.begin(); it != cmap.end(); ++it) {
        DM::Edge * e = (DM::Edge*) it->second;
        DM::Node * n1_1 = sys->getNode(e->getStartpointName());
        DM::Node * n2_1 = sys->getNode(e->getEndpointName());

        if (n1 == n1_1 && n2 == n2_1)
            return e;
        if (!OrientationMatters && n1 == n2_1 && n2 == n1_1)
            return e;

    }

    return 0;
}

DM::Edge * TBVectorData::getEdge(DM::System * sys, DM::View & view, DM::Edge * e, bool OrientationMatters) {

    return TBVectorData::getEdge(sys,
                                 view,
                                 (DM::Node *)  sys->getComponent(e->getStartpointName()),
                                 (DM::Node *) sys->getComponent(e->getEndpointName()),
                                 OrientationMatters);
}

DM::Node * TBVectorData::getNode2D(DM::System *sys, DM::View &view, DM::Node n, double err) {

    DM::ComponentMap cmap = sys->getAllComponentsInView(view);
    for (DM::ComponentMap::const_iterator it = cmap.begin(); it != cmap.end(); ++it) {
        DM::Node * n_1 = (DM::Node *) it->second;
        if (n_1->compare2d(n, err))
            return n_1;
    }
    return 0;

}

DM::Node * TBVectorData::addNodeToSystem2D(DM::System *sys,DM::View &view,DM::Node n1,  bool CheckForExisting, double err) {
    DM::Node * new_Node = 0;


    new_Node = TBVectorData::getNode2D(sys, view, n1, err);
    if (new_Node != 0 && CheckForExisting == true) {
        return new_Node;
    }

    new_Node =  sys->addNode(n1.getX(), n1.getY(), n1.getZ(), view);

    return new_Node;
}


std::vector<DM::Edge*> TBVectorData::getConnectedEdges(DM::System *sys, DM::View &view, DM::Node n1, double err) {

    std::vector<DM::Edge*> result;
    std::vector<std::string> names = sys->getNamesOfComponentsInView(view);

    foreach (std::string id, names ) {
        std::vector<DM::Node * > nodes;
        DM::Edge * e = sys->getEdge(id);
        nodes.push_back(sys->getNode(e->getStartpointName()));
        nodes.push_back(sys->getNode(e->getEndpointName()));

        foreach (DM::Node * n, nodes) {
            if (n->compare2d(n1, err))
                result.push_back(e);
        }
    }

    return result;

}


std::vector<DM::Node*> TBVectorData::getNodeListFromFace(DM::System *sys, DM::Face *face) {
    std::vector<DM::Node*> result;
    std::vector<std::string> edgelist = face->getEdges();
    foreach (std::string eid, edgelist) {
        DM::Edge * e = sys->getEdge(eid);
        result.push_back(sys->getNode(e->getStartpointName()));

    }
    return result;
}

void TBVectorData::splitEdge(DM::System *sys, DM::Edge *e, DM::Node *n, DM::View &view) {

    DM::Edge * e1 = new DM::Edge(*e);
    std::set<std::string> views = e1->getInViews();
    sys->addEdge(e1);
    foreach (std::string v, views) {
        sys->removeComponentFromView(e1, sys->getViewDefinition(v));

    }
    e1->setEndpointName(n->getName());
    sys->addComponentToView(e1, view);

    DM::Edge * e2 = new DM::Edge(*e);
    sys->addEdge(e2);
    views = e2->getInViews();
    foreach (std::string v, views) {
        sys->removeComponentFromView(e2, sys->getViewDefinition(v));
    }
    e2->setStartpointName(n->getName());
    sys->addComponentToView(e2, view);

    sys->removeComponentFromView(e, view);


}
