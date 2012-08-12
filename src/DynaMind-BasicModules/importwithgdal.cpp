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

#include "importwithgdal.h"
#include "tbvectordata.h"

#include <QHash>

DM_DECLARE_NODE_NAME(ImportwithGDAL, Modules)

ImportwithGDAL::ImportwithGDAL()
{
    this->FileName = "";
    this->addParameter("Filename", DM::FILENAME, &this->FileName);
    this->ViewName = "";
    this->addParameter("ViewName", DM::STRING, &this->ViewName);
    this->tol = 0.01;
    this->addParameter("Tolerance", DM::DOUBLE, &this->tol);
    devider = 100;
}

DM::Node * ImportwithGDAL::addNode(DM::System * sys, double x, double y, double z) {
    //CreateKey
    DM::Node n_tmp(x,y,z);

    int ix = (int) x / devider;
    int iy = (int) y / devider;
    QString key = QString::number(ix) + "|" +  QString::number(iy);
    std::vector<DM::Node* > * nodes = nodeList[key];
    if (!nodes) {
        nodes = new std::vector<DM::Node* >;
        nodeList[key] = nodes;
    }

    foreach (DM::Node * n, *nodes) {
        if (n->compare2d(&n_tmp, tol))
            return n;
    }

    DM::Node * res_n = sys->addNode(n_tmp);
    nodes->push_back(res_n);
    return res_n;
}

void ImportwithGDAL::appendAttributes(Component *cmp, OGRFeatureDefn *poFDefn, OGRFeature *poFeature)
{

    int iField;

    for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
    {
        OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
        Attribute attr(poFieldDefn->GetNameRef());
        if( poFieldDefn->GetType() == OFTInteger )
            attr.setDouble(poFeature->GetFieldAsInteger( iField ));
        else if( poFieldDefn->GetType() == OFTReal )
            attr.setDouble(poFeature->GetFieldAsDouble( iField ));
        else if( poFieldDefn->GetType() == OFTString )
            attr.setString(poFeature->GetFieldAsString( iField ));
        else
            attr.setString(poFeature->GetFieldAsString( iField ));
        cmp->addAttribute(attr);
    }
}

Component *ImportwithGDAL::loadNode(System *sys, OGRFeature *poFeature)
{
    OGRGeometry *poGeometry;
    poGeometry = poFeature->GetGeometryRef();
    DM::Node * n = 0;
    if( poGeometry != NULL
            && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
    {
        OGRPoint *poPoint = (OGRPoint *) poGeometry;
        n = this->addNode(sys, poPoint->getX(), poPoint->getY(), 0);
        n->setView(this->view);
    }
    return n;

}

Component *ImportwithGDAL::loadFace(System *sys, OGRFeature *poFeature)
{
    OGRGeometry *poGeometry;
    poGeometry = poFeature->GetGeometryRef();
    DM::Node * n = 0;
    if( poGeometry != NULL
            && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon )
    {
        OGRPolygon *poPolygon = (OGRPolygon *) poGeometry;
        OGRLinearRing * ring = poPolygon->getExteriorRing();
        int npoints = ring->getNumPoints();
        if (npoints == 0)
            return 0;
        OGRPoint *poPoint = new OGRPoint();
        std::vector<Node*> nlist;

        for (int i = 0; i < npoints; i++) {
            ring->getPoint(i, poPoint);
            n = this->addNode(sys, poPoint->getX(), poPoint->getY(), 0);
            nlist.push_back(n);

        }
        delete poPoint;
        return sys->addFace(nlist, this->view);
    }
    return 0;

}

void ImportwithGDAL::init() {
    if (FileName.empty())
        return;
    if (ViewName.empty())
        return;

    view = DM::View();
    view.setName(ViewName);

    OGRRegisterAll();

    OGRDataSource       *poDS;
    int driverCount = OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
    poDS = OGRSFDriverRegistrar::Open( FileName.c_str(), FALSE );
    if( poDS == NULL )
    {
        printf( "Open failed.\n" );
        return;
    }

    OGRLayer  *poLayer;
    poLayer = poDS->GetLayer(0);
    OGRFeature *poFeature;

    poLayer->ResetReading();
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
        OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
        int iField;

        for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
        {
            OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );
            view.addAttribute(poFieldDefn->GetNameRef());
        }

        OGRGeometry *poGeometry;

        poGeometry = poFeature->GetGeometryRef();
        if( poGeometry != NULL
                && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint )
        {
            view.setType(DM::NODE);
            view.setAccessType(DM::WRITE);
        }
        if( poGeometry != NULL
                && wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon )
        {
            view.setType(DM::FACE);
            view.setAccessType(DM::WRITE);
        }
        else
        {
            printf( "no point geometry\n" );
        }
        OGRFeature::DestroyFeature( poFeature );

        break;
    }
    std::vector<DM::View> data;
    data.push_back(view);

    this->addData("Data", data);
    OGRDataSource::DestroyDataSource( poDS );



}

void ImportwithGDAL::run() {

    DM::System * sys = this->getData("Data");
    OGRRegisterAll();

    OGRDataSource       *poDS;
    int driverCount = OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount();
    poDS = OGRSFDriverRegistrar::Open( FileName.c_str(), FALSE );
    if( poDS == NULL )
    {
        printf( "Open failed.\n" );
        return;
    }

    OGRLayer  *poLayer;

    int layerCount = poDS->GetLayerCount();
    poLayer = poDS->GetLayer(0);
    //poLayer = poDS->GetLayerByName( "point" );

    OGRFeature *poFeature;

    poLayer->ResetReading();
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {

        OGRFeatureDefn *poFDefn = poLayer->GetLayerDefn();
        DM::Component * cmp;
        if (view.getType() == DM::NODE)
            cmp = this->loadNode(sys, poFeature);
        if (view.getType() == DM::FACE)
            cmp = this->loadFace(sys, poFeature);
        if (cmp)
        this->appendAttributes(cmp, poFDefn, poFeature);
        OGRFeature::DestroyFeature( poFeature );
    }

    OGRDataSource::DestroyDataSource( poDS );
}


