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


#include "calculatecentroid.h"
#include <cgalgeometry.h>
#include "tbvectordata.h"
#include "guicalculatecentroid.h"
#include <sstream>
#include <math.h>


DM_DECLARE_NODE_NAME(CalculateCentroid, Modules)

CalculateCentroid::CalculateCentroid()
{
    this->city = 0;
    this->NameOfExistingView = "";
    std::vector<DM::View> data;
    data.push_back(  DM::View ("dummy", DM::SUBSYSTEM, DM::MODIFY) );
    this->addParameter("NameOfExistingView", DM::STRING, & this->NameOfExistingView);
    this->addData("Data", data);
    changed = true;
}


void CalculateCentroid::init()
{
	std::map<std::string, DM::View> views = getViewsInStream()["Data"];
	if(views.size() == 0)
	{
		DM::Logger(DM::Warning) << "empty stream in module '" << getClassName() << "'";
		return;
	}

    /*city = this->getData("Data");
    if (city == 0)
        return;
    std::vector<std::string> views = city->getNamesOfViews();

    foreach (std::string s, views)
        DM::Logger(DM::Debug) << s;
		*/
    if (this->NameOfExistingView.empty())
        return;
    /*DM::View * v = city->getViewDefinition(NameOfExistingView);
    if (!v)
        return;*/
	DM::View v;
	if(!map_contains(&views, NameOfExistingView, v))
	{
		DM::Logger(DM::Warning) << "view '" << NameOfExistingView 
			<< "' does not exist in stream 'Data' in module '" << getClassName() << "'";
		return;
	}

    DM::View writeView = DM::View(v.getName(), v.getType(), DM::READ);
    writeView.addAttribute("centroid_x");
    writeView.addAttribute("centroid_y");
    writeView.addAttribute("area");

    std::stringstream ss;
    ss << v.getName() << "_CENTROIDS";

    newPoints = DM::View(ss.str(), DM::NODE, DM::WRITE);

    std::stringstream link;
    link << v.getName()<< "_ID";
    newPoints.addLinks(link.str(), writeView.getName());


    std::vector<DM::View> data;
    data.push_back(writeView);
    data.push_back(newPoints);
    this->addData("Data", data);
    vData = writeView;

    //this->updateParameter();
}


void CalculateCentroid::run() {
    city = this->getData("Data");
    std::vector<std::string> names =city->getUUIDsOfComponentsInView(vData);
    std::stringstream link;
    link << vData.getName();
    int elements = names.size();
    for (int i = 0; i < elements; i++){
        Face * f = city->getFace(names[i]);

        if(!f)
        {
            DM::Logger(DM::Error) << "Face does not exist";
            return;
        }

        Node p = DM::CGALGeometry::CalculateCentroid(this->city, f);
        double area = TBVectorData::CalculateArea(this->city, f);
        f->addAttribute("centroid_x", p.getX());
        f->addAttribute("centroid_y", p.getY());
        f->addAttribute("area", area);

        Node * cn = city->addNode(p, newPoints);
        Attribute attr(link.str());
        attr.setLink(vData.getName(), f->getUUID());
        cn->addAttribute(attr);
    }
}
bool CalculateCentroid::createInputDialog() {
    QWidget * w = new GUICalculateCentroid(this);
    w->show();
    return true;
}

void CalculateCentroid::setNameOfView(string name) {
    changed = true;
    this->NameOfExistingView = name;

}
DM::System * CalculateCentroid::getSystemIn() {
    return this->city;
}

string CalculateCentroid::getHelpUrl()
{
    return "https://github.com/iut-ibk/DynaMind-ToolBox/wiki/CalculateCentroid";
}

