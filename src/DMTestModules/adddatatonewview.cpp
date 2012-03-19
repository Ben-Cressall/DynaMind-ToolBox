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
#include "adddatatonewview.h"
#include "guiadddatatonewview.h"


DM_DECLARE_NODE_NAME(AddDataToNewView, Modules)
AddDataToNewView::AddDataToNewView()
{
    sys_in = 0;
    this->NameOfNewView = "";

    data.push_back(  DM::View ("dummy", DM::SUBSYSTEM, DM::MODIFY) );

    this->addParameter("NameOfNewView", DM::STRING, &this->NameOfNewView);
    this->addParameter("NameOfExistingView", DM::STRING, &this->NameOfExistingView);
    this->addParameter("newAttributes", DM::STRING_LIST, &this->newAttributes);
    this->addData("Data", data);
}

void AddDataToNewView::run()
{
    DM::System * sys = this->getData("Data");
    DM::View v_existing= sys->getViewDefinition(NameOfExistingView);
    DM::ComponentMap cmp = sys->getAllComponentsInView(v_existing);

    DM::View v_new= sys->getViewDefinition(NameOfNewView);

    for (DM::ComponentMap::const_iterator it = cmp.begin();
         it != cmp.end();
         ++it) {
        DM::Component * c = it->second;
        foreach (std::string attr, newAttributes) {
            c->addAttribute(attr, 0);
        }
        sys->addComponentToView(c, v_new);
    }


}

void AddDataToNewView::init()
{

    sys_in = this->getData("Data");
    if (sys_in == 0)
        return;
    std::vector<std::string> views = sys_in->getViews();

    foreach (std::string s, views)
        DM::Logger(DM::Debug) << s;


    if (this->NameOfExistingView.empty())
        return;

    if (this->NameOfNewView.empty())
        return;




    DM::View view = sys_in->getViewDefinition(NameOfExistingView);

    DM::View newView(getParameterAsString("NameOfNewView"), view.getType(), DM::WRITE);

    //Try to get Existing View
    foreach (DM::View v, data) {
        if (v.getName().compare(newView.getName()) == 0) {
            newView = v;
        }
    }

    bool changed = false;
    foreach (std::string s, getParameter<std::vector<std::string> >("newAttributes")) {
        std::vector<std::string>  writes_already = newView.getWriteAttributes();
        if (find(writes_already.begin(), writes_already.end(), s) != writes_already.end())
            continue;
        newView.addAttribute(s);
        changed = true;
    }

    if (NameOfNewView.compare(NameOfNewView_old) != 0) {
        changed = true;
        NameOfNewView_old = NameOfNewView;
    }

    if (changed == true) {
        std::vector<DM::View> new_data;
        foreach (DM::View v, this->data) {
            if (v.getName().compare(newView.getName()) != 0)
                new_data.push_back(v);
            new_data.push_back(newView);
        }
        data = new_data;
        this->addData("Data", data);

    }


}
bool AddDataToNewView::createInputDialog() {
    QWidget * w = new GUIAddDatatoNewView(this);
    w->show();
    return true;
}

DM::System * AddDataToNewView::getSystemIn() {
    return this->sys_in;
}

void AddDataToNewView::addView()
{


}

void AddDataToNewView::addAttribute(string s) {
    this->newAttributes.push_back(s);
}
