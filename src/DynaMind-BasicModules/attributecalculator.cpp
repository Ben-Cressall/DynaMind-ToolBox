/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012-2013  Christian Urich

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

#include "attributecalculator.h"
#include "guiattributecalculator.h"
#include "userdefinedfunctions.h"
#include "loopgroup.h"
#include "parser/mpParser.h"

DM_DECLARE_NODE_NAME(AttributeCalculator, Modules)

//private implemenation to remove muparser dependeny in the header
struct  AttributeCalculator_Impl
{
	DM::System * m_sys;
	void getLinkedAttribute(std::vector< mup::Value> * varaible_container, DM::Component *currentcmp,std::string name);
};

AttributeCalculator::AttributeCalculator()
{
	this->m_p = new AttributeCalculator_Impl();

	this->nameOfBaseView = "";
	this->addParameter("NameOfBaseView", DM::STRING, & this->nameOfBaseView);
	this->variablesMap = std::map<std::string, std::string>();
	this->addParameter("variablesMap", DM::STRING_MAP, & this->variablesMap);
	this->nameOfNewAttribute = "";
	this->addParameter("nameOfNewAttribute", DM::STRING, & this->nameOfNewAttribute);
	this->addParameter("equation", DM::STRING, & this->equation);
	this->asVector = false;
	this->addParameter("asVector", DM::BOOL, & this->asVector);

	sys_in = 0;
	std::vector<DM::View> data;
	data.push_back(  DM::View ("dummy", DM::SUBSYSTEM, DM::MODIFY) );
	this->addData("Data", data);
}

AttributeCalculator::~AttributeCalculator()
{
	delete m_p;
}

void AttributeCalculator::init() {
	if (nameOfBaseView.empty())
		return;
	if (nameOfNewAttribute.empty())
		return;
	if (equation.empty())
		return;

	DM::View baseView = getViewInStream("Data", nameOfBaseView);
	if (baseView.getName().length() == 0)
		return;

	viewsmap.clear();
	variableNames.clear();

	viewsmap[nameOfBaseView] = DM::View(baseView.getName(), baseView.getType(), DM::READ);

	bool modify = false;
	for (std::map<std::string, std::string>::const_iterator it = variablesMap.begin();
		 it != variablesMap.end();
		 ++it) 
	{
		QStringList viewNameList = QString::fromStdString(it->first).split(".");
		std::string viewname = viewNameList.first().toStdString();
		std::string attributename = viewNameList.last().toStdString();
		variableNames.push_back(it->second);

		if (map_contains(&viewsmap, viewname))
		{
			baseView  = getViewInStream("Data", viewname);
			if (baseView.getName().length() == 0)
				return;
			viewsmap[viewname] = DM::View(baseView.getName(), baseView.getType(), DM::READ);
		}

		if (attributename == nameOfNewAttribute)
			modify = true;
		else
			viewsmap[viewname].addAttribute(attributename, DM::Attribute::NOTYPE, DM::READ);
	}

	viewsmap[nameOfBaseView].addAttribute(nameOfNewAttribute, DM::Attribute::NOTYPE, modify ? DM::MODIFY : DM::WRITE);

	std::vector<DM::View> data;
	for (std::map<std::string, DM::View>::const_iterator it = viewsmap.begin(); it != viewsmap.end(); ++it)
		data.push_back(it->second);

	this->addData("Data", data);
	int i = 0;
	i++;
}

void  AttributeCalculator_Impl::getLinkedAttribute(std::vector< mup::Value> * varaible_container, Component *currentcmp, std::string name )
{
	QStringList viewNameList = QString::fromStdString(name).split(".");
	//Remove First Element, is already what comes with currentcmp
	viewNameList.removeFirst();

	Attribute * attr = currentcmp->getAttribute(viewNameList.front().toStdString());

	if (attr->getType() == Attribute::LINK)
	{
		std::string newSearchName = viewNameList.join(".").toStdString();
		foreach(Component* nextcmp, attr->getLinkedComponents())
		{
			if(!nextcmp)
			{
				Logger(Error) << "Linked Element does not exist";
				return;
			}
			this->getLinkedAttribute(varaible_container, nextcmp, newSearchName);
		}
	}

	switch (attr->getType()) {
	case Attribute::DOUBLE:
		varaible_container->push_back( mup::Value(attr->getDouble()));
		break;
	case Attribute::STRING:
		varaible_container->push_back(mup::Value(attr->getString()));
		break;
	default:
		varaible_container->push_back(mup::Value(0));
		break;
	}
}


QString AttributeCalculator::IfElseConverter(QString expression)
{

	if (!expression.contains("if")) return expression;
	int firstif = expression.indexOf("if");
	expression = expression.remove(firstif,2);
	int first_semicolon = expression.indexOf(",");
	int else_semicolon = expression.indexOf(",",first_semicolon+1);
	int second_if = expression.indexOf("if");

	expression = expression.replace(first_semicolon, 1,"?");

	if (second_if < else_semicolon && second_if != -1) {
		expression = IfElseConverter(expression);
	}

	second_if = expression.indexOf(",");
	expression = expression.replace(second_if, 1,":");

	if (expression.contains("if")){
		second_if = expression.indexOf(",");
		expression = expression.replace(second_if, 1,":");
		expression = IfElseConverter(expression);

	}
	return expression;
}

void AttributeCalculator::run() {
	mup::Value mp_c;
	this->sys_in = this->getData("Data");
	m_p->m_sys = sys_in;
	std::map<std::string, mup::Value * > doubleVariables;
	mup::ParserX * p  = new mup::ParserX();
	foreach(const std::string& variable, variableNames)
	{
		mup::Value * d = new mup::Value(0.0);
		doubleVariables[variable] = d;
		p->DefineVar(variable, d);

		std::string nov_variable = "nov_" + variable;

		d = new mup::Value(0.0);
		doubleVariables[nov_variable] = d;
		p->DefineVar(nov_variable, d);

		std::string first_variable = "first_" + variable;

		d = new mup::Value(0.0);
		doubleVariables[first_variable] = d;
		p->DefineVar(first_variable, d);
	}
	p->DefineFun(new dm::Random);
	p->DefineFun(new dm::Round);
	p->DefineVar("counter", &mp_c);

	Logger(Standard) << IfElseConverter(QString::fromStdString(equation)).toStdString();
	p->SetExpr(IfElseConverter(QString::fromStdString(equation)).toStdString());

	foreach(Component* cmp, sys_in->getAllComponentsInView(viewsmap[nameOfBaseView]))
	{
		//mp_counter= (int) this->getInternalCounter()+1;
		Group* lg = dynamic_cast<Group*>(getOwner());
		if(lg) {
			mp_c = lg->getGroupCounter();
			DM::Logger(DM::Debug) << "counter " << lg->getGroupCounter();
		}
		else
		{
			DM::Logger(DM::Debug) << "attribute calc: counter not found";
			mp_c = 0;
		}
		for (std::map<std::string, std::string>::const_iterator it = variablesMap.begin();
			 it != variablesMap.end();
			 ++it)
		{
			std::string varvalue = it->second;
			//All attributes are stored in one container that is evaluated Later.
			std::vector< mup::Value> variable_container;
			//Can be later replaced by a function
			m_p->getLinkedAttribute(&variable_container, cmp, it->first);

			double val = 0;
			QStringList string_vals;
			double nov = 0;
			foreach (mup::Value v, variable_container)
			{
				switch (v.GetType()) {
				case 's':
					string_vals.append(QString::fromStdString(v.GetString()));
					break;
				case 'i':
					val += v.GetInteger();
					break;
				case 'f':
					val += v.GetFloat();
					break;
				default:
					Logger(Warning) << "Unknown type";
				}
				nov ++;
			}
			if (string_vals.size() == 0)
				*doubleVariables[varvalue] = val;
			else
				*doubleVariables[varvalue] = string_vals.join(",").toStdString();

			*doubleVariables["nov_" + varvalue] = nov;
			if (variable_container.size() > 0) *doubleVariables["first_" + varvalue] = variable_container[0];
			else *doubleVariables["first_" + varvalue] =  0;
		}

		try
		{
			mup::Value val = p->Eval();
			//Logger(Debug) << val.ToString();
			if (!this->asVector) {
				switch (val.GetType()) {
				case 's':
					cmp->addAttribute(nameOfNewAttribute, val.GetString());
					break;
				case 'i':
					cmp->addAttribute(nameOfNewAttribute, val.GetInteger());
					break;
				case 'f':
					cmp->addAttribute(nameOfNewAttribute, val.GetFloat());
					break;
				default:
					Logger(Warning) << "Unknown type";
				}
			} else
			{
				DM::Attribute * attri = cmp->getAttribute(nameOfNewAttribute);
				std::vector<double> vD = attri->getDoubleVector();
				vD.push_back(val.GetFloat());
				attri->setDoubleVector(vD);
			}
		}
		catch (mup::ParserError &e)
		{
			Logger(Error) << e.GetMsg();
		}
	}

	foreach(std::string variable, variableNames)
		delete doubleVariables[variable];

	doubleVariables.clear();
	delete p;
}

bool AttributeCalculator::createInputDialog() {
	QWidget * w = new GUIAttributeCalculator(this);
	w->show();
	return true;
}

string AttributeCalculator::getHelpUrl()
{
	return "https://github.com/iut-ibk/DynaMind-BasicModules/blob/master/doc/AttributeCalculator.md";
}
