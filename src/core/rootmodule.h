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
#ifndef ROOTMODULE_H
#define ROOTMODULE_H

#include "compilersettings.h"
#include "module.h"

class VIBE_HELPER_DLL_EXPORT RootModule : public vibens::Module
{
public:
    RootModule();

    boost::shared_ptr<vibens::Module> clone() const;
    const RasterData &getRasterData(const std::string &name, int T) const;
    const VectorData &getVectorData(const std::string &name) const;
    void run();
};

#endif // ROOTMODULE_H
