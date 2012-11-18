/**
 * @file
 * @author  Gregor Burger <burger.gregor@gmail.com>
 * @version 1.0
 * @section LICENSE
 *
 * This file is part of DynaMind
 *
 * Copyright (C) 2012  Gregor Burger
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef ADDLAYERDIALOG_H
#define ADDLAYERDIALOG_H
#include <dmcompilersettings.h>
#include <QDialog>

namespace Ui {
class AddLayerDialog;
}

class QTreeWidgetItem;

namespace DM {
    class System;
    class Layer;
    class View;
    class Viewer;
    class Attribute;



class DM_HELPER_DLL_EXPORT AddLayerDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit AddLayerDialog(DM::System *system, QWidget *parent = 0);
    ~AddLayerDialog();
    DM::Layer *getLayer(DM::Viewer *v);
    
    QStringList getAttributeVectorNames() const;
    bool isOverdrawLayer() const;
    
private slots:
    void on_viewList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_attributeList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    
private:
    DM::System *system;
    Ui::AddLayerDialog *ui;
    QColor start, stop;
    
    DM::View *view;
    std::string attribute;
    void getAttributesFromComponent(View &view, QMap<std::string, DM::Attribute *> & attributes, std::string leadingName="");
    std::string newAttributeName(std::string viewName, std::string name);
};

}

#endif // ADDLAYERDIALOG_H
