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

#ifndef GUIATTRIBUTECALCULATOR_H
#define GUIATTRIBUTECALCULATOR_H

#include <QDialog>
#include <attributecalculator.h>
namespace Ui {
class GUIAttributeCalculator;
}
class QTreeWidgetItem;
class GUIAttributeCalculator : public QDialog
{
	Q_OBJECT

public:
	explicit GUIAttributeCalculator(DM::Module * m, QWidget *parent = 0);
	~GUIAttributeCalculator();

private:
	Ui::GUIAttributeCalculator *ui;
	AttributeCalculator * attrcalc;
	void updateAttributeView();
	QString viewName;


	void createTreeViewEntries(QTreeWidgetItem * root_port, std::string viewname);


protected slots:
	void on_addButton_clicked();
	void on_comboView_currentIndexChanged (int);
public slots:
	void accept();
};

#endif // GUIATTRIBUTECALCULATOR_H
