"""
@file
@author  Chrisitan Urich <christian.urich@gmail.com>
@version 1.0
@section LICENSE

This file is part of VIBe2
Copyright (C) 2011  Christian Urich

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
"""

from pydynamind import *
from numpy import *
from os import  *

class WhiteNoise(Module):
        def __init__(self):
            Module.__init__(self)
            self.createParameter("Height",LONG,"Sample Description")
            self.createParameter("Width",LONG,"Sample Description")
            self.createParameter("CellSize",DOUBLE,"Sample Description")
            self.Height = 200
            self.Width = 200
            self.CellSize = 20

            views = []
            self.r = View("OUT", RASTERDATA, WRITE)
            views.append(self.r)
            self.addData("OUT", views)

    
        def run(self):
            r = self.getRasterData("OUT", self.r)
            a = random.random((self.Width,self.Height))
            r.setSize(200,200, self.CellSize)
            for i in range(self.Width):
                for j in range(self.Height):
                    r.setValue(i,j,a[i,j])
            


