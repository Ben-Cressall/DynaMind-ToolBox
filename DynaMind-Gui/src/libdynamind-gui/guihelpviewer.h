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

#ifndef GUIHELPVIEWER_H
#define GUIHELPVIEWER_H
#include <dmcompilersettings.h>
#include <guisimulation.h>
#include <QWidget>
#include <QUrl>

namespace Ui {
class GUIHelpViewer;
}

class DM_HELPER_DLL_EXPORT GUIHelpViewer : public QWidget
{
    Q_OBJECT

public:
	explicit GUIHelpViewer(QWidget *parent = 0);
	~GUIHelpViewer();

    void getBaseUrl(QSettings settings);
private:
	Ui::GUIHelpViewer *ui;
	QUrl currentUrl;
	QUrl url_view_not_avaiable;
	QString getBaseUrl();
public slots:
	void showHelpForModule(DM::Module* m);
	void on_commandBackToOvwerView_clicked();
};

#endif // GUIHELPVIEWER_H
