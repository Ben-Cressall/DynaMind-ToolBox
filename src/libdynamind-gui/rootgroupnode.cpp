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

#include <rootgroupnode.h>
#include <QGraphicsDropShadowEffect>
#include <qgraphicsview.h>
#include <modelnode.h>
//#include <QDropEvent>
#include <QGraphicsSceneDragDropEvent>
#include <qtreewidget.h>
#include <guisimulation.h>
#include <dmsimulation.h>
#include <dmgroup.h>
#include <QKeyEvent>
#include <qapplication.h>
#include <qclipboard.h>
#include <dmsimulationwriter.h>
#include <simulationio.h>

SimulationTab::SimulationTab(QWidget* parent, GUISimulation *sim, DM::Group* parentGroup): 
	QGraphicsScene(parent), parentGroup(parentGroup)
{
	//scene = new QGraphicsScene(parent);
	viewer = new QGraphicsView(this, parent);
    viewer->setRenderHints(QPainter::Antialiasing);
    viewer->setAcceptDrops(true);

	this->sim = sim;
	hoveredGroupNode = NULL;
	//ModelNode* node = new ModelNode(0, sim);
	//scene->addItem(node);

	/*
	this->setGraphicsEffect(new  QGraphicsDropShadowEffect(this));
    this->setVisible(true);

    this->setFlag(QGraphicsItem::ItemIsSelectable, false);
    this->setFlag(QGraphicsItem::ItemIsMovable, false);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);*/

	//connect(this, SIGNAL(selectionChanged()), this, SLOT(clearSelection()));

	connect(sim->getTabWidget(), SIGNAL(currentChanged(int)), this, SLOT(clearSelection()));

	connect(this, SIGNAL(selectionChanged()), this, SLOT(enhanceSelection()));
}

void SimulationTab::enhanceSelection()
{
	foreach(QGraphicsItem* item, selectedItems())
		if(ModelNode* n = (ModelNode*)item)
		{
			DM::Module* group = n->getModule();
			if(group->isGroup())
				foreach(DM::Module* m, sim->getModules())
					if(m->getOwner() == group)
						sim->getModelNode(m)->setSelected(true);
		}
}

void SimulationTab::mousePressEvent(QGraphicsSceneMouseEvent *event) 
{
	if(!itemAt(event->scenePos()))
		foreach(SimulationTab* t, sim->getTabs())
			t->clearSelection();

	if(event->buttons() == Qt::LeftButton)
		viewer->setDragMode(QGraphicsView::ScrollHandDrag);
	else if(event->buttons() == Qt::RightButton)
		viewer->setDragMode(QGraphicsView::RubberBandDrag);

	QGraphicsScene::mousePressEvent(event);
}
void SimulationTab::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) 
{	
	if(hoveredGroupNode && selectedItems().size() > 0)
	{
		int i = 0;
		foreach(SimulationTab* tab, sim->getTabs())
		{
			if(tab->getParentGroup() == hoveredGroupNode->getModule())
			{
				copySelection();
				deleteSelection();
				sim->selectTab(i);
				tab->pasteSelection(QPointF(0,0));

				hoveredGroupNode->setHovered(false);
				hoveredGroupNode = NULL;
				break;
			}
			i++;
		}
	}

	viewer->setDragMode(QGraphicsView::NoDrag);
	QGraphicsScene::mouseReleaseEvent(event);
}

void SimulationTab::keyPressEvent(QKeyEvent * keyEvent )
{
	if(keyEvent->key() == Qt::Key_Delete)
		deleteSelection();
	else if (keyEvent->matches(QKeySequence::Copy))
		copySelection();
	else if (keyEvent->matches(QKeySequence::Paste))
	{	
		QByteArray data = QApplication::clipboard()->text().toUtf8();
		QBuffer buffer(&data);
		importSimulation(&buffer, cursorPos);
	}
	else if (keyEvent->matches(QKeySequence::Cut))
	{
		copySelection();
		deleteSelection();
	}
}

void SimulationTab::copySelection()
{
	std::list<DM::Module*> modules;
	std::set<DM::Module*> moduleSet;
	std::list<DM::Simulation::Link*> links;

	foreach(SimulationTab* t, sim->getTabs())
		foreach(QGraphicsItem* item, t->selectedItems())
		if(ModelNode* node = (ModelNode*)item)
		{
			modules.push_back(node->getModule());
			moduleSet.insert(node->getModule());
		}

		foreach(DM::Simulation::Link* l, sim->getLinks())
		{
			if(moduleSet.find(l->src) != moduleSet.end() &&
				moduleSet.find(l->dest) != moduleSet.end())
				links.push_back(l);
		}

		QByteArray data;
		QBuffer buffer(&data);
		DM::SimulationWriter::writeSimulation(&buffer, sim->currentDocument, modules, links, parentGroup);
		sim->appendGuiInformation(&buffer, modules);
		QApplication::clipboard()->setText(QString(data));
}

void SimulationTab::pasteSelection(const QPointF& pos)
{
	QByteArray data = QApplication::clipboard()->text().toUtf8();
	QBuffer buffer(&data);
	importSimulation(&buffer, cursorPos);
}

void SimulationTab::deleteSelection()
{
	foreach(QGraphicsItem* item, selectedItems())
		if(ModelNode* n = (ModelNode*)item)
			n->deleteModelNode();
}

void SimulationTab::importSimulation(QIODevice* source, const QPointF& target)
{
	clearSelection();

	float minx = 0;
	float miny = 0;

	std::map<std::string, DM::Module*> modMap;
	bool success = ((DM::Simulation*)sim)->loadSimulation(source,  sim->currentDocument, modMap, parentGroup, true);

	GuiSimulationReader simio(source);
	std::map<QString, ModuleExEntry> moduleExInfo = simio.getEntries();


	for(std::map<QString, ModuleExEntry>::iterator it = moduleExInfo.begin();
		it != moduleExInfo.end(); ++it)
	{
		if(modMap[it->first.toStdString()]->getOwner() == NULL)
		{
			minx = min(minx, (float)it->second.posX);
			miny = min(miny, (float)it->second.posY);
		}
	}

	minx -= target.x();
	miny -= target.y();

	for(std::map<QString, ModuleExEntry>::iterator it = moduleExInfo.begin();
		it != moduleExInfo.end(); ++it)
	{
		DM::Module* m = NULL;
		if(map_contains(&modMap, it->first.toStdString(), m) && m)
		{
			ModelNode* node = sim->getModelNode(m);
			if(m->getOwner() == NULL)
				node->setPos(QPointF(it->second.posX-minx, it->second.posY-miny));
			else
				node->setPos(QPointF(it->second.posX, it->second.posY));

			node->setSelected(true);
		}
	}

	sim->getTabWidget()->setCurrentWidget(this->getQGViewer());
}

void SimulationTab::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	cursorPos = event->scenePos();
	if(hoveredGroupNode)
	{
		hoveredGroupNode->setHovered(false);
		hoveredGroupNode = NULL;
	}

	if(event->buttons() == Qt::LeftButton)
		if(ModelNode* movingNode = dynamic_cast<ModelNode*>(itemAt(cursorPos)))
			foreach(QGraphicsItem* it, items(cursorPos))
			{
				ModelNode* node = dynamic_cast<ModelNode*>(it);
				if(node && !node->isGroup() && node != movingNode && node->getModule()->isGroup())
				{
					node->setHovered(true);
					hoveredGroupNode = node;
					break;
				}
			}

	QGraphicsScene::mouseMoveEvent(event);
}

SimulationTab::~SimulationTab()
{

}
void SimulationTab::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}

void SimulationTab::dropEvent(QGraphicsSceneDragDropEvent *event)
{
	DM::Group* parent = this->parentGroup;
	ModelNode* node = dynamic_cast<ModelNode*>(this->itemAt(event->scenePos()));
	if(node)
	{
		if(node->getModule()->isGroup())
			parent = (DM::Group*)node->getModule();
		else
			return;
	}

	DM::Logger(DM::Debug) << "tab drop";

    event->accept();

	QTreeWidget *moduleTree = dynamic_cast<QTreeWidget*>(event->source());

	if(!moduleTree || !moduleTree->currentItem())
		return;

	QTreeWidgetItem* item = moduleTree->currentItem();
	QString type = item->text(1);
	QString moduleName = item->text(0);

	if(type == "Module")
	{
		if(DM::Module* m = sim->addModule(moduleName.toStdString(), parent))
		{
			sim->getModelNode(m)->setPos(event->scenePos());	// move to cursor
		}

		// get module node from simulation
		//ModelNode* node = sim->guiAddModule(moduleName);
		// move to cursor
		//node->setPos(event->scenePos());
		// add module to render list
		//addItem(node);
		// get ports
		//sim->guiUpdatePorts(node);
	}
	else if(type == "Simulation")
	{
		QFile file(moduleName);
		importSimulation(&file, event->scenePos());
	}

    /*std::stringstream ss;
    QTreeWidget * lw = (QTreeWidget*) event->source();
    QString classname =  lw->currentItem()->text(0);
    std::string type = lw->currentItem()->text(1).toStdString();
    if (type.compare("Module") == 0) {
        emit NewModule(classname, event->scenePos(), this->rootGroup->getDMModel());
    } else {
        this->ResultViewer->importSimulation( lw->currentItem()->text(2), event->scenePos());
    }*/

}

void SimulationTab::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    event->accept();

	float s = (event->delta()>0?1:-1) * 0.1f;
	viewer->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	viewer->scale(1+s,1+s);
}

#define GROUPNODE_H
#ifndef GROUPNODE_H
#define GROUPNODE_H

#include "rootgroupnode.h"
#include <linknode.h>
#include <moduledescription.h>
#include <QGraphicsDropShadowEffect>

#include <ColorPalette.h>

#include <dmgroup.h>
#include <dmmodule.h>
#include <dmsimulation.h>
#include <guiport.h>
#include <dmporttuple.h>
#include <dmgroup.h>
#include <iostream>
#include <groupnode.h>


RootGroupNode::RootGroupNode()
{

}
RootGroupNode::~RootGroupNode() {

    DM::Logger(DM::Debug) << "Remove RootGroupNode ";

    if (this->getDMModel() == 0)
        return;
    while (this->childnodes.size() > 0)
        delete this->childnodes[0];

    //this->OutputTuplePorts.clear();
    //this->InPortTuplePorts.clear();




}


void RootGroupNode::removeModelNode(ModelNode *m) {
    int index = childnodes.indexOf(m);
    if (index > -1)
        this->childnodes.remove(index);
}

void RootGroupNode::changeGroupID(QString Name) {
    this->getName() = Name;
}


/*
void RootGroupNode::updatePorts () {
    DM::Group * g = (DM::Group*)this->getDMModel();

    foreach (DM::PortTuple * p,g->getInPortTuples()){
        this->addTuplePort(p);

    }
    foreach (DM::PortTuple * p,g->getOutPortTuples()){
        this->addTuplePort(p);

    }
    ModelNode::updatePorts();
}*/
/*
void RootGroupNode::addTuplePort(DM::PortTuple * p) {

    //Inport
    if  (p->getPortType() > DM::OUTPORTS) {
        foreach (QString pname, ExistingInPorts) {
            if (pname.compare(QString::fromStdString(p->getName())) == 0) {
                return;
            }
        }

        ExistingInPorts << QString::fromStdString(p->getName());
        PortNode * gui_p = new  PortNode(this, p->getOutPort());
        this->ports.append(gui_p);
        gui_p->setPos(0,gui_p->boundingRect().height()*this->inputCounter++);

        //Outport
    } else {
        foreach (QString pname, ExistingOutPorts) {
            if (pname.compare(QString::fromStdString(p->getName())) == 0) {
                return;
            }
        }
        ExistingOutPorts << QString::fromStdString(p->getName());
        PortNode *gui_p = new  PortNode(this, p->getInPort());
        this->ports.append(gui_p);
        gui_p->setPos(  l,gui_p->boundingRect().height()*this->outputCounter++);
    }


}*/
/*
PortNode *  RootGroupNode::getGUIPort(DM::Port * p) {
    foreach(GUIPortTuple * gui_pt,this->OutputTuplePorts) {
        if (gui_pt->inPort->getVIBePort() == p)
            return gui_pt->inPort;
        if (gui_pt->outPort->getVIBePort() == p)
            return gui_pt->outPort;
    }
    foreach(GUIPortTuple * gui_pt,this->InPortTuplePorts) {
        if (gui_pt->inPort->getVIBePort() == p)
            return gui_pt->inPort;
        if (gui_pt->outPort->getVIBePort() == p)
            return gui_pt->outPort;
    }



    return ModelNode::getGUIPort( p);

    return 0;
}*/

/*void RootGroupNode::removeTuplePort(int Type, QString s) {

}*/

RootGroupNode::RootGroupNode(  /*DM::Module *module,*/ GUISimulation * s): ModelNode( 0, s)
{

    this->childnodes = QVector<ModelNode*>();

    this->minimized = false;
    this->visible = true;
    this->setParentItem(0);
    this->outputCounter = 0;
    this->inputCounter = 0;
    this->x1 = 0;
    this->y1 = 0;
    this->x2 = 0;
    this->y2 = 0;


    this->outputCounter = 1;
    this->inputCounter = 1;
    this->setZValue(-1);
    this->setGraphicsEffect(new  QGraphicsDropShadowEffect(this));
    this->setVisible(true);

    this->setFlag(QGraphicsItem::ItemIsSelectable, false);
    this->setFlag(QGraphicsItem::ItemIsMovable, false);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    this->simpleTextItem = new QGraphicsSimpleTextItem (QString::number(id));
    double w = this->simpleTextItem->boundingRect().width()+40;
    w = w < 140 ? 140 : w;
    l = w+4;
    h =  this->simpleTextItem->boundingRect().height()+65;
    Color = COLOR_MODULE;
    updatePorts();
}
/*
void RootGroupNode::RePosTuplePorts() {
    foreach(PortNode * p, this->ports) {
        if (p->getPortType() > DM::OUTPORTS) {
            p->setPos(l, p->pos().y());
        }
    }
}*/

void RootGroupNode::setSelected(bool selected ) {
    /*foreach(ModelNode * m, this->childnodes) {
        m->setSelected(true);
        if (m->isGroup()) {
            GroupNode * g = (GroupNode *) m;
            g->setSelected(selected);
        }
    }*/

    QGraphicsItem::setSelected ( selected );


}

void RootGroupNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {

    QString name;
    //DM::Group;
    this->setGroupZValue();

    recalculateLandH();

    if(this->isSelected()== true) {
        painter->setBrush(Qt::gray);

    } else {
        painter->setBrush(Qt::white);
    }
    this->simpleTextItem->setText("Name:"+ QString::fromStdString(this->getDMModel()->getName()));
    if (simpleTextItem->boundingRect().width()+40 > l)
        l = simpleTextItem->boundingRect().width()+40;
    painter->drawRect(0, 0, l,h);

    this->setPos(x1-40, y1-20);

    painter->drawText(QPoint(5,15), "Name:"+ QString::fromStdString(this->getDMModel()->getName()));


    /*if((RePosFlag) != 0) {
        RePosTuplePorts();
        RePosFlag = false;
    }*/



}
QRectF RootGroupNode::boundingRect() const {
    return QRect(-100, -100, l+500, h+500);

}
void RootGroupNode::addModelNode(ModelNode *m) {
    this->childnodes.push_back(m);
    DM::Module * m1 = m->getDMModel();
    //m->getDMModel()->setGroup((DM::Group *)this->getDMModel());
    m->setParentGroup(this);
    this->recalculateLandH();
    this->update();

}
void RootGroupNode::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )  {
    this->setSelected(true);

    if (this->parentGroup != 0) {
        this->parentGroup->recalculateLandH();
        this->parentGroup->update();
    }

    ModelNode::QGraphicsItem::mouseMoveEvent(event);
}

void RootGroupNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    foreach(ModelNode * m, this->childnodes) {
        m->setSelected(false);
    }
}


void  RootGroupNode::setGroupZValue() {
    if (this->parentGroup != 0) {
        if (this->parentGroup->zValue()+1 != this->zValue())
            this->setZValue(this->parentGroup->zValue()+1);
    }
    foreach (ModelNode * m, this->childnodes ){
        DM::Module * m1 = m->getDMModel();
        if (m1 == 0)
            continue;
        //if(m->getDMModel()->isGroup()) {
        //    GroupNode * g = (GroupNode * ) m;
            //g->setGroupZValue();
        //} else {
            if (m->zValue() != this->zValue()+1)
                m->setZValue(this->zValue()+1);
        //}
    }
}

void RootGroupNode::recalculateLandH() {
    float lold = l;


    this->setFlag(QGraphicsItem::ItemIsMovable, true);


    if (this->childnodes.size() > 0) {
        x1 = childnodes[0]->sceneBoundingRect().x();
        y1 = childnodes[0]->sceneBoundingRect().y();
        x2 = childnodes[0]->sceneBoundingRect().x() + childnodes[0]->sceneBoundingRect().width();
        y2 = childnodes[0]->sceneBoundingRect().y() + childnodes[0]->sceneBoundingRect().height();
        for(int i = 0; i < childnodes.size(); i++) {
            ModelNode * m = childnodes.at(i);
            if (m->sceneBoundingRect().x() < x1)
                x1 = m->sceneBoundingRect().x();
            if (m->sceneBoundingRect().y() < y1)
                y1 = m->sceneBoundingRect().y();
            if (m->sceneBoundingRect().x()+m->sceneBoundingRect().width() > x2)
                x2 = m->sceneBoundingRect().x()+m->sceneBoundingRect().width();
            if (m->sceneBoundingRect().y()+m->sceneBoundingRect().height() > y2)
                y2 = m->sceneBoundingRect().y()+m->sceneBoundingRect().height();
        }
    }

    if(!this->minimized) {
        l = x2-x1+80;
        h =  y2-y1+40;
    } else {
        l = 100;
        h = 85;
    }
    if((lold - l) != 0) {
        RePosFlag = true;
        this->prepareGeometryChange();
        this->update(this->boundingRect());
    }
    if (this->parentGroup != 0) {
        this->parentGroup->recalculateLandH();
    }

}


#endif