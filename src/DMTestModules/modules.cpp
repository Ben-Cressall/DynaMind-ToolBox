/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of VIBe2
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
#include "nodefactory.h"
#include "moduleregistry.h"
#include "testmodule.h"
#include "inoutmodule.h"
#include "inout2.h"
<<<<<<< HEAD
#include "userdefineddata.h"
=======
#include "checkshape.h"

>>>>>>> 41798387892b46e8f71d22788b107472712d963d
using namespace std;



extern "C" void VIBE_HELPER_DLL_EXPORT  registerModules(ModuleRegistry *registry) {
    registry->addNodeFactory(new NodeFactory<TestModule>());
    registry->addNodeFactory(new NodeFactory<InOut>());
    registry->addNodeFactory(new NodeFactory<InOut2>());
<<<<<<< HEAD
    registry->addNodeFactory(new NodeFactory<UserdefinedData>());
=======
    registry->addNodeFactory(new NodeFactory<CheckShape>());
>>>>>>> 41798387892b46e8f71d22788b107472712d963d
}

