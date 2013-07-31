/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @author Michael Mair <michael.mair@gmail.com>
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
#include <algorithm>


DM_DECLARE_NODE_NAME(AddDataToNewView, Modules)
AddDataToNewView::AddDataToNewView()
{
    sys_in = NULL;
    this->NameOfNewView = "";
    this->onlySelected = false;

    data.push_back(  DM::View ("dummy", DM::SUBSYSTEM, DM::MODIFY) );
    this->addData("Data", data);

    this->addParameter("NameOfNewView", DM::STRING, &this->NameOfNewView);
    this->addParameter("NameOfExistingView", DM::STRING, &this->NameOfExistingView);
    this->addParameter("newAttributes", DM::STRING_LIST, &this->newAttributes);
    this->addParameter("onlySelected", DM::BOOL, &this->onlySelected);
}

void AddDataToNewView::run()
{
	DM::System * sys = this->getData("Data");
	DM::View * v_existing= sys->getViewDefinition(NameOfExistingView);
	if (!v_existing) 
	{
		DM::Logger(DM::Error) << "input view '" << NameOfExistingView << "' does not exist";
		return;
	}
	DM::View * v_new= sys->getViewDefinition(NameOfNewView);
	if(!v_new)
	{
		DM::Logger(DM::Error) << "output view '" << NameOfNewView << "' does not exist (AddDataToNewView::init went wrong)";
		return;
	}

	DM::ComponentMap cmp = sys->getAllComponentsInView(*v_existing);
	for (DM::ComponentMap::const_iterator it = cmp.begin(); it != cmp.end(); ++it) 
	{
		DM::Component * c = sys->getComponent(it->first);
		if(this->onlySelected)
			if(DM::Attribute* a = c->getAttribute("selected"))
				if(a->getDouble() < 0.0001)
					continue;

		sys->addComponentToView(c, *v_new);
	}
}

void AddDataToNewView::init()
{
	// TODO: Works fine until someone is changing something upstream -> no update downstream!
	// NOTE: redone, upper comment may not be valid anymore

	sys_in = this->getData("Data");
	if (!sys_in || this->NameOfExistingView.empty() || this->NameOfNewView.empty())
		return;
	
	DM::View* inViewDef = sys_in->getViewDefinition(NameOfExistingView);
	if (!inViewDef) 
	{
		DM::Logger(DM::Warning) << "view '" << NameOfExistingView << "' does not exist ";
		return;
	}

	if(NameOfExistingView == NameOfNewView)
	{
		// modify only
		inViewDef->setAccessType(DM::MODIFY);
		// add new attributes
		foreach(std::string s, getParameter<std::vector<std::string> >("newAttributes"))
			inViewDef->addAttribute(s);
	}
	else
	{
		// create new views
		DM::View inView(NameOfExistingView, inViewDef->getType(), DM::READ);
		DM::View outView(NameOfNewView, inViewDef->getType(), DM::WRITE);
		// get existent attributes
		foreach(std::string s, inViewDef->getReadAttributes())
			outView.addAttribute(s);
		foreach(std::string s, inViewDef->getWriteAttributes())
			outView.addAttribute(s);
		// add new attributes
		foreach(std::string s, getParameter<std::vector<std::string> >("newAttributes"))
			outView.addAttribute(s);

		std::vector<DM::View> data;
		data.push_back(inView);
		data.push_back(outView);
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

string AddDataToNewView::getHelpUrl()
{
    return "https://github.com/iut-ibk/DynaMind-BasicModules/blob/master/doc/AddDataToNewView.md";
}
