/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012  Christian Urich

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

#include "testsystem.h"
#include <dm.h>
namespace {
    TEST_F(TestSystem,memoryTest){
        DM::System * sys = new DM::System();
        DM::Node * n = sys->addNode(new DM::Node(0,0,0));
        std::string uuid = n->getUUID();
        n->addAttribute("Attribute1", 0);
        DM::System * sys1 = sys->createSuccessor();
        n = sys1->addNode(new DM::Node(0,0,0));
        n->addAttribute("Attribute2", 0);
        DM::Component * cmp = sys1->getComponent(uuid);
        cmp->addAttribute("Attribute1", 0);
        cmp->addAttribute("Attribute2", 0);
        cmp->changeAttribute("Attribute1", 0);
        sys1->createSuccessor();
        delete sys;
        ASSERT_TRUE(true);
    }
}
