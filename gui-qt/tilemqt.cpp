/****************************************************************************
**
** Copyright (C) 2009 Hugues Luc BRUANT aka fullmetalcoder 
**                    <non.deterministic.finite.organism@gmail.com>
** 
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and appearing in the
** file GPL.txt included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "tilemqt.h"

/*!
	\file tilemqt.cpp
	\brief Implementation of the TilEmQt class
*/

#include "calcgrid.h"
#include "calcdebuger.h"
#include "calcgridmanager.h"
#include "connectionmanager.h"

#include <QAction>
#include <QToolBar>
#include <QDockWidget>
#include <QFileDialog>

/*!
	\class TilEmQt
	\brief TilEmQt main window
	
	This class is responsible for top level GUI.
*/

TilEmQt::TilEmQt(QWidget *p)
 : QMainWindow(p)
{
	m_calcGrid = new CalcGrid(this);
	m_calcDebuger = new CalcDebuger(this);
	m_calcManager = new CalcGridManager(m_calcGrid);
	m_connectionManager = new ConnectionManager(this);
	
	QAction *a;
	QToolBar *tb = addToolBar(tr("Emulation"));
	
	a = tb->addAction(tr("Quit"));
	connect(a, SIGNAL( triggered() ), SLOT( close() ) );
	
	a = tb->addAction(tr("Pause"));
	m_calcGrid->connect(a, SIGNAL( triggered() ), SLOT( pause() ) );
	
	a = tb->addAction(tr("Add calc"));
	connect(a, SIGNAL( triggered() ), SLOT( addCalc() ) );
	
	setCentralWidget(m_calcGrid);
	
	QDockWidget *mgr = new QDockWidget(this);
	mgr->setWindowTitle(tr("Manager"));
	mgr->setWidget(m_calcManager);
	addDockWidget(Qt::LeftDockWidgetArea, mgr);
	
	QDockWidget *dbg = new QDockWidget(this);
	dbg->setWindowTitle(tr("Debuger"));
	dbg->setWidget(m_calcDebuger);
	addDockWidget(Qt::RightDockWidgetArea, dbg);
}

TilEmQt::~TilEmQt()
{
	
}

void TilEmQt::addCalc()
{
	QString rom = QFileDialog::getOpenFileName(this, tr("Choose a ROM file"));
	addCalc(rom);
}

void TilEmQt::addCalc(const QString& rom)
{
	if ( QFile::exists(rom) )
		m_calcGrid->addCalc(rom);
}
