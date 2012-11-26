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

#include <dmcomponent.h>
#include <dmnode.h>
#include <cstdlib>
#include <math.h>
using namespace DM;

Node::Node( double x, double y, double z) : Component()
{
    this->x=x;
    this->y=y;
    this->z=z;
}

Node::Node() : Component()
{
    this->x = 0;
    this->y = 0;
    this->z = 0;
}


Node::Node(const Node& n) : Component(n)
{
    x=n.x;
    y=n.y;
    z=n.z;
}

void Node::operator =(const Node &n)
{

    x=n.x;
    y=n.y;
    z=n.z;

    this->mutex = new QMutex(QMutex::Recursive);
}



double Node::getX() const
{
    return x;
}

double Node::getY() const
{
    return y;
}

double Node::getZ() const
{
    return z;
}

const QVector<double> Node::get() const
{
    QVector<double> v;
    v.push_back(v_[0]);
    v.push_back(v_[1]);
    v.push_back(v_[2]);
    return v;
}

void Node::setX(double x)
{
    this->x=x;
}

void Node::setY(double y)
{
    this->y=y;
}

void Node::setZ(double z)
{
    this->z=z;
}

Component* Node::clone()
{
    return new Node(*this);
}


bool Node::operator ==(const Node & other) const {

    return this->x == other.getX() && this->y == other.getY() && this->z == other.getZ();
}
Node Node::operator -(const Node & other) const {
    return(Node(this->x - other.getX(), this->y - other.getY(), this->z - other.getZ()));
}

Node Node::operator +(const Node & other) const {
    return(Node(this->x + other.getX(), this->y + other.getY(), this->z + other.getZ()));
}

Node Node::operator *(const double &val) const
{
    return(Node(this->x * val, this->y  * val, this->z * val));
}

Node Node::operator /(const double &val) const
{
    return(Node(this->x / val, this->y  / val, this->z / val));
}


bool Node::compare2d(const Node &other, double round ) const {

    return fabs( this->x - other.getX() ) <= round   &&  fabs( this->y - other.getY() ) <= round;
}

bool Node::compare2d(const Node * other , double round ) const {

    return fabs( this->x - other->getX() ) <= round   &&  fabs( this->y - other->getY() ) <= round;
}
