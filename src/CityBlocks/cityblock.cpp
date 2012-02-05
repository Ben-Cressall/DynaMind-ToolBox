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

#include "cityblock.h"
#include <DM.h>
#include <math.h>
#include <tbvectordata.h>

DM_DECLARE_NODE_NAME(CityBlock,BlockCity)

CityBlock::CityBlock()
{
    std::vector<DM::View> views;
    superblock = DM::View("SUPERBLOCK", DM::FACE, DM::READ);
    superblock.addAttribute("CityBlock_Width");
    superblock.addAttribute("CityBlock_Height");
    cityblock = DM::View("CITYBLOCK", DM::FACE, DM::WRITE);

    streets = DM::View("STREET", DM::EDGE, DM::WRITE);
    intersections = DM::View("INTERSECTION", DM::NODE, DM::WRITE);

    views.push_back(superblock);
    views.push_back(cityblock);
    views.push_back(streets);
    views.push_back(intersections);


    this->width = 100;
    this->height = 100;

    this->addParameter("Width", DM::DOUBLE, &this->width);
    this->addParameter("Height", DM::DOUBLE, &this->height);


    this->addData("City", views);
}

void CityBlock::run() {

    DM::System * city = this->getData("City");



    std::vector<std::string> blockids = city->getNamesOfComponentsInView(superblock);

    foreach (std::string blockid, blockids) {
        //calulculate height;

        DM::Face * fblock = city->getFace(blockid);
        double minX = 0;
        double maxX = 0;
        double minY = 0;
        double maxY = 0;
        for (int i = 0; i < fblock->getEdges().size(); i++){
            DM::Edge * e = city->getEdge(fblock->getEdges()[i]);



            DM::Node * n1 = city->getNode(e->getStartpointName());
            DM::Node * n2 = city->getNode(e->getEndpointName());

            if (i == 0) {
                minX = n1->getX();
                maxX = n1->getX();
                minY = n1->getY();
                maxY = n1->getY();
            }

            if(minX > n1->getX())
                minX = n1->getX();
            if(minX > n2->getX())
                minX = n2->getX();
            if(maxX < n1->getX())
                maxX = n1->getX();
            if(maxX < n2->getX())
                maxX = n2->getX();

            if(minY > n1->getY())
                minY = n1->getY();
            if(minY > n2->getY())
                minY = n2->getY();
            if(maxY < n1->getY())
                maxY = n1->getY();
            if(maxY < n2->getY())
                maxY = n2->getY();

        }
        double blockHeight = maxX - minX;
        double blockWidth = maxY - minY;


        DM::Logger(DM::Debug) << blockHeight;
        DM::Logger(DM::Debug) << blockWidth;


        //Create Parcels

        int elements_x = blockWidth/this->height;
        int elements_y = blockHeight/this->width;
        double realwidth = blockWidth / elements_x;
        double realheight = blockHeight / elements_y;
        fblock->addAttribute("CityBlock_Width",realwidth);
        fblock->addAttribute("CityBlock_Height",realheight);


        for (int x = 0; x < elements_x; x++) {
            for (int y = 0; y < elements_y; y++) {

                DM::Node * n1 = TBVectorData::addNodeToSystem2D(city,
                                                                intersections,
                                                                DM::Node(minX + realwidth*x,minY + realheight*y,0),
                                                                true,
                                                                .001);
                DM::Node * n2 = TBVectorData::addNodeToSystem2D(city,
                                                                intersections,
                                                                DM::Node(minX + realwidth*(x+1),minY + realheight*y,0),
                                                                true,
                                                                .001);
                DM::Node * n3 = TBVectorData::addNodeToSystem2D(city,
                                                                intersections,
                                                                DM::Node(minX + realwidth*(x+1),minY + realheight*(y+1),0),
                                                                true,
                                                                .001);
                DM::Node * n4 = TBVectorData::addNodeToSystem2D(city,
                                                                intersections,
                                                                DM::Node (minX + realwidth*x,minY + realheight*(y+1),0),
                                                                true,
                                                                .001);

                DM::Edge * e1 = city->addEdge(n1, n2);
                DM::Edge * e2 = city->addEdge(n2, n3);
                DM::Edge * e3 = city->addEdge(n3, n4);
                DM::Edge * e4 = city->addEdge(n4, n1);

                //Every Edge is also a Street
                if (TBVectorData::getEdge(city, streets, e1, false) == 0) {
                    city->addComponentToView(e1, streets);


                }
                if (TBVectorData::getEdge(city, streets, e2, false) == 0) {
                    city->addComponentToView(e2, streets);

                }
                if (TBVectorData::getEdge(city, streets, e3, false) == 0) {
                    city->addComponentToView(e3, streets);

                }
                if (TBVectorData::getEdge(city, streets, e4, false) == 0) {
                    city->addComponentToView(e4, streets);

                }

                std::vector<DM::Edge*> ve;
                ve.push_back(e1);
                ve.push_back(e2);
                ve.push_back(e3);
                ve.push_back(e4);


                DM::Face * f = city->addFace(ve, cityblock);
            }
        }
    }
    DM::Logger(DM::Debug) << "Number of CityBlocks " << city->getAllComponentsInView(cityblock).size();
    DM::Logger(DM::Debug) << "Number of Streets " << city->getAllComponentsInView(streets).size();

}
