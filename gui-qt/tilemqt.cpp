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

#include <QAction>
#include <QToolBar>
#include <QFileDialog>

#include "calcgrid.h"

/*!
	\class TilEmQt
	\brief TilEmQt main window
	
	This class is responsible for top level GUI.
*/

TilEmQt::TilEmQt(QWidget *p)
 : QMainWindow(p)
{
	m_grid = new CalcGrid(this);
	
	QAction *a;
	QToolBar *tb = addToolBar(tr("Emulation"));
	
	a = tb->addAction(tr("Quit"));
	connect(a, SIGNAL( triggered() ), SLOT( close() ) );
	
	a = tb->addAction(tr("Pause"));
	m_grid->connect(a, SIGNAL( triggered() ), SLOT( pause() ) );
	
	a = tb->addAction(tr("Add calc"));
	connect(a, SIGNAL( triggered() ), SLOT( addCalc() ) );
	
	setCentralWidget(m_grid);
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
		m_grid->addCalc(rom);
}
