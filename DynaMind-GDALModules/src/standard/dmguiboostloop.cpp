/**
* @file
* @author  Chrisitan Urich <christian.urich@gmail.com>
* @version 1.0
* @section LICENSE
*
* This file is part of DynaMind
*
* Copyright (C) 2011  Christian Urich

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

#include "dmguiboostloop.h"
#include "ui_dmguiboostloop.h"
#include "dmboostloop.h"
#include <QInputDialog>
#include <QMessageBox>

DMGUIBoostLoop::DMGUIBoostLoop(DMBoostLoop * m, QWidget *parent):
	QDialog(parent),
    ui(new Ui::DMGUIBoostLoop)
{
	ui->setupUi(this);

	this->m = m;
	
	ui->repeats->setValue(m->runs);
    ui->viewnames->setText(QString::fromStdString(m->viewnames));
    ui->attributenames->setText(QString::fromStdString(m->attributenames));
    ui->filternames->setText(QString::fromStdString(m->filternames));
	ui->repeats->setMinimum(1);

	foreach (std::string s, m->readStreams)
		insertStreamEntry(s, false);

	foreach (std::string s, m->writeStreams)
		insertStreamEntry(s, true);
}

DMGUIBoostLoop::~DMGUIBoostLoop()
{
	delete ui;
}

void DMGUIBoostLoop::insertStreamEntry(std::string name, bool write)
{
	if(write)
		ui->streamList->addItem(QString::fromStdString(name) + "\twrite");
	else
		ui->streamList->addItem(QString::fromStdString(name) + "\tread");
}

void DMGUIBoostLoop::on_addReadStream_clicked()
{
	QString name = QInputDialog::getText(this, "set name", "specify name of read stream");
	if(!name.isEmpty())
	{
		if(m->addStream(name.toStdString(), false))
			insertStreamEntry(name.toStdString(), false);
		else
		{
			QMessageBox box;
			box.setText("Name already taken");
			box.exec();
		}	
	}
}
void DMGUIBoostLoop::on_addWriteStream_clicked()
{
	QString name = QInputDialog::getText(this, "set name", "specify name of write stream");
	if(!name.isEmpty())
	{
		if(m->addStream(name.toStdString(), true))
			insertStreamEntry(name.toStdString(), true);
		else	
		{
			QMessageBox box;
			box.setText("Name already taken");
			box.exec();
		}	
	}
}
void DMGUIBoostLoop::on_removeStream_clicked()
{
	QListWidgetItem* item = ui->streamList->currentItem();
	if(!item)
		return;
	
	QString qname = item->text();
	if(qname.endsWith("\tread"))
		qname.chop(5);
	else if(qname.endsWith("\twrite"))
		qname.chop(6);

	std::string name = qname.toStdString(); 

	if(m->removeStream(name))
		delete ui->streamList->currentItem();
	else
	{
		QMessageBox box;
		box.setText("stream '" + item->text() + "' not found");
		box.exec();
	}
}

void DMGUIBoostLoop::accept()
{
	m->runs = ui->repeats->value();
    m->viewnames = ui->viewnames->text().toStdString();
    m->attributenames = ui->attributenames->text().toStdString();
    m->filternames = ui->filternames->text().toStdString();
	QDialog::accept();
}
