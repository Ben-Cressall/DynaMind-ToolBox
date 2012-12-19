/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @author  Michael Mair <abroxos@gmail.com>
 * @version 1.0
 * @section LICENSE
 * This file is part of DynaMite
 *
 * Copyright (C) 2011  Christian Urich, Michael Mair, Markus Sengthaler

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

#include <dmmodule.h>
#include <dmcomponent.h>
#include <dmattribute.h>
#include <QVariant>
#include "dmdbconnector.h"

using namespace DM;

static DbCache<Attribute*,Attribute::AttributeValue> attributeCache(1000000);

#ifdef CACHE_PROFILING
void Attribute::PrintStatistics()
{
    Logger(Standard) << "Attribute cache statistics:\t"
                     << "misses: " << (long)attributeCache.misses
                     << "\thits: " << (long)attributeCache.hits;
    attributeCache.ResetProfilingCounters();
}
#endif

QByteArray GetBinaryValue(std::vector<double> v)
{
	QByteArray bytes;
	QDataStream s(&bytes, QIODevice::WriteOnly);
	s << QVector<double>::fromStdVector(v);
	return bytes;
}

QByteArray GetBinaryValue(std::vector<std::string> v)
{
	std::vector<QString> qv;
	foreach(std::string item, v)
		qv.push_back(QString::fromStdString(item));

	QByteArray bytes;
	QDataStream s(&bytes, QIODevice::WriteOnly);
	s << QVector<QString>::fromStdVector(qv);
	return bytes;
}

QByteArray GetBinaryValue(std::vector<LinkAttribute> v)
{
	QByteArray bytes;
	QDataStream s(&bytes, QIODevice::WriteOnly);
	s << (int)v.size();
	foreach(LinkAttribute it, v)
	{
		s << QString::fromStdString(it.uuid);
		s << QString::fromStdString(it.viewname);
	}
	return bytes;
}

QByteArray GetBinaryValue(TimeSeriesAttribute a)
{
	QByteArray bytes;
	QDataStream s(&bytes, QIODevice::WriteOnly);
	int size = a.timestamp.size();
	s << size;
	for(int i=0;i<size;i++)
	{
		s << QString::fromStdString(a.timestamp[i]);
		s << a.value[i];
	}
	return bytes;
}

std::vector<LinkAttribute> ToLinkVector(QVariant q)
{
	std::vector<LinkAttribute> result;
	QByteArray bytes = q.toByteArray();
	QDataStream s(&bytes, QIODevice::ReadOnly);
	int size;
	s >> size;
	QString uuid;
	QString view;
	LinkAttribute la;
	for(int i=0;i<size;i++)
	{
		s >> uuid;
		s >> view;
		la.uuid = uuid.toStdString();
		la.viewname = view.toStdString();
		result.push_back(la);
	}
	return result;
}

std::vector<double> ToDoubleVector(QVariant q)
{
	QByteArray bytes = q.toByteArray();
	QDataStream s(&bytes, QIODevice::ReadOnly);
	QVector<double> v;
	s >> v;
	return v.toStdVector();
}

std::vector<std::string> ToStringVector(QVariant q)
{
	QByteArray bytes = q.toByteArray();
	QDataStream s(&bytes, QIODevice::ReadOnly);
	QVector<QString> qv;
	s >> qv;
	std::vector<std::string> sv;
	foreach(QString qstr, qv)
		sv.push_back(qstr.toStdString());

	return sv;
}

TimeSeriesAttribute ToTimeSeriesAttribute(QVariant q)
{
	QByteArray bytes = q.toByteArray();
	QDataStream s(&bytes, QIODevice::ReadOnly);
	TimeSeriesAttribute ts;
	int size;
	s >> size;
	for(int i=0;i<size;i++)
	{
		QString str;
		s >> str;
		double d;
		s >> d;

		ts.timestamp.push_back(str.toStdString());
		ts.value.push_back(d);
	}
	return ts;
}

Attribute::AttributeValue::AttributeValue(const AttributeValue& ref)
{
	this->type = ref.type;
	//this->size = ref.size;
	switch(ref.type)
	{
	case NOTYPE:
		ptr = NULL;
		break;
	case DOUBLE:
		ptr = new double(*((double*)ref.ptr));
		break;
	case STRING:
		ptr = new std::string(*((std::string*)ref.ptr));
		break;
	case TIMESERIES:
		ptr = new TimeSeriesAttribute(*((TimeSeriesAttribute*)ref.ptr));
		break;
	case LINK:
		ptr = new LinkAttribute(*((LinkAttribute*)ref.ptr));
		break;
	case DOUBLEVECTOR:
		ptr = new std::vector<double>(*((std::vector<double>*)ref.ptr));
		break;
	case STRINGVECTOR:
		ptr = new std::vector<std::string>(*((std::vector<std::string>*)ref.ptr));
		break;
	default: 
		type = NOTYPE;
		ptr = NULL;
		break;
	}
}

Attribute::AttributeValue::AttributeValue(QVariant var, AttributeType type)
{
	this->type = type;
	switch(type)
	{
	case NOTYPE:	ptr = NULL;
		break;
	case DOUBLE:	ptr = new double(var.toDouble());
		break;
	case STRING:	ptr = new std::string(var.toString().toStdString());
		break;
	case TIMESERIES:ptr = new TimeSeriesAttribute(ToTimeSeriesAttribute(var));
		break;
	case LINK:		ptr = new std::vector<LinkAttribute>(ToLinkVector(var));
		break;
	case DOUBLEVECTOR:	ptr = new std::vector<double>(ToDoubleVector(var));
		break;
	case STRINGVECTOR:	ptr = new std::vector<std::string>(ToStringVector(var));
		break;
	default: ptr = NULL;
		type = NOTYPE;
		break;
	}
}
Attribute::AttributeValue::AttributeValue(double d)
{
	type = DOUBLE;
	ptr = new double(d);
}
Attribute::AttributeValue::AttributeValue(std::string string)
{
	type = STRING;
	ptr = new std::string(string);
}

QVariant Attribute::AttributeValue::toQVariant()
{
	switch(type)
	{
	case NOTYPE:	return QVariant();
	case DOUBLE:	return QVariant::fromValue(*(double*)ptr);
	case STRING:	return QString::fromStdString(*(std::string*)ptr);
	case TIMESERIES:return GetBinaryValue(*(TimeSeriesAttribute*)ptr);
	case LINK:		return GetBinaryValue(*(std::vector<LinkAttribute>*)ptr);
	case DOUBLEVECTOR:	return GetBinaryValue(*(std::vector<double>*)ptr);
	case STRINGVECTOR:	return GetBinaryValue(*(std::vector<std::string>*)ptr);
	default:		return QVariant();
	}
}

Attribute::Attribute()
{
	_uuid = QUuid::createUuid();
    name="";
	value = new AttributeValue();
	owner = NULL;
	isInserted = false;
	//DBConnector::getInstance();
    //SQLInsert(NOTYPE);
}

Attribute::Attribute(const Attribute &newattribute)
{
	_uuid = QUuid::createUuid();
    name=newattribute.name;
	value = new AttributeValue(*newattribute.getValue());
	owner = NULL;
	isInserted = false;
    //AttributeType type = newattribute.getType();
    //QVariant value;
    //newattribute.SQLGetValue(value);
    //SQLInsert(type, value);
    //SQLInsertThis(type);
    //SQLSetValue(type, value);
}

Attribute::Attribute(std::string name)
{
	_uuid = QUuid::createUuid();
    this->name=name;
	owner = NULL;
	value = new AttributeValue();
	isInserted = false;
    //SQLInsert(NOTYPE);
}


Attribute::Attribute(std::string name, double val)
{
	_uuid = QUuid::createUuid();
    this->name=name;
	owner = NULL;
	isInserted = false;
	value = new AttributeValue(val);
    //SQLInsert(DOUBLE);
}
Attribute::Attribute(std::string name, std::string val)
{
	_uuid = QUuid::createUuid();
    this->name=name;
	owner = NULL;
	isInserted = false;
	value = new AttributeValue(val);
    //SQLInsert(STRING);
}

Attribute::~Attribute()
{
	if(value)	delete value;
	if(isInserted)
	{
		DBConnector::getInstance()->Delete("attributes", _uuid);
		attributeCache.remove(this);
	}
	//SQLDeleteThis();
}

Attribute::AttributeType Attribute::getType() const
{
	return getValue()->type;
    //return SQLGetType();
}

void Attribute::setName(std::string name)
{
    this->name=name;
	//SQLSetName(name);
}

std::string Attribute::getName()
{
    return name;
}

void Attribute::setDouble(double v)
{
	AttributeValue* a = getValue();
	a->Free();
	a->type = DOUBLE;
	a->ptr = new double(v);
    //SQLSetValue(DOUBLE, v);
}

double Attribute::getDouble()
{
	AttributeValue* a = getValue();
	if(a->type == DOUBLE)	return *((double*)a->ptr);

	//QVariant value;
    //if(SQLGetValue(value))	return value.toDouble();
    return 0;
}

void Attribute::setString(std::string s)
{
	AttributeValue* a = getValue();
	a->Free();
	a->type = STRING;
	a->ptr = new std::string(s);
    //SQLSetValue(STRING, QString::fromStdString(s).toAscii());
}

std::string Attribute::getString()
{	
	AttributeValue* a = getValue();
	if(a->type == STRING)	return *((std::string*)a->ptr);
    //QVariant value;
    //if(SQLGetValue(value))	return QString(value.toByteArray()).toStdString();
    return "";
}

void Attribute::setDoubleVector(std::vector<double> v)
{
	AttributeValue* a = getValue();
	a->Free();
	a->type = DOUBLEVECTOR;
	a->ptr = new std::vector<double>(v);
	//SQLSetValue(DOUBLEVECTOR, GetBinaryValue(v));
}

std::vector<double> Attribute::getDoubleVector()
{
	AttributeValue* a = getValue();
	if(a->type == DOUBLEVECTOR)	return *((std::vector<double>*)a->ptr);
	//QVariant value;
	//if(SQLGetValue(value))	return ToDoubleVector(value);
	return std::vector<double>();
}

void Attribute::setStringVector(std::vector<std::string> s)
{
	AttributeValue* a = getValue();
	a->Free();
	a->type = STRINGVECTOR;
	a->ptr = new std::vector<std::string>(s);
	//SQLSetValue(STRINGVECTOR, GetBinaryValue(s));
}

std::vector<std::string> Attribute::getStringVector()
{
	AttributeValue* a = getValue();
	if(a->type == STRINGVECTOR)	return *((std::vector<std::string>*)a->ptr);
	//QVariant value;
	//if(SQLGetValue(value))	return ToStringVector(value);
	return std::vector<string>();
}

bool Attribute::hasDouble()
{
    if (getType() == Attribute::DOUBLE)
        return true;
    return false;
}
bool Attribute::hasDoubleVector()
{
    if (getType() == Attribute::DOUBLEVECTOR)
        return true;
    return false;
}
bool Attribute::hasString()
{
    if (getType() == Attribute::STRING)
        return true;
    return false;
}
bool Attribute::hasStringVector()
{
    if (getType() == Attribute::STRINGVECTOR)
        return true;
    return false;
}

void Attribute::setLink(string viewname, string uuid)
{
	std::vector<LinkAttribute> links = getLinks();
	LinkAttribute att;
	att.uuid = uuid;
	att.viewname = viewname;
    links.push_back(att);
	setLinks(links);
}

void Attribute::setLinks(std::vector<LinkAttribute> links)
{
	AttributeValue* a = getValue();
	a->Free();
	a->type = LINK;
	a->ptr = new std::vector<LinkAttribute>(links);
	//SQLSetValue(LINK, GetBinaryValue(links));
}

LinkAttribute Attribute::getLink()
{
	AttributeValue* a = getValue();
	if(a->type == LINK)	return (*((std::vector<LinkAttribute>*)a->ptr))[0];
	/*QVariant value;
	if(SQLGetValue(value))
	{
		std::vector<LinkAttribute> linkvector = ToLinkVector(value);
		if(linkvector.size())
            return linkvector[0];
	}*/
	return LinkAttribute();
}

std::vector<LinkAttribute> Attribute::getLinks()
{
	AttributeValue* a = getValue();
	if(a->type == LINK)	return *((std::vector<LinkAttribute>*)a->ptr);
	return std::vector<LinkAttribute>();
	/*std::vector<LinkAttribute> linkvector;
	QVariant value;
	if(SQLGetValue(value))
        linkvector = ToLinkVector(value);
    return linkvector;*/
}

void Attribute::addTimeSeries(std::vector<std::string> timestamp, std::vector<double> value)
{
    if(timestamp.size()!=value.size())
    {
        DM::Logger(DM::Error) << "Length of time and value vector are not equal";
        return;
    }
	AttributeValue* a = getValue();
	a->Free();
	a->type = TIMESERIES;
	a->ptr = new TimeSeriesAttribute(&timestamp, &value);
	/*
	QByteArray qba;
	QDataStream s(&qba, QIODevice::WriteOnly);
	s << (int)value.size();
	for(unsigned int i=0;i<value.size();i++)
	{
		s << value[i];
		s << QString::fromStdString(timestamp[i]);
	}
	SQLSetValue(LINK, qba);*/
}

void Attribute::getTimeSeries(std::vector<std::string> *timestamp, std::vector<double> *value)
{
	AttributeValue* a = getValue();
	if(a->type == TIMESERIES)	
	{
		*timestamp = ((TimeSeriesAttribute*)a->ptr)->timestamp;
		*value = ((TimeSeriesAttribute*)a->ptr)->value;
	}
	/*
	QVariant data;
	SQLGetValue(data);
	QByteArray qba = data.toByteArray();
	QDataStream s(&qba, QIODevice::ReadOnly);
	int size;
	s >> size;

	if(size==0)
	{
        DM::Logger(DM::Error) << "timestamp or value are equal null";
        return;
    }
	
	double dbl;
	QString str;
	for(int i=0;i<size;i++)
	{
		s >> dbl;
		s >> str;
		value->push_back(dbl);
		timestamp->push_back(str.toStdString());
    }*/
}

void Attribute::setType(AttributeType type)
{
	AttributeValue* a = getValue();
	a->Free();
	a->type = type;
	switch(type)
	{
	case NOTYPE:
		a->ptr = NULL;
		break;
	case DOUBLE:
		a->ptr = new double(0);
		break;
	case STRING:
		a->ptr = new std::string();
		break;
	case TIMESERIES:
		a->ptr = new TimeSeriesAttribute();
		break;
	case LINK:
		a->ptr = new LinkAttribute();
		break;
	case DOUBLEVECTOR:
		a->ptr = new std::vector<double>();
		break;
	case STRINGVECTOR:
		a->ptr = new std::vector<std::string>();
		break;
	default: a->ptr = NULL;
		a->type = NOTYPE;
		break;
	}
    //SQLSetType(type);
}
void Attribute::Change(Attribute &attribute)
{
    name=attribute.name;
	//owner = attribute.owner;
	value = new AttributeValue(*attribute.value);
    //QVariant value;
    //attribute.SQLGetValue(value);
    //this->SQLSetValue(attribute.getType(), value);
}

const char *Attribute::getTypeName() const
{
	return Attribute::getTypeName(this->getType());
}

const char *Attribute::getTypeName(Attribute::AttributeType type)
{
    const char *arr[] = {
        "NOTYPE",
        "DOUBLE",
        "STRING",
        "TIMESERIES",
        "LINK",
        "DOUBLEVECTOR",
        "STRINGVECTOR"
    };
    return arr[type];
}

Attribute::AttributeValue* Attribute::getValue() const
{
	if(value)	return value;
	else		return attributeCache.get((Attribute*)this);
}
/*
void Attribute::SQLInsert(AttributeType type)
{
    _uuid = QUuid::createUuid();

    DBConnector::getInstance()->Insert("attributes", _uuid,
                                       "name", QString::fromStdString(name),
                                       "type", QVariant::fromValue((int)type));
	attributeCache.add(_uuid, new RawAttribute(name,"",type,0));
}

void Attribute::SQLInsert(AttributeType type, QVariant value)
{
    _uuid = QUuid::createUuid();

    DBConnector::getInstance()->Insert("attributes", _uuid,
                                       "name", QString::fromStdString(name),
                                       "type", QVariant::fromValue((int)type),
                                       "value", value);
    attributeCache.add(_uuid, new RawAttribute(name,"",type,value));
}
void Attribute::SQLDeleteThis()
{
    DBConnector::getInstance()->Delete("attributes", _uuid);
    attributeCache.remove(_uuid);
}
*/
void Attribute::SetOwner(Component* owner)
{
	this->owner = owner;
	attributeCache.add(this, value);
	value = NULL;
	// TODO: make shure its not bound to another component
    //DBConnector::getInstance()->Update("attributes", _uuid,
    //                                   "owner",     QString::fromStdString(owner->getUUID()));
}
/*
void Attribute::SQLSetName(std::string newname)
{	
    if(RawAttribute *att = attributeCache.get(_uuid))
    {
        att->name = name;
        attributeCache.get(_uuid);  // push to top
    }

    DBConnector::getInstance()->Update("attributes", _uuid,
                                       "name",      QString::fromStdString(newname));
}
void Attribute::SQLSetType(AttributeType newtype)
{
    SQLSetValue(newtype,0);
}

Attribute::AttributeType Attribute::SQLGetType() const
{
    if(RawAttribute *att = attributeCache.get(_uuid))
        return att->type;

    QVariant value;
    if(DBConnector::getInstance()->Select("attributes", _uuid, "type", &value))
        return (AttributeType)value.toInt();
    return NOTYPE;
}

bool Attribute::SQLGetValue(QVariant &value) const
{
    if(RawAttribute *att = attributeCache.get(_uuid))
    {
        value = att->value;
        return true;
    }
    return DBConnector::getInstance()->Select("attributes",
                                              _uuid, "value", &value);
}
void Attribute::SQLSetValue(AttributeType type, QVariant value)
{
    if(RawAttribute *att = attributeCache.get(_uuid))
    {
        att->type = type;
        att->value = value;
        attributeCache.get(_uuid);  // push to top
    }

    DBConnector::getInstance()->Update("attributes", _uuid,
                                        "type",      QVariant::fromValue((int)type),
                                        "value",     value);
}
*/

Attribute::AttributeValue* Attribute::LoadFromDb()
{
	QVariant t,v;
    DBConnector::getInstance()->Select("attributes", _uuid,
                                       "type",     &t,
                                       "value",     &v);
    return new AttributeValue(t,(AttributeType)v.toInt());
}

void Attribute::SaveToDb(Attribute::AttributeValue *val)
{
	if(isInserted)
    {
        DBConnector::getInstance()->Update("attributes", _uuid,
											"type",     QVariant::fromValue((int)val->type),
											"value",     val->toQVariant());
    }
    else
    {
        DBConnector::getInstance()->Insert("attributes", _uuid,
											"type",     QVariant::fromValue((int)val->type),
											"value",     val->toQVariant());
        isInserted = true;
    }
}