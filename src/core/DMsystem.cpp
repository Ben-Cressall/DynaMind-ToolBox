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

#include <DMattribute.h>
#include <DMcomponent.h>
#include <DMnode.h>
#include <DMedge.h>
#include <dmface.h>
#include <rasterdata.h>
#include <DMsystem.h>
#include <vibe_logger.h>

using namespace DM;

System::System(std::string name, const DM::View & view ) : Component()
{

}



void System::updateViews(Component * c) {
    foreach (std::string view, c->getInViews()) {
        this->views[view][c->getName()] = c;
    }
}

std::vector<std::string> System::getNamesOfViews() {
    std::vector<std::string> names;

    for ( std::map<std::string, View>::const_iterator it = this->viewdefinitions.begin(); it != this->viewdefinitions.end(); ++it  ) {
        names.push_back(it->first);
    }

    return names;

}
DM::View  System::getViewDefinition(string name) {

    return viewdefinitions[name];
}

System::System(const System& s) : Component(s)
{

    subsystems=s.subsystems;
    nodes=s.nodes;
    edges=s.edges;
    faces = s.faces;
    rasterdata = s.rasterdata;

    viewdefinitions = s.viewdefinitions;

    //Update SubSystem
    std::map<std::string,System*>::iterator its;

    for ( its=subsystems.begin() ; its != subsystems.end(); its++ ) {
        System * s = static_cast<System*>(ownedchilds[(*its).first]);
        subsystems[(*its).first]=s;
        this->updateViews(s);

    }

    //Update Nodes
    std::map<std::string,Node*>::iterator itn;

    for ( itn=nodes.begin() ; itn != nodes.end(); itn++ ) {
        Node * n = static_cast<Node*>(ownedchilds[(*itn).first]);
        nodes[(*itn).first] = n;
        this->updateViews(n);
    }

    //Update Edges
    std::map<std::string,Edge*>::iterator ite;

    for ( ite=edges.begin() ; ite != edges.end(); ite++ ) {
        Edge * e = static_cast<Edge*>(ownedchilds[(*ite).first]);
        edges[(*ite).first]= e;
        this->updateViews(e);
    }

    //Update Faces
    std::map<std::string,Face*>::iterator itf;

    for ( itf=faces.begin() ; itf != faces.end(); itf++ ) {
        Face * f = static_cast<Face*>(ownedchilds[(*itf).first]);
        faces[(*itf).first]= f;
        this->updateViews(f);
    }

}

System::~System()
{

}

Node * System::addNode(Node* node)
{
    if(!addChild(node))
        return 0;

    nodes[node->getName()]=node;
    return node;
}

RasterData * System::addRasterData(RasterData *r)
{
    if(!addChild(r))
        return 0;

    rasterdata[r->getName()] = r;
    return r;
}

System* System::createSubSystem(std::string name,const DM::View & view)
{
    System* newsystem = new System(name, view);
    this->addSubSystem(newsystem,view);
    return newsystem;
}

Node * System::addNode(double x, double y, double z,  const DM::View & view) {

    Node * n = this->addNode(new Node(x, y, z));

    if (n == 0)
        return 0;
    if (!view.getName().empty()) {
        this->views[view.getName()][n->getName()] = n;
        n->setView(view.getName());
    }


    return n;
}

Edge * System::addEdge(Edge* edge)
{
    if(!getNode(edge->getStartpointName()) || !getNode(edge->getEndpointName()))
        return 0;

    if(!addChild(edge))
        return 0;

    edges[edge->getName()]=edge;
    return edge;
}
Edge * System::addEdge(Node * start, Node * end, const View &view)
{
    Edge * e = this->addEdge(new Edge(start->getName(), end->getName()));

    if (e == 0)
        return 0;
    if (!view.getName().empty()) {
        this->views[view.getName()][e->getName()] = e;
        e->setView(view.getName());
    }
    return e;
}

Face * System::addFace(Face *f) {
    if(!addChild(f))
        return 0;

    faces[f->getName()]=f;
    return f;
}

Face * System::addFace(std::vector<Edge*> edges,  const DM::View & view)
{


    std::vector<std::string> stringEdges;

    foreach (Edge* e, edges)
        stringEdges.push_back(e->getName());


    Face * f = this->addFace(new Face(stringEdges));

    if (f == 0)
        return 0;
    if (!view.getName().empty()) {
        this->views[view.getName()][f->getName()] = f;
        f->setView(view.getName());
    }
    return f;
}

Node* System::getNode(std::string name)
{
    if(nodes.find(name)==nodes.end())
        return 0;

    return nodes[name];
}

Edge* System::getEdge(std::string name)
{
    if(edges.find(name)==edges.end())
        return 0;

    return edges[name];
}
Face* System::getFace(std::string name)
{
    if(faces.find(name)==faces.end())
        return 0;

    return faces[name];
}


Component * System::getComponent(std::string name) {
    if(nodes.find(name)!=nodes.end())
        return nodes[name];
    if(edges.find(name)!=edges.end())
        return edges[name];
    if(faces.find(name)!=faces.end())
        return faces[name];
    if(subsystems.find(name)!=subsystems.end())
        return subsystems[name];
    if(rasterdata.find(name)!=rasterdata.end())
        return rasterdata[name];
    return 0;

}

bool System::removeFace(std::string name)
{
    //check if name is a edge instance
    if(faces.find(name)==faces.end())
        return false;

    if(!removeChild(name))
        return false;

    faces.erase(name);
    return true;
}

bool System::removeEdge(std::string name)
{
    //check if name is a edge instance
    if(edges.find(name)==edges.end())
        return false;

    if(!removeChild(name))
        return false;

    edges.erase(name);
    return true;
}

bool System::removeNode(std::string name)
{
    //check if name is a node instance
    if(nodes.find(name)==nodes.end())
        return false;

    //remove node
    if(!removeChild(name))
        return false;

    nodes.erase(name);

    //find all connected edges and remove them
    std::vector<std::string> connectededges;

    std::map<std::string,Edge*>::iterator ite;


    for ( ite=edges.begin() ; ite != edges.end(); ite++ )
    {
        Edge* tmpedge = edges[(*ite).first];

        if(!tmpedge->getStartpointName().compare(name) || !tmpedge->getEndpointName().compare(name))
            connectededges.push_back(tmpedge->getName());
    }

    for(int index=0; index<connectededges.size(); index++)
    {
        if(!removeEdge(connectededges[index]))
            return false;
    }

    return true;
}
bool System::addComponentToView(Component *comp, const View &view) {
    this->views[view.getName()][comp->getName()] = comp;
    comp->setView(view.getName());
}

bool System::addSubSystem(System *newsystem,  const DM::View & view)
{
    //TODO add View to subsystem
    if(!addChild(newsystem))
        return false;

    subsystems[newsystem->getName()]=newsystem;



    if (!view.getName().empty()) {
        this->views[view.getName()][newsystem->getName()] = newsystem;
        newsystem->setView(view.getName());
    }

    return true;
}

bool System::removeSubSystem(std::string name)
{
    if(!removeChild(name))
        return false;

    subsystems.erase(name);

    return true;
}
std::map<std::string, Component*> System::getAllComponentsInView(DM::View & view) {

    return views[view.getName()];
}
std::vector<std::string> System::getNamesOfComponentsInView(DM::View & view) {



    std::vector<std::string> names;


    std::map<std::string, DM::Component*> components_view = this->getAllComponentsInView(view);
    for (std::map<std::string, DM::Component*>::const_iterator it = components_view.begin(); it != components_view.end(); ++it) {
        names.push_back(it->first);
    }
    return names;
}
System* System::getSubSystem(std::string name)
{
    if(subsystems.find(name)==subsystems.end())
        return 0;

    return subsystems[name];
}

std::map<std::string, System*> System::getAllSubSystems()
{
    return subsystems;
}

System* System::createSuccessor()
{
    System* result = new System(*this);
    predecessors.push_back(this);
    return result;
}

Component* System::clone()
{
    return new System(*this);
}

bool System::addView(View view)
{

    //For each view one dummy element will be created
    //Check for existing View
    DM::View  existingView = this->viewdefinitions[view.getName()];


    if (!view.writes()) {
        return true;
    }
    DM::Component * dummy  = 0;
    if (!existingView.getIdOfDummyComponent().empty()) {
        dummy = this->getComponent(existingView.getIdOfDummyComponent());
    } else {

        if ( DM::NODE == view.getType()) {
            dummy = this->addNode(0,0,0);
        }
        if (  DM::EDGE == view.getType()) {
            DM::Node * n1 =this->addNode(0,0,0);
            DM::Node * n2 =this->addNode(0,0,0);
            dummy = this->addEdge(n1,n2);
        }
        if (  DM::FACE == view.getType()) {
            DM::Node * n1 =this->addNode(0,0,0);
            DM::Node * n2 =this->addNode(0,0,0);
            DM::Edge * e1 = this->addEdge(n1,n2);
            std::vector<Edge*> ve;
            ve.push_back(e1);
            dummy = this->addFace(ve);
        }
        if (  DM::SUBSYSTEM == view.getType()) {
            dummy = new DM::System(view.getName());
            this->addSubSystem((DM::System*) dummy);
        }
        if (  DM::RASTERDATA == view.getType()) {
            dummy = new DM::RasterData();
            this->addRasterData((DM::RasterData*) dummy);
        }
    }
    view.setIdOfDummyComponent(dummy->getName());


    //extend Dummy Attribute
    foreach (std::string a , view.getWriteAttributes()) {
        dummy->addAttribute(DM::Attribute(a));
    }

    this->viewdefinitions[view.getName()] = view;

    return true;
}

std::vector<System*> System::getPredecessorStates()
{
    return predecessors;
}

std::map<std::string, Node*> System::getAllNodes()
{
    return nodes;
}

std::map<std::string, Edge*> System::getAllEdges()
{
    return edges;
}

std::map<std::string, Face*> System::getAllFaces()
{
    return faces;
}

const std::vector<std::string> System::getViews()  {

    std::vector<std::string> viewlist;

    for (std::map<std::string, View>::const_iterator it = viewdefinitions.begin(); it != viewdefinitions.end(); ++it) {
        viewlist.push_back(it->first);
    }


    return viewlist;

}
