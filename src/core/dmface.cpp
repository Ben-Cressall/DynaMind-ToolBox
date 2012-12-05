/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011  Christian Urich, Markus Sengthaler
 
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

#include <dmcomponent.h>
#include "dmface.h"
#include "dmnode.h"
#include "dmsystem.h"
#include "dmlogger.h"

#include "dmdbconnector.h"
#include <QByteArray>

using namespace DM;

/*
QByteArray GetBytes(std::vector<std::string> stringvector)
{
    QByteArray qba;
    QDataStream stream(&qba, QIODevice::WriteOnly);

    stream << (int)stringvector.size();
    for(unsigned int i=0;i<stringvector.size();i++)
        stream << QUuid(QString::fromStdString(stringvector[i]));

    return qba;
}*/

QByteArray GetBytes(std::vector<Node*> nodevector)
{
    QByteArray qba;
    QDataStream stream(&qba, QIODevice::WriteOnly);

    stream << (int)nodevector.size();
    for(unsigned int i=0;i<nodevector.size();i++)
        stream << nodevector[i]->getQUUID();

    return qba;
}
/*
QByteArray GetBytes(std::vector<std::vector<std::string> > stringvectorvector)
{
    QByteArray qba;
    QDataStream stream(&qba, QIODevice::WriteOnly);

    stream << (int)stringvectorvector.size();
    for(unsigned int i=0;i<stringvectorvector.size();i++)
    {
        stream << (int)stringvectorvector[i].size();
        for(unsigned int j=0;j<stringvectorvector[i].size();j++)
                stream << QUuid(QString::fromStdString(stringvectorvector[i][j]));
    }
    return qba;
}*/

QByteArray GetBytes(std::vector<Face*> facevector)
{
    QByteArray qba;
    QDataStream stream(&qba, QIODevice::WriteOnly);

    stream << (int)facevector.size();
    for(unsigned int i=0;i<facevector.size();i++)
    {
        stream << GetBytes(facevector[i]->getNodePointers());
    }
    return qba;
}

Face::Face(std::vector<std::string> nodes) : Component(true)
{
    Logger(Error) << "Warning: Face::Face(std::vector<std::string> nodes)\
                         doesnt work anymore, use Face::Face(std::vector<Node*> nodes) instead";
    /*
    foreach(std::string nodeUuid, nodes)
    {
        _nodes.push_back(this->getCurrentSystem()->getNode(nodeUuid));
    }

    DBConnector::getInstance()->Insert("faces", uuid.toRfc4122(),
                                       "nodes", GetBytes(_nodes));*/
}

Face::Face(std::vector<Node*> nodes) : Component(true)
{
    this->_nodes = nodes;
    DBConnector::getInstance()->Insert("faces", uuid.toRfc4122(),
                                       "nodes", GetBytes(_nodes));
}

Face::Face(const Face& e) : Component(e, true)
{
    this->_nodes = e._nodes;
    this->_holes = e._holes;
    DBConnector::getInstance()->Insert("faces", uuid.toRfc4122(),
                                       "nodes", GetBytes(e._nodes),
                                       "holes", GetBytes(e._holes));
}
Face::~Face()
{
    Component::SQLDelete();
}

std::vector<std::string> GetVector(QByteArray qba)
{
	QDataStream stream(&qba, QIODevice::ReadOnly);
	QString str;
	std::vector<std::string> result;

	unsigned int len=0;
	stream >> len;
	for(unsigned int i=0;i<len;i++)
	{
		stream>>str;
		result.push_back(str.toStdString());
	}
	return result;
}	

std::vector<std::string> Face::getNodes() const
{
    std::vector<std::string> nodes;
    foreach(Node* n, _nodes)
    {
        nodes.push_back(n->getUUID());
    }
    /*QVariant value;
    if(DBConnector::getInstance()->Select("faces",  uuid.toRfc4122(),
                                          "nodes", &value))
        nodes = GetVector(value.toByteArray());*/
    return nodes;
}
std::vector<Node*> Face::getNodePointers() const
{
    return _nodes;
}

Component* Face::clone()
{
    return new Face(*this);
}

DM::Components Face::getType()
{
	return DM::FACE;
}
QString Face::getTableName()
{
    return "faces";
}
/*std::vector<std::vector<std::string> > GetVectorVector(QByteArray qba)
{
    QDataStream stream(&qba, QIODevice::ReadWrite);
	std::vector<std::vector<std::string> > result;

	unsigned int len=0;
	stream >> len;
	for(unsigned int i=0;i<len;i++)
	{
		std::vector<std::string> v;
		unsigned int ilen = 0;
		stream >> ilen;
		for(unsigned int i=0;i<ilen;i++)
		{
			QString qstr;
			stream >> qstr;
			v.push_back(qstr.toStdString());
		}
		result.push_back(v);
	}
	return result;
}*/

const std::vector<std::vector<std::string> > Face::getHoles() const
{
    std::vector<std::vector<std::string> > holes;
    foreach(Face* f, _holes)
    {
        std::vector<std::string> hole;
        foreach(Node* n, f->getNodePointers())
        {
            hole.push_back(n->getUUID());
        }
        holes.push_back(hole);
    }

    /*QVariant value;
    if(DBConnector::getInstance()->Select("faces",  uuid.toRfc4122(),
                                          "holes", &value))
        holes = GetVectorVector(value.toByteArray());*/
    return holes;
}

const std::vector<Face*> Face::getHolePointers() const
{
    return _holes;
}

void Face::addHole(std::vector<std::string> hole)
{
    System *curSys = this->getCurrentSystem();
    std::vector<Node*> holeNodes;
    foreach(std::string uuidNodes, hole)
        holeNodes.push_back(curSys->getNode(uuidNodes));

    addHole(holeNodes);
}

void Face::addHole(std::vector<Node*> hole)
{
    _holes.push_back(getCurrentSystem()->addFace(hole));
}

void Face::addHole(Face* hole)
{
    if(hole==this)
    {
        Logger(Error) << "addHole: self reference not possible";
        return;
    }
    _holes.push_back(hole);
    SQLUpdateValues();
}

void Face::SQLUpdateValues()
{
    DBConnector::getInstance()->Update("faces", uuid.toRfc4122(),
                                       "nodes", GetBytes(_nodes),
                                       "holes", GetBytes(_holes));
}
