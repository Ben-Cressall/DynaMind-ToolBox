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

#ifndef SYSTEM_H
#define SYSTEM_H

#include <map>
#include <vector>
#include <DMview.h>
namespace DM {
    enum Components {
        NODE,
        EDGE,
        FACE,
        SUBSYSTEM,
        RASTERDATA
    };

    class Component;
    class Node;
    class Edge;
    class Face;
    class RasterData;

    class System : public Component
    {
    private:
        std::vector<System*> predecessors;
        std::map<std::string, Node* > nodes;
        std::map<std::string, Edge* > edges;
        std::map<std::string, Face* > faces;
        std::map<std::string, RasterData *> rasterdata;
        std::map<std::string, System*> subsystems;
        std::map<std::string, View> viewdefinitions;
        std::map<std::string, std::map<std::string, Component*> > views;
        RasterData * addRasterData(RasterData * r);
        void updateViews (Component * c);

    public:
        System(std::string name, std::string view = "");
        System(const System& s);
        ~System();

        Node * addNode(Node* node);
        Node * addNode(double x, double y, double z, std::string view = "");
        Edge* addEdge(Edge* edge);
        Edge* addEdge(Node * start, Node * end, std::string view = "");
        Face * addFace(Face * f);
        Face * addFace(std::vector<Edge*> edges, std::string view = "");
        Node* getNode(std::string name);
        Edge* getEdge(std::string name);
        Face * getFace(std::string name);
        bool removeEdge(std::string name);
        bool removeNode(std::string name);
        bool removeFace(std::string name);
        std::map<std::string, Node*> getAllNodes();
        std::map<std::string, Edge*> getAllEdges();
        std::map<std::string, Face*> getAllFaces();
        std::vector<System*> getPredecessorStates();
        bool addSubSystem(System *newsystem, std::string view = "");
        System* createSubSystem(std::string name, std::string view);
        bool removeSubSystem(std::string name);
        System* getSubSystem(std::string name);
        std::map<std::string, System*> getAllSubSystems();
        System* createSuccessor();
        bool addView(DM::View view);
        std::vector<std::string> getNamesOfViews();
        Component * getComponent(std::string name);
        DM::View getViewDefinition(std::string name);
        Component* clone();
        const std::vector<std::string> getViews();

        std::map<std::string, Component*> getAllComponentsInView(std::string view);
        std::vector<std::string> getNamesOfComponentsInView(std::string view);




    };
}
#endif // SYSTEM_H
