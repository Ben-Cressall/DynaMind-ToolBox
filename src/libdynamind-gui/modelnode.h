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

#ifndef MODELNODE_H
#define MODELNODE_H

#include "dmcompilersettings.h"
#include "moduledescription.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QString>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <dmmodule.h>
#include <dmport.h>
#include <dmsimulation.h>
#include <guiportobserver.h>
#include <QWidget>

class ModelNodeButton;
class RootGroupNode;
class PortNode;
class GUISimulation;
class  DM_HELPER_DLL_EXPORT ModelNode : public  QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    enum { Type = UserType + 1 };

private:
	//QString moduleName;
	bool hasPort(std::string portName);
public:
	QStringList inPorts;
	QStringList outPorts;


protected:
    GUIPortObserver guiPortObserver;
    float x1;
    float y1;
    float x2;
    float y2;
    float l;
    float h;
    int inputCounter;
    int outputCounter;
    QVector<PortNode*> ports;
    //QVector<ModelNode * > * nodes;
    RootGroupNode * parentGroup;

    //std::string  VIBeModuleUUID ;
    QGraphicsSimpleTextItem * simpleTextItem;

    int id;
    bool minimized;
    bool visible;

    //QColor Color;

    virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event ) ;
    virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
    virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    GUISimulation * simulation;

	DM::Module* module;

public:
    //QStringList ExistingInPorts;
    //QStringList ExistingOutPorts;
    //ModelNode(QGraphicsItem * parent = 0, QGraphicsScene * scene = 0);
    ModelNode(DM::Module* m, GUISimulation* sim);

    //int type() const {return Type; }
    virtual ~ModelNode();
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void setParentGroup(RootGroupNode * parentGroup){this->parentGroup = parentGroup;}

    int getID(){return this->id;}
    //QString getName(){return QString::fromStdString(this->getDMModel()->getName());}
    //void setID(int id){this->getDMModel()->setID(id); this->id = id;}

    //void addPort(DM::Port * p);
    //virtual PortNode * getGUIPort(DM::Port * p);
    //std::map<std::string, int> getParameters(){return this->getDMModel()->getParameterList();}

    //std::string getParameterAsString(std::string name);

    //void removePort(int Type, QString s);
    bool isMinimized(){return this->minimized;}

    virtual bool isGroup(){return false;}

    //virtual void setMinimized(bool b);


    bool GroupVisible(){return this->visible;}

    //View
    virtual void recalculateLandH(){}
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QRectF boundingRect() const;

    //DM::Module * getDMModel();
    GUISimulation * getSimulation() {return this->simulation;}
    //void  setSimulation(GUISimulation *s) {this->simulation = s;}
	
    //virtual void updatePorts();
    void updatePorts();

    virtual void setSelected ( bool selected ){QGraphicsItem::setSelected ( selected );}

    //virtual void resetModel();

    //std::string getGroupUUID();
	DM::Module* getModule() const
	{
		return module;
	}

public slots:
    //void addGroup();
    //void removeGroup();
    void deleteModelNode();
    void editModelNode();
    //void renameModelNode();
    void printData();
    void viewData(int portIndex);
    void showHelp();
	void showDialog();
    //void setDebug();
    //void setRelease();
    //void setResetModule();
   signals:
    void showHelp(std::string, std::string);
    //void showHelp(std::string);


};

#endif // MODELNODE_H
