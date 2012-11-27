/**
 * @file
 * @author  Markus Sengthaler <m.sengthaler@gmail.com>
 * @version 1.0
 * @section LICENSE
 * This file is part of DynaMind
 *
 * Copyright (C) 2011  Markus Sengthaler

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


#ifndef DMDBCONNECTOR_H
#define DMDBCONNECTOR_H
#include <dmcompilersettings.h>

#include <QSqlQuery>

namespace DM {

void DM_HELPER_DLL_EXPORT PrintSqlError(QSqlQuery *q);

class DMSqlParameter
{
public:
    QString name;
    QVariant value;
    DMSqlParameter(QString name, QVariant value)
    {
        this->name = name;
        this->value = value;
    }
};

class SingletonDestroyer;

class DM_HELPER_DLL_EXPORT DBConnector
{
    friend class SingletonDestroyer;
private:
	static DBConnector* instance;
    DBConnector();
	
    //static int _linkID;
    static QMap<QString,QSqlQuery*> mapQuery;
    static bool _bTransaction;
    static QSqlDatabase _db;

    static SingletonDestroyer _destroyer;
    bool CreateTables();
    bool DropTables();

protected:
    virtual ~DBConnector();
public:

    QSqlQuery *getQuery(QString cmd);
    void ExecuteQuery(QSqlQuery *q);
    bool ExecuteSelectQuery(QSqlQuery *q);

    static DBConnector* getInstance();
    void BeginTransaction();
    void CommitTransaction();
    // inserts with uuid
    void Insert(QString table,  QByteArray uuid);
    void Insert(QString table,  QByteArray uuid,
                                QString parName0, QVariant parValue0);
    void Insert(QString table,  QByteArray uuid,
                                QString parName0, QVariant parValue0,
                                QString parName1, QVariant parValue1);
    void Insert(QString table,  QByteArray uuid,
                                QString parName0, QVariant parValue0,
                                QString parName1, QVariant parValue1,
                                QString parName2, QVariant parValue2);
    // inserts with uuid and stateuuid
    /*
    void Insert(QString table,  QByteArray uuid, QString stateUuid);
    void Insert(QString table,  QByteArray uuid, QString stateUuid,
                                QString parName0, QVariant parValue0);
    void Insert(QString table,  QByteArray uuid, QString stateUuid,
                                QString parName0, QVariant parValue0,
                                QString parName1, QVariant parValue1);
    void Insert(QString table,  QByteArray uuid, QString stateUuid,
                                QString parName0, QVariant parValue0,
                                QString parName1, QVariant parValue1,
                                QString parName2, QVariant parValue2);*/
    // updates with uuid
    void Update(QString table,  QByteArray uuid,
                                QString parName0, QVariant parValue0);
    void Update(QString table,  QByteArray uuid,
                                QString parName0, QVariant parValue0,
                                QString parName1, QVariant parValue1);
    void Update(QString table,  QByteArray uuid,
                                QString parName0, QVariant parValue0,
                                QString parName1, QVariant parValue1,
                                QString parName2, QVariant parValue2);
    // updates with uuid and stateuuid
    /*
    void Update(QString table,  QByteArray uuid, QString stateUuid,
                                QString parName0, QVariant parValue0);
    void Update(QString table,  QByteArray uuid, QString stateUuid,
                                QString parName0, QVariant parValue0,
                                QString parName1, QVariant parValue1);
    void Update(QString table,  QByteArray uuid, QString stateUuid,
                                QString parName0, QVariant parValue0,
                                QString parName1, QVariant parValue1,
                                QString parName2, QVariant parValue2);*/
    // delete with uuid
    void Delete(QString table,  QByteArray uuid);
    // delete with uuid and stateuuid
    void Delete(QString table,  QByteArray uuid, QString stateUuid);
    // select single entry with uuid
    bool Select(QString table, QByteArray uuid,
                QString valName, QVariant *value);
    bool Select(QString table, QByteArray uuid,
                QString valName0, QVariant *value0,
                QString valName1, QVariant *value1);
    bool Select(QString table, QByteArray uuid,
                QString valName0, QVariant *value0,
                QString valName1, QVariant *value1,
                QString valName2, QVariant *value2);
    // select single entry with uuid and stateuuid
    /*
    bool Select(QString table, QByteArray uuid, QString stateuuid,
                QString valName, QVariant *value);
    bool Select(QString table, QByteArray uuid, QString stateuuid,
                QString valName0, QVariant *value0,
                QString valName1, QVariant *value1);
    bool Select(QString table, QByteArray uuid, QString stateuuid,
                QString valName0, QVariant *value0,
                QString valName1, QVariant *value1,
                QString valName2, QVariant *value2);*/
/*
    void Duplicate(QString table, QByteArray uuid, QString stateuuid,
                                               QString newuuid, QString newStateUuid);*/
    //static int GetNewLinkID();
};

class SingletonDestroyer
{
    public:
        SingletonDestroyer(DBConnector* = NULL);
        ~SingletonDestroyer();

        void SetSingleton(DBConnector* s);
    private:
        DBConnector* _singleton;
};

}


#endif
