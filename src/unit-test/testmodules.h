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

#ifndef TESTMODULES_H
#define TESTMODULES_H

#include "gtest/gtest.h"

namespace {
    class TestModules : public ::testing::Test
    {
    public:
        DM::Simulation *sim;

    protected:
            TestModules() {}
            virtual ~TestModules(){}
            virtual void SetUp();
            virtual void TearDown();
    };

    TEST_F(TestModules,setParameterInModule)
    {
        DM::Module * m = sim->addModule("TestModule");
        EXPECT_TRUE(m!=0);
        m->setParameterValue("DoubleValue", "0.1");
        double val = m->getParameter<double>("DoubleValue");
        EXPECT_EQ(0.1,val);
    }
}
#endif // TESTMODULES_H
