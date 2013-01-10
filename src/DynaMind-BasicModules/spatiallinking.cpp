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

#include "spatiallinking.h"
#include <tbvectordata.h>
#include <QHash>
#include <QPolygonF>

#ifdef _OPENMP
#include <omp.h>
#endif


DM_DECLARE_NODE_NAME(SpatialLinking, Modules)

SpatialLinking::SpatialLinking()
{
    this->city = 0;
    this->base = "";
    this->linkto = "";
    spatialL = 100;

    this->addParameter("Base", DM::STRING, & this->base);
    this->addParameter("Link", DM::STRING, & this->linkto);

    std::vector<DM::View> data;
    data.push_back(  DM::View ("dummy", DM::SUBSYSTEM, DM::MODIFY) );
    this->addData("Data", data);

}

void SpatialLinking::init() {
    city = this->getData("Data");
    if (city == 0)
        return;
    if (base.empty())
        return;
    if (linkto.empty())
        return;

    DM::View * v1 = this->city->getViewDefinition(base);
    DM::View * v2 = this->city->getViewDefinition(linkto);

    if (!v1) {
        Logger(Error) << "View " << base << "doesn't exist";
        return;
    }

    if (!v2) {
        Logger(Error) << "View " << linkto << "doesn't exist";
        return;
    }
    this->vbase = DM::View(v1->getName(), v1->getType(), READ);
    this->vlinkto = DM::View(v2->getName(), v2->getType(), READ);

    this->vbase.addLinks(linkto, vlinkto);
    this->vlinkto.addLinks(base, vbase);

    std::vector<DM::View> data;
    data.push_back(vbase);
    data.push_back(vlinkto);
    this->addData("Data", data);
}

string SpatialLinking::getHelpUrl()
{
    return "https://docs.google.com/document/pub?id=1Vxx4vlj5lokvCAHEATsTIm0Q4n8kG92Krp8e2bLf9Zw";
}

void SpatialLinking::run() {
    city = this->getData("Data");
    std::vector<std::string> baseUUIDs = city->getUUIDsOfComponentsInView(vbase);
    std::vector<QPointF> centerPoints;

    //Node id that point to elements in the vector are stored in Hashmap for faster lookup
    QHash<QString, std::vector<int> * > nodesMap;

    //Init Point List
    int counterID = -1;
    foreach (std::string baseUUID, baseUUIDs) {
        counterID++;
        Node c(0,0,0);
        if (vbase.getType() == DM::FACE) {
            Face * f = city->getFace(baseUUID);
            DM::Node d (TBVectorData::CaclulateCentroid(city, f));
            c.setX(d.getX());
            c.setY(d.getY());
            c.setZ(d.getZ());
        }
        if (vbase.getType() == DM::NODE) {
            DM::Node * n1 = city->getNode(baseUUID);
            c.setX(n1->getX());
            c.setY(n1->getY());
            c.setZ(n1->getZ());
        }
        centerPoints.push_back(QPointF(c.getX(), c.getY()));
        //CreateKey
        int x = c.getX() / spatialL;
        int y = c.getY() / spatialL;

        QString key = QString::number(x) + "|" +  QString::number(y);
        std::vector<int> * vn;
        if (!nodesMap.contains(key)) {
            vn = new std::vector<int>();
            nodesMap[key] = vn;
        }
        vn->push_back(counterID);
    }
    //Check if Center point is within toLink

    std::vector<std::string> linkUUIDs = city->getUUIDsOfComponentsInView(vlinkto);

    int CounterElementLinked = 0;
    int NumberOfLinks = linkUUIDs.size();
#pragma omp parallel for
    for (int i = 0; i < NumberOfLinks; i++) {
    //foreach (std::string linkUUID, linkUUIDs) {
        std::string linkUUID = linkUUIDs[i];
        QPolygonF qf = TBVectorData::FaceAsQPolgonF(city, city->getFace(linkUUID));

        //Search Space
        int xmin = (int) qf.boundingRect().x() / spatialL;
        int ymin = (int) (qf.boundingRect().y() )/spatialL ;
        int xmax = (int) (qf.boundingRect().x() + qf.boundingRect().width())/spatialL;
        int ymax = (int) (qf.boundingRect().y() + qf.boundingRect().height())/spatialL;
        std::vector<LinkAttribute> links;
        int elementInSearchSpace = 0;
        for (int x = xmin; x <= xmax; x++) {
            for (int y = ymin; y <= ymax; y++) {
                QString key = QString::number(x) + "|" +  QString::number(y);
                //Test Each Key
                std::vector<int> * centers = nodesMap[key];
                if (!centers) {

                    continue;
                }
                elementInSearchSpace=elementInSearchSpace+centers->size();
                foreach (int id, (*centers)) {
                    if (qf.containsPoint(centerPoints[id], Qt::OddEvenFill)) {
                        LinkAttribute lto;
                        lto.viewname = linkto;
                        lto.uuid = linkUUID;
                        Component * cmp = city->getComponent(baseUUIDs[id]);
                        Attribute * attr = cmp->getAttribute(linkto);
                        std::vector<LinkAttribute>  ls = attr->getLinks();
                        ls.push_back(lto);
                        attr->setLinks(ls);

                        LinkAttribute lbase;
                        lbase.viewname = base;
                        lbase.uuid = cmp->getUUID();

                        links.push_back(lbase);
                        CounterElementLinked++;
                    }

                }

            }
        }
        //Logger(Debug) << "Element in search space " << elementInSearchSpace;
        //Logger(Debug) << "Linked to " << links.size();
        Component * cmp = city->getComponent(linkUUID);
        Attribute * attr = cmp->getAttribute(base);
        attr->setLinks(links);
    }


    foreach(QString key, nodesMap.keys()) {
        delete nodesMap[key];
    }
    nodesMap.clear();

    Logger(DM::Debug) << "Elements Linked " << CounterElementLinked;



}
