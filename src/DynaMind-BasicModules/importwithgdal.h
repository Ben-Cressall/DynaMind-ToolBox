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

#ifndef IMPORTWITHGDAL_H
#define IMPORTWITHGDAL_H
#include <dmmodule.h>
#include <dm.h>
#include "ogrsf_frmts.h"
#include "gdal_priv.h"
#include <QHash>
#include <QString>


using namespace DM;

class DM_HELPER_DLL_EXPORT ImportwithGDAL : public Module
{
    DM_DECLARE_NODE(ImportwithGDAL)

    private:
        bool fileok;
        std::string FileName;
        std::string ViewName;
        bool isvectordata;
        int epsgcode;
        bool transformok;
        double tol;
        bool append;
        DM::View view;
        double devider;
        std::map<std::string, std::string> attributesToImport;
        bool ImportAll;
        OGRCoordinateTransformation *poCT;

        QHash<QString, std::vector<DM::Node* > *> nodeList;
        DM::Node * addNode(DM::System * sys, double x, double y, double z);
        void appendAttributes(DM::Component * cmp, OGRFeatureDefn *poFDefn, OGRFeature *poFeature);
        DM::Component * loadNode(DM::System * sys,  OGRFeature *poFeature);
        DM::Component * loadEdge(DM::System * sys,  OGRFeature *poFeature);
        DM::Component * loadFace(DM::System * sys,  OGRFeature *poFeature);
        void initPointList(DM::System * sys);
        QString createHash(double x, double y);
        void vectorDataInit(OGRDataSource       *poDS);
        void rasterDataInit(GDALDataset  *poDataset);
        bool importVectorData();
        bool importRasterData();
        bool transform(double *x, double *y);

    public:
        void run();
        void init();
        ImportwithGDAL();
        ~ImportwithGDAL();
};

#endif // IMPORTWITHGDAL_H
