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
#include "dmdbconnector.h"

using namespace DM;
/*
RasterData::RasterData(QByteArray qba):Component()
{
	QDataStream stream(&qba, QIODevice::ReadOnly);
	qint64 l;
	stream >> l;	
	width = l;

	stream >> l;	
	height = l;

	stream >> cellSize;
	stream >> NoValue;
	stream >> minValue;
	stream >> maxValue;

	stream >> debugValue;
	//stream >> isClone;
}

QByteArray RasterData::GetValue()
{
	QByteArray bytes;
	QDataStream stream(&bytes, QIODevice::WriteOnly);
	stream << width;
	stream << height;
	stream << cellSize;
	stream << NoValue;
	stream << minValue;
	stream << maxValue;
	stream << debugValue;
	//stream << isClone;

	return bytes;
}
*/
RasterData::RasterData(long  width, long  height, double  cellSize) : Component()
{
    this->width = width;
    this->height = height;
    this->cellSize = cellSize;
    this->NoValue = -9999;
    this->minValue = -9999;
    this->maxValue = -9999;
    this->debugValue = 0;
	linkID = 0;
	/*
    data = new double*[width];
    for (long i = 0; i < width; i++) {
        data[i] = new double[height];
    }*/
	SQLInsert();
	SQLInsertField(width, height, NoValue);
}
RasterData::RasterData() : Component() {
    this->cellSize = 0;
    this->width = 0;
    this->height = 0;
	linkID = 0;
    //this->data = 0;
    //this->isClone = false;

	SQLInsert();
	//SQLSetValues();
}
RasterData::RasterData(const RasterData &other) : Component(other) 
{
    this->cellSize = other.cellSize;
    this->width = other.width;
    this->height = other.height;
    this->NoValue = other.NoValue;
    this->minValue = other.minValue;
    this->maxValue = other.maxValue;
    this->debugValue = other.debugValue;
	linkID = 0;
	/*
    data = new double*[width];
    for (long i = 0; i < width; i++)
	{
        data[i] = new double[height];
		memcpy(data[i],other.data[i],sizeof(double)*height);
	}*/

    //this->isClone = true;

	SQLInsert();
	SQLInsertField(width, height, NoValue);

	// TODO copy values!!

	//SQLSetValues();
}

Components RasterData::getType()
{
	return DM::RASTERDATA;
}

double RasterData::getSum() const
{
    double sum = 0;
    for ( long i = 0; i < width; i++ )
	{
        for (  long j = 0; j < height; j++ ) 
		{
            //double val = data[i][j];
            double val = SQLGetValue(i,j);
            if ( val != NoValue)
                sum += val;
        }
    }
    return sum;
}
double RasterData::getValue(long x, long y) const {
    if (  x >-1 && y >-1 && x < this->width && y < this->height) 
        return  SQLGetValue(x,y);
        //return  data[x][y];
    else
        return  this->NoValue;
}
/*
void RasterData::createNewDataSet() {
    double **data_old = this->data;
    this->minValue = 0;
    this->maxValue = 0;

    data = new double*[width];
    for (long i = 0; i < width; i++) {
        data[i] = new double[height];
    }

    for (unsigned int i = 0; i < getWidth(); i++) {
        for (unsigned int j = 0; j < getHeight();j++) {
            data[i][j] =  data_old[i][j];
        }
    }

    this->isClone = false;
	SQLSetValues();
}*/

bool RasterData::setValue(long x, long y, double value) 
{
    //if (this->isClone == true)
    //    this->createNewDataSet();

    if (  x >-1 && y >-1 && x < this->width && y < this->height) 
	{
		SQLSetValue(x,y,value);
        //data[x][y] = value;

        if (minValue == this->NoValue || minValue > value)
            minValue = value;

        if (maxValue == this->NoValue || maxValue < value)
            maxValue = value;
		
        return true;
    } 
    return false;
}

RasterData::~RasterData() {

//    if (isClone)
//        return;
    /*for (long i = 0; i < width; i++) {
        delete[] this->data[i];
    }
    if (height != 0)
        delete[] data;
    data = 0;
	*/
	SQLDelete();
}

void RasterData::getNeighboorhood(double** d, int width, int height, int x, int y) {
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
            d[k][l] = this->getValue(x_cell,y_cell);
       /*     if (d[k][l] < 0 ) {
                vibens::Logger( vibens::Debug) << "Error";
            }*/

            l++;
        }
        k++;
    }
}
void RasterData::getMoorNeighbourhood(std::vector<double> &neigh, long x, long y) {
    int counter = 0;
    for ( long j = y-1; j <= y + 1; j++ ) {
        for ( long i = x-1; i <= x + 1; i++ ) {

            neigh[counter] = this->getValue(i,j);
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

            neigh[counter] = this->getValue(i,j);
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


void RasterData::setSize(long width, long height, double cellsize) 
{
    if (width != this->width || height != this->height || this->cellSize != cellsize) 
	{
        this->width = width;
        this->height = height;
        this->cellSize = cellsize;
        this->NoValue = -9999;
        this->minValue = -9999;
        this->maxValue = -9999;
		/*
        data = new double*[width];
        for (long i = 0; i < width; i++)
            data[i] = new double[height];*/
		SQLInsertField(width, height, NoValue);
    }
	//SQLSetValues();
}

void RasterData::clear() {
    for (int y = 0; y < this->height; y++)
        for (int x = 0; x < this->width; x++)
			SQLSetValue(x,y,0);
            //this->data[x][y] = 0;
}

Component * RasterData::clone() {
    return new RasterData(*this);
}

void RasterData::SQLInsert()
{
	SQLInsertAs("rasterdata");
	//SQLSetValues();
}
void RasterData::SQLDelete()
{
	SQLDeleteAs("rasterdata");
	SQLDeleteField();
}
void RasterData::SQLInsertField(long width, long height, double value)
{
	if(linkID!=0)
		return;

	linkID = DBConnector::GetNewLinkID();
	SQLUpdateLink(linkID);
	QString strQuery = "INSERT INTO rasterfields(datalink,x,y,value) VALUES ";

	for(long x = 0; x < width; x++)
	{
		for(long y = 0; y < height; y++)
		{
			strQuery += "(";
			strQuery += QString::number(linkID);
			strQuery += ",";
			strQuery += QString::number(x);
			strQuery += ",";
			strQuery += QString::number(y);
			strQuery += ",";
			strQuery += QString::number(NoValue);
			strQuery += "),";
		}
	}
	// eliminate the last ,
	strQuery.chop(1);

	QSqlQuery q;
	q.prepare(strQuery);
	if(!q.exec())	PrintSqlError(&q);
}
void RasterData::SQLDeleteField()
{
	if(linkID==0)
		return;

	QSqlQuery q;
	q.prepare("DELETE FROM rasterfields WHERE datalink=?");
	q.addBindValue(linkID);
	linkID = 0;
	if(!q.exec())	PrintSqlError(&q);
}
double RasterData::SQLGetValue(long x, long y) const
{
	QSqlQuery q;
	q.prepare("SELECT value FROM rasterfields WHERE datalink=? AND x=? AND y=?");
	q.addBindValue(linkID);
	q.addBindValue(x);
	q.addBindValue(y);
	if(!q.exec())	PrintSqlError(&q);
	if(q.next())
	{
		return q.value(0).toDouble();
	}
	return NoValue;
}
void RasterData::SQLSetValue(long x, long y, double value)
{
	QSqlQuery q;
	q.prepare("UPDATE rasterfields SET value = ? WHERE datalink=? AND x=? AND y=?");
	q.addBindValue(value);
	q.addBindValue(linkID);
	q.addBindValue(x);
	q.addBindValue(y);
	if(!q.exec())	PrintSqlError(&q);
}
void RasterData::SQLUpdateLink(int id)
{
	QSqlQuery q;
	q.prepare("UPDATE rasterdatas SET datalink = ? WHERE uuid LIKE ? AND stateuuid LIKE ?");
	q.addBindValue(id);
	q.addBindValue(QString::fromStdString(uuid));
	q.addBindValue(QString::fromStdString(stateUuid));
	if(!q.exec())	PrintSqlError(&q);
}


