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
	DM::View  v_existing = this->getViewInStream("Data", NameOfExistingView);
	if (v_existing.getType() ==  -1)
	{
		DM::Logger(DM::Error) << "input view '" << NameOfExistingView << "' does not exist";
		return;
	}
	DM::View  v_new = DM::View( this->NameOfNewView, v_existing.getType(), DM::WRITE );

	foreach(DM::Component* c, sys->getAllComponentsInView(v_existing))
	{
		if(this->onlySelected)
			if(DM::Attribute* a = c->getAttribute("selected"))
				if(a->getDouble() < 0.0001)
					continue;

		sys->addComponentToView(c, v_new);
	}
}

void AddDataToNewView::init()
{
	// TODO: Works fine until someone is changing something upstream -> no update downstream!
	// NOTE: redone, upper comment may not be valid anymore

	std::vector<DM::View> views = getViewsInStream("Data");
	if(views.size() == 0)
	{
		DM::Logger(DM::Warning) << "empty stream in module '" << getClassName() << "'";
		return;
	}

	//sys_in = this->getData("Data");
	if (this->NameOfExistingView.empty() || this->NameOfNewView.empty())
		return;

	/*DM::View* inViewDef = sys_in->getViewDefinition(NameOfExistingView);
	if (!inViewDef)
	{
		DM::Logger(DM::Warning) << "view '" << NameOfExistingView << "' does not exist ";
		return;
	}*/

	

	DM::View inViewDef = getViewInStream("Data", NameOfExistingView).clone(DM::READ);
	if(inViewDef.getName().empty())
	{
		DM::Logger(DM::Warning) << "view '" << NameOfExistingView
			<< "' does not exist in stream 'Data' in module '" << getClassName() << "'";
		return;
	}

	if(NameOfExistingView == NameOfNewView)
	{
		inViewDef = inViewDef.clone(DM::READ);
		// modify only
		//inViewDef.setAccessType(DM::MODIFY);
		// add new attributes
		foreach(std::string s, this->newAttributes)
			inViewDef.addAttribute(s, DM::Attribute::NOTYPE, DM::WRITE);

		std::vector<DM::View> data;
		data.push_back(inViewDef);
		this->addData("Data", data);
	}
	else
	{
		// create new views
		DM::View inView = inViewDef.clone(DM::READ);
		DM::View outView = inViewDef.clone(DM::WRITE, NameOfNewView);
		foreach(std::string s, this->newAttributes)
			outView.addAttribute(s, DM::Attribute::NOTYPE, DM::WRITE);

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
