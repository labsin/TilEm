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

#include "calcgrid.h"

/*!
	\file calcgrid.cpp
	\brief Implementation of the CalcGrid class
*/

#include "calcview.h"
#include "connectionmanager.h"

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

CalcGrid::CalcGrid(QWidget *p)
 : QScrollArea(p), m_mode(MergedWindows)
{
	setWidgetResizable(true);
	QWidget *w = new QWidget;
	m_grid = new QHBoxLayout(w);
	setWidget(w);
	
	m_manager = new ConnectionManager(this);
}

CalcGrid::~CalcGrid()
{
	foreach ( CalcView *v, m_calcs )
		if ( !v->parent() )
			delete v;
}

CalcGrid::DisplayMode CalcGrid::displayMode() const
{
	return m_mode;
}

void CalcGrid::setDisplayMode(CalcGrid::DisplayMode m)
{
	if ( m_mode == m )
		return;
	
	if ( m_mode == MergedWindows )
	{
		// unmerge...
		foreach ( CalcView *v, m_calcs )
		{
			m_grid->addWidget(v);
			v->setParent(0);
		}
	}
	
	m_mode = m;
	
	if ( m_mode == MergedWindows )
	{
		// merge...
		foreach ( CalcView *v, m_calcs )
		{
			m_grid->addWidget(v);
		}
	}
}

int CalcGrid::addCalc(CalcView *c)
{
	if ( !c )
		return -1;
	
	// autoconnect calcs by two for testing purpose
	if ( m_calcs.count() & 1 )
		m_manager->addConnection(m_calcs.last()->calc(), c->calc());
	
	m_calcs << c;
	
	if ( m_mode == MergedWindows )
		m_grid->addWidget(c);
	else
		c->setParent(0);
	
	c->setFocus();
	
	return m_calcs.count() - 1;
}

int CalcGrid::addCalc(const QString& romfile)
{
	return addCalc(new CalcView(romfile, this));
}

void CalcGrid::removeCalc(int idx, bool del)
{
	if ( idx < 0 && idx >= m_calcs.count() )
		return;
	
	CalcView *c = m_calcs.takeAt(idx);
	
	if ( m_mode == MergedWindows )
		m_grid->removeWidget(c);
	else
		c->hide();
	
	if ( del )
		delete c;
}

void CalcGrid::removeCalc(CalcView *c, bool del)
{
	if ( !c )
		return;
	
	m_calcs.removeAll(c);
	
	if ( m_mode == MergedWindows )
		m_grid->removeWidget(c);
	else
		c->hide();
	
	if ( del )
		delete c;
}

void CalcGrid::closeEvent(QCloseEvent *e)
{
	if ( m_mode == FloatingWindows )
	{
		int ret = QMessageBox::warning(
									this,
									tr("Closing"),
									tr("Closing this window will close all floating calculator windows.\nDo you want to continue?"),
									QMessageBox::Yes | QMessageBox::No
								);
		
		if ( ret == QMessageBox::Yes )
		{
			e->accept();
		} else {
			e->ignore();
		}
		
		return;
	}
	
	QWidget::closeEvent(e);
}

void CalcGrid::keyPressEvent(QKeyEvent *e)
{
	if ( e->key() == Qt::Key_F1 )
		setDisplayMode(MergedWindows);
	else if ( e->key() == Qt::Key_F2 )
		setDisplayMode(FloatingWindows);
	
	return QScrollArea::keyPressEvent(e);
}

bool CalcGrid::focusNextPrevChild(bool next)
{
	int idx = (focusedCalc() + 1) % m_calcs.count();
	
	CalcView *c = m_calcs.at(idx);
	
	if ( m_mode == MergedWindows )
		ensureWidgetVisible(c);
	else
		c->raise();
	
	c->setFocus();
	return true;
}

int CalcGrid::focusedCalc() const
{
	for ( int i = 0; i < m_calcs.count(); ++i )
		if ( m_calcs.at(i)->hasFocus() )
			return i;
	
	return -1;
}
