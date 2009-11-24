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

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QGridLayout>

CalcGrid::CalcGrid(QWidget *p)
 : QScrollArea(p)
{
	setWidgetResizable(true);
	QWidget *w = new QWidget;
	m_grid = new QHBoxLayout(w);
	setWidget(w);
}

CalcGrid::~CalcGrid()
{
	
}

int CalcGrid::addCalc(CalcView *c)
{
	if ( !c )
		return -1;
	
	m_calcs << c;
	
	m_grid->addWidget(c);
	
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
	
	m_grid->removeWidget(c);
	
	if ( del )
		delete c;
}

void CalcGrid::removeCalc(CalcView *c, bool del)
{
	if ( !c )
		return;
	
	m_grid->removeWidget(c);
	
	if ( del )
		delete c;
}

void CalcGrid::keyPressEvent(QKeyEvent *e)
{
	return QScrollArea::keyPressEvent(e);
}

bool CalcGrid::focusNextPrevChild(bool next)
{
	int idx = (focusedCalc() + 1) % m_calcs.count();
	
	CalcView *c = m_calcs.at(idx);
	ensureWidgetVisible(c);
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
