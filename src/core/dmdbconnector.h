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

class QSqlQuery;
class QSqlError;
class QSqlDatabase;

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

template<class Tkey,class Tvalue>
class Cache
{
private:
    class Node
    {
    public:
        Tkey key;
        Tvalue value;
        Node* next;
        Node* last;
        Node(Tkey k, Tvalue v)
        {
            key=k;
            value=v;
            next = NULL;
            last = NULL;
        }
    };

    Node*   _root;
    Node*   _last;
    unsigned int    _size;
    unsigned int    _cnt;

    void push_front(Node *n)
    {
        if(_last==NULL)
            _last = n;

        n->next = _root;
        n->last = NULL;
        if(_root != NULL)
            _root->last = n;
        _root = n;
        _cnt++;
    }
    Node* pop(Node* n)
    {
        Node* last = n->last;
        Node* next = n->next;
        if(last)
            last->next = next;
        if(next)
            next->last = last;
        if(n==_last)
            _last = last;
        if(n==_root)
            _root = next;
        _cnt--;
        return n;
    }
    Node* search(Tkey key)
    {
        Node *n = _root;
        while(n!=NULL && n->key != key)
            n = n->next;
        return n;
    }

public:
    Cache(unsigned int size)
    {
        _size=size;
        _cnt=0;
        _root = NULL;
        _last = NULL;
    }
    ~Cache()
    {
        Node* cur;
        Node* next;
        next = _root;
        while(next!=NULL)
        {
            cur=next;
            next=cur->next;
            delete cur;
        }
    }

    Tvalue get(Tkey key)
    {
        Node *n = search(key);
        // push front
        if(n!=NULL)
        {
            pop(n);
            push_front(n);
        }
        else
            return Tvalue();
        return n->value;
    }
    void add(Tkey key,Tvalue value)
    {
        Node *n = new Node(key,value);
        push_front(n);

        if(_cnt>_size)
            delete pop(_last);
    }
    void change(Tkey key,Tvalue value)
    {
        Node *n = search(key);
        if(n!=NULL)
            n->value = value;
    }
};

}   // namespace DM


#endif
