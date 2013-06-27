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

#ifndef GROUPNODE_H
#define GROUPNODE_H

#include <modelnode.h>
/*#include <moduledescription.h>
#include "dmcompilersettings.h"
#include <rootgroupnode.h>
namespace DM {
    class Module;
    class PortTuple;
}

class LinkNode;
struct LinkNodeTuple;


struct GUIPortTuple {
    PortNode * inPort;
    PortNode * outPort;
};*/

class DM_HELPER_DLL_EXPORT  GroupNode : public  QObject, public QGraphicsItem//, public DM::ModuleObserver
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

private:
	/*
    QVector<GUIPortTuple * > OutputTuplePorts;
    QVector<GUIPortTuple * > InPortTuplePorts;

    ModuleDescription module;
    bool RePosFlag;
    std::string name;
    QVector<ModelNode * > childnodes;
    std::string UUID;
    RootGroupNode * rg;
	*/

	// these ports are inside the group, providing data 
	// flow from outside to inside and vice versa
	QVector<PortNode*>	groupPorts;
	// the sice of the outer group border
	int w,h;
public:
	// observer methods
	//void notifyAddPort(const std::string &name, const DM::PortType type);
	//void notifyRemovePort(const std::string &name, const DM::PortType type);

    GroupNode(DM::Module* parent, GUISimulation* sim);
	/*
    virtual ~GroupNode();
    void addTuplePort(DM::PortTuple * p);
    void removeTuplePort(int Type, QString s);
    GroupNode( DM::Module *module, GUISimulation * s);
     virtual PortNode * getGUIPort(DM::Port * p);

	 */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	
    QRectF boundingRect() const;
    /*void setSelected ( bool selected );
    void RePosTuplePorts();

    bool isGroup(){return true;}

    void changeGroupID(QString Name);

    void removeModelNode(ModelNode *m);
    QVector<ModelNode * > getChildNodes() {return this->childnodes;}
    virtual void updatePorts();
    void setRootGroupNode(RootGroupNode * rg) {this->rg = rg;}
    RootGroupNode * getRootGroupNode() {return this->rg;}

public slots:
    void minimize();
    void maximize();

signals:
  void removeGroupNode(QString UUID);*/
	//void updatePorts();
};

#endif // GROUPNODE_H
