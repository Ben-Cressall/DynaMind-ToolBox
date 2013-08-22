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

#ifndef PORTNODE_H
#define PORTNODE_H

#include "dmcompilersettings.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QString>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QColor>
#include <QObject>
#include <dmmodule.h>

#define PORT_DRAW_SIZE 14
#define PORT_DRAW_SELECTED_SIZE 16

class ModelNode;
class LinkNode;
class GUISimulation;


class DM_HELPER_DLL_EXPORT PortNode : public QGraphicsItem
{
public:
	PortNode(QString portName, DM::Module * m, DM::PortType type, 
		QGraphicsItem* parent, GUISimulation* simulation);
	~PortNode();

	void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
	void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	void mousePressEvent ( QGraphicsSceneMouseEvent * event );
	void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);
	void updatePos();
	void setHover(bool b);
	void removeLink(LinkNode * l);
	void addLink(LinkNode* l);

	GUISimulation*	getSimulation() const {return this->simulation;}
	DM::Module*		getModule()		const {return module;};
	DM::PortType	getType()		const {return portType;};
	QString			getPortName()	const {return this->portName;}

	QRectF	boundingRect()	const;
	QPointF getCenterPos()	const;
private:
	QVector<LinkNode * > linkNodes;
	LinkNode* unstableLink;

	QString portName;
	bool isHover;
	QGraphicsSimpleTextItem portLabel;
	GUISimulation * simulation;

	DM::PortType	portType;
	DM::Module*		module;
};

#endif // PORTNODE_H
