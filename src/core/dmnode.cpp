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

using namespace DM;


Node::Node( double x, double y, double z) : Component()
{
    this->x=x;
    this->y=y;
    this->z=z;
}

Node::Node(const Node& n) : Component(n)
{
    x=n.x;
    y=n.y;
    z=n.z;
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


bool Node::compare2d(const Node &other, double round ) const {

    return abs( this->x - other.getX() ) <= round   &&  abs( this->y - other.getY() ) <= round;
}

bool Node::compare2d(const Node * other , double round ) const {

    return abs( this->x - other->getX() ) <= round   &&  abs( this->y - other->getY() ) <= round;
}
