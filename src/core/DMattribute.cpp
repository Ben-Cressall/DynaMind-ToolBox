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

#include <DMcomponent.h>
#include <DMattribute.h>
#include <QVariant>

using namespace DM;

Attribute::Attribute(std::string name, std::string id)
{
    this->name=name;
    this->id=id;
}

Attribute::~Attribute()
{
}

void Attribute::setName(std::string name)
{
    this->name=name;
}

void Attribute::setID(std::string ID)
{
    this->id=ID;
}
std::string Attribute::getName()
{
    return name;
}
std::string Attribute::getID()
{
    return id;
}

void Attribute::setValue(double v)
{

}
double Attribute::getDouble(){
    return 0;
}
void Attribute::setValue(std::string s)
{

}
std::string Attribute::getString(){
    return "";
}
void Attribute::setValue(std::vector<double> v)
{
}
std::vector<double> Attribute::getDoubleVector()
{
    return vector<double>(2,2);
}
void Attribute::setValue(std::vector<std::string> s)
{
}
std::vector<std::string> Attribute::getStringVector()
{
    return vector<string>(2,"");
}
