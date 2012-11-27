/**
 * @file
 * @author  Chrisitan Urich <christian.urich@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2011  Christian Urich, Markus Sengthaler

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
#include <dmcomponent.h>
#include "dmrasterdata.h"
#include <QPointF>
#include <time.h>
#include <QMutex>
#include <dmlogger.h>
#include <math.h>
#include "dmdbconnector.h"
#include <QVariant>

using namespace DM;

RasterData::RasterData(long width, long height, double cellsizeX, double cellsizeY, double xoffset, double yoffset) : Component(true)
{
    //DM::Logger(DM::Warning) << "New implemented RasterData --- please check existing code";

    this->width = width;
    this->height = height;
    this->cellSizeX = cellsizeX;
    this->cellSizeY = cellsizeY;
    this->xoffset = xoffset;
    this->yoffset = yoffset;

    this->minValue = -9999;
    this->maxValue = -9999;
    this->NoValue = -9999;
    this->debugValue = 0;
    //this->_linkID = 0;

	SQLInsert();
    SQLInsertField(width, height);
}
RasterData::RasterData() : Component(true)
{
    this->cellSizeX = 0;
    this->cellSizeY = 0;
    this->xoffset = 0;
    this->yoffset = 0;
    this->width = 0;
    this->height = 0;
    //this->_linkID = 0;

    SQLInsert();
}
RasterData::RasterData(const RasterData &other) : Component(other, true)
{
    this->width = other.width;
    this->height = other.height;
    this->NoValue = other.NoValue;
    this->minValue = other.minValue;
    this->maxValue = other.maxValue;
    this->debugValue = other.debugValue;

    this->cellSizeX = other.cellSizeX;
    this->cellSizeY = other.cellSizeY;
    this->xoffset = other.xoffset;
    this->yoffset = other.yoffset;
    //this->_linkID = 0;

	SQLInsert();
    SQLInsertField(width, height);

    /*
	for(long x=0;x<3;x++)
		for(long y=0;y<3;y++)
            this->setValue(x,y,other.getValue(x,y));*/
    for(long x=0;x<height;x++)
        this->SQLSetRow(x, other.SQLGetRow(x));
}

Components RasterData::getType()
{
	return DM::RASTERDATA;
}
QString RasterData::getTableName()
{
    return "rasterdatas";
}
double RasterData::getSum() const
{
    double sum = 0;
    for ( long i = 0; i < width; i++ )
	{
        for (  long j = 0; j < height; j++ ) 
        {
            double val = SQLGetValue(i,j);
            if ( val != NoValue)
                sum += val;
        }
    }
    return sum;
}

double RasterData::getCell(long x, long y) const
{
    if (  x >-1 && y >-1 && x < this->width && y < this->height) 
        return  SQLGetValue(x,y);
    else
        return  this->NoValue;
}

bool RasterData::setCell(long x, long y, double value)
{
    if (  x >-1 && y >-1 && x < this->width && y < this->height) 
	{
		SQLSetValue(x,y,value);

        if (minValue == this->NoValue || minValue > value)
            minValue = value;

        if (maxValue == this->NoValue || maxValue < value)
            maxValue = value;

        return true;
    } 
    return false;
}

RasterData::~RasterData()
{
    SQLDeleteField();
    Component::SQLDelete();
}

void RasterData::getNeighboorhood(float** d, int width, int height, int x, int y) {
    int dx = (int) (width -1)/2;
    int dy = (int) (height -1)/2;
    int x_cell;
    int y_cell;

    int k = 0;
    for ( long i = x-dx; i <= x + dx; i++ ) {
        x_cell = i;
        if ( i < 0) {
            x_cell = this->width + i;
        }
        if ( i >= this->width) {
            x_cell = i - this->width;
        }
        int l = 0;
        for ( long j = y-dy; j <= y + dy; j++ ) {
            y_cell = j;
            if ( j < 0) {
                y_cell = this->height + j;
            }
            if ( j >= this->height) {
                y_cell = j - this->height;
            }
            d[k][l] = this->getCell(x_cell,y_cell);

            l++;
        }
        k++;
    }
}
void RasterData::getMoorNeighbourhood(std::vector<double> &neigh, long x, long y) {
    int counter = 0;
    for ( long j = y-1; j <= y + 1; j++ ) {
        for ( long i = x-1; i <= x + 1; i++ ) {

            neigh[counter] = this->getCell(i,j);
            counter++;
        }
    }
    if ( x-1 < 0 && y-1 < 0) {
        neigh[0] = neigh[8];
        neigh[1] = neigh[7];
        neigh[2] = neigh[8];
        neigh[3] = neigh[5];
        neigh[6] = neigh[8];

    } else if ( x+1 >=  this->width && y-1 < 0) {
        neigh[0] = neigh[6];
        neigh[1] = neigh[7];
        neigh[2] = neigh[6];
        neigh[5] = neigh[3];
        neigh[8] = neigh[6];

    } else if ( x+1 >= this->width && y+1 >= this->height) {
        neigh[2] = neigh[0];
        neigh[5] = neigh[3];
        neigh[8] = neigh[0];
        neigh[7] = neigh[1];
        neigh[6] = neigh[0];

    } else if ( x-1 < 0 && y+1 >=  this->height) {
        neigh[0] = neigh[2];
        neigh[3] = neigh[5];
        neigh[6] = neigh[2];
        neigh[7] = neigh[1];
        neigh[8] = neigh[2];

    } else if ( x-1 < 0 ) {
        neigh[0] = neigh[2];
        neigh[3] = neigh[5];
        neigh[6] = neigh[8];

    } else if ( y-1 < 0 ) {
        neigh[0] = neigh[6];
        neigh[1] = neigh[7];
        neigh[2] = neigh[8];

    } else if (x+1 >= this->width ) {
        neigh[2] = neigh[0];
        neigh[5] = neigh[3];
        neigh[8] = neigh[6];

    }
    else if (y+1 >= this->height ) {
        neigh[6] = neigh[0];
        neigh[7] = neigh[1];
        neigh[8] = neigh[2];

    }

}
std::vector<double>  RasterData::getMoorNeighbourhood(long x, long y) const {
    std::vector<double> neigh(9);
    int counter = 0;
    for ( long j = y-1; j <= y + 1; j++ ) {
        for ( long i = x-1; i <= x + 1; i++ ) {

            neigh[counter] = this->getCell(i,j);
            counter++;
        }
    }
    if ( x-1 < 0 && y-1 < 0) {
        neigh[0] = neigh[8];
        neigh[1] = neigh[7];
        neigh[2] = neigh[8];
        neigh[3] = neigh[5];
        neigh[6] = neigh[8];

    } else if ( x+1 >=  this->width && y-1 < 0) {
        neigh[0] = neigh[6];
        neigh[1] = neigh[7];
        neigh[2] = neigh[6];
        neigh[5] = neigh[3];
        neigh[8] = neigh[6];

    } else if ( x+1 >= this->width && y+1 >= this->height) {
        neigh[2] = neigh[0];
        neigh[5] = neigh[3];
        neigh[8] = neigh[0];
        neigh[7] = neigh[1];
        neigh[6] = neigh[0];

    } else if ( x-1 < 0 && y+1 >=  this->height) {
        neigh[0] = neigh[2];
        neigh[3] = neigh[5];
        neigh[6] = neigh[2];
        neigh[7] = neigh[1];
        neigh[8] = neigh[2];

    } else if ( x-1 < 0 ) {
        neigh[0] = neigh[2];
        neigh[3] = neigh[5];
        neigh[6] = neigh[8];

    } else if ( y-1 < 0 ) {
        neigh[0] = neigh[6];
        neigh[1] = neigh[7];
        neigh[2] = neigh[8];

    } else if (x+1 >= this->width ) {
        neigh[2] = neigh[0];
        neigh[5] = neigh[3];
        neigh[8] = neigh[6];

    }
    else if (y+1 >= this->height ) {
        neigh[6] = neigh[0];
        neigh[7] = neigh[1];
        neigh[8] = neigh[2];

    }
    return neigh;

}

void RasterData::setSize(long width, long height, double cellsizeX, double cellsizeY, double xoffset, double yoffset) {
    if (width != this->width || height != this->height || this->cellSizeX != cellsizeX || this->cellSizeY != cellsizeY)
    {
        this->width = width;
        this->height = height;
        this->cellSizeX = cellsizeX;
        this->cellSizeY = cellsizeY;
        this->xoffset = xoffset;
        this->yoffset = yoffset;
        this->NoValue = -9999;
        this->minValue = -9999;
        this->maxValue = -9999;

        SQLInsertField(width, height);
    }
}

void RasterData::clear() {
    for (int y = 0; y < this->height; y++)
        for (int x = 0; x < this->width; x++)
            SQLSetValue(x,y,0);
}

Component * RasterData::clone() {
    return new RasterData(*this);
}

void RasterData::SQLInsert()
{
    DBConnector::getInstance()->Insert("rasterdatas", uuid.toRfc4122());
}


bool RasterData::setValue(long x, long y, double value)
{
    return setCell((int)((x-xoffset)/cellSizeX),(int)((y-yoffset)/cellSizeY),value);
}

double RasterData::getValue(long x, long y) const
{
    return getCell((int)((x-xoffset)/cellSizeX),(int)((y-yoffset)/cellSizeY));
}

void RasterData::SQLInsertField(long width, long height)
{
	if(width == 0 || height == 0)
		return;
    //if(GetLinkID()!=0)
    //if(_linkID!=0)
    //    return;

    //int linkID = DBConnector::GetNewLinkID();
    //this->_linkID = linkID;
    //SQLUpdateLink(linkID);

    double* buffer = new double[width];
    for(long x = 0; x < height; x++)
    {
        /*
        QByteArray qba;
        QDataStream stream(&qba, QIODevice::WriteOnly);

        for(long y = 0; y < height; y++)
            stream << NoValue;
*/
        for(long y = 0; y < width; y++)
            buffer[y] = NoValue;
        QByteArray qba((char*)buffer, sizeof(double)*width);

        QSqlQuery *q = DBConnector::getInstance()->getQuery("INSERT INTO rasterfields(owner,x,data) VALUES (?,?,?)");
        q->addBindValue(uuid.toRfc4122());
        q->addBindValue(QVariant::fromValue(x));
        q->addBindValue(qba.toBase64());
        DBConnector::getInstance()->ExecuteQuery(q);
    }
    delete buffer;
/*
    QString strQuery;
    QSqlQuery q;
    DBConnector *db = DBConnector::getInstance();

    long maxUnions = 500;
    long counter = maxUnions;
    for(long x = 0; x < width; x++)
    {
        for(long y = 0; y < height; y++)
        {
            counter++;
            if(counter>=maxUnions)
            {
                if(x!=0 && y!=0)
                {
                    QSqlQuery *q = db->getQuery(strQuery);
                    db->ExecuteQuery(q);
                }

                strQuery = "INSERT INTO rasterfields \n SELECT "
                        +QString::number(linkID)+" AS datalink, "
                        +QString::number(x)+" AS x, "
                        +QString::number(y)+" AS y, "
                        +QString::number(NoValue)+" AS value\n";
                counter = 0;
            }
            else
            {
                strQuery += "UNION SELECT "
                        +QString::number(linkID)+","
                        +QString::number(x)+","
                        +QString::number(y)+","
                        +QString::number(NoValue)+"\n";
            }
        }
    }

    if(counter!=0)
    {
        QSqlQuery *q = db->getQuery(strQuery);
        db->ExecuteQuery(q);
        db->CommitTransaction();
    }*/
    /*
	for(long x = 0; x < width; x++)
	{
		for(long y = 0; y < height; y++)
        {
            QSqlQuery *q = DBConnector::getInstance()->getQuery("INSERT INTO rasterfields(datalink,x,y,value) VALUES (?,?,?,?)");
            q->addBindValue(linkID);
            q->addBindValue(QVariant::fromValue(x));
            q->addBindValue(QVariant::fromValue(y));
            q->addBindValue(NoValue);
            DBConnector::getInstance()->ExecuteQuery(q);
		}
    }*/
}
void RasterData::SQLDeleteField()
{
    if(width==0 || height==0)
        return;

    QSqlQuery *q = DBConnector::getInstance()->getQuery("DELETE FROM rasterfields WHERE owner LIKE ?");
    q->addBindValue(uuid.toRfc4122());
    DBConnector::getInstance()->ExecuteQuery(q);
}
QByteArray RasterData::SQLGetRow(long x) const
{
    QSqlQuery *q = DBConnector::getInstance()->getQuery("SELECT data FROM rasterfields WHERE owner LIKE ? AND x=?");
    q->addBindValue(uuid.toRfc4122());
    q->addBindValue(QVariant::fromValue(x));
    if(!DBConnector::getInstance()->ExecuteSelectQuery(q))
        return QByteArray();

    return QByteArray::fromBase64(q->value(0).toByteArray());
}

double RasterData::SQLGetValue(long x, long y) const
{
    QByteArray qba = SQLGetRow(x);
    double* darray = (double*)qba.data();
    return darray[y];
}
void RasterData::SQLSetValue(long x, long y, double value)
{
    QByteArray qba = SQLGetRow(x);
    ((double*)qba.data())[y] = value;
    SQLSetRow(x,qba);
}
void RasterData::SQLSetRow(long x, QByteArray data)
{
    QSqlQuery *q = DBConnector::getInstance()->getQuery("UPDATE rasterfields SET data = ? WHERE owner LIKE ? AND x=?");
    q->addBindValue(data.toBase64());
    q->addBindValue(uuid.toRfc4122());
    q->addBindValue(QVariant::fromValue(x));
    DBConnector::getInstance()->ExecuteQuery(q);
}
/*
void RasterData::SQLUpdateLink(int id)
{
    DBConnector::getInstance()->Update("rasterdatas", uuid.toRfc4122(),
                                                      QString::fromStdString(stateUuid),
                                       "datalink",    QVariant::fromValue(id));
}*/
/*
int RasterData::GetLinkID() const
{
    QVariant value;
    if(DBConnector::getInstance()->Select("rasterdatas", uuid.toRfc4122(),
                                                         QString::fromStdString(stateUuid),
                                          "datalink",    &value))
        return value.toInt();
	return 0;
}*/
