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
#include "dmedge.h"


using namespace DM;

Face::Face(std::vector<std::string> nodes) : Component()
{
    this->nodes = nodes;
}
Face::Face(const Face& e) : Component(e)
{
    this->nodes=e.nodes;
}
std::vector<std::string> Face::getNodes() {
    return this->nodes;
}

Component* Face::clone()
{
    return new Face(*this);
}
DM::Components Face::getType()
{
	return DM::FACE;
}

void Face::getRawData(QBuffer* buf)
{
	DM::GetRawVectorData(nodes, buf);
}