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

#include "calc.h"
#include "calcview.h"
#include "calclink.h"

#include <QAction>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

CalcGrid::CalcGrid(QWidget *p)
 : QScrollArea(p)
{
	setFocusPolicy(Qt::NoFocus);
	
	setWidgetResizable(true);
	QWidget *w = new QWidget;
	m_grid = new QHBoxLayout(w);
	setWidget(w);
	
	QAction *a;
	a = new QAction(tr("Float all"), this);
	connect(a, SIGNAL( triggered() ), this, SLOT( floatAllCalcs() ) );
	a->setShortcut(QKeySequence("F1"));
	addAction(a);
	a = new QAction(tr("Dock all"), this);
	connect(a, SIGNAL( triggered() ), this, SLOT( dockAllCalcs() ) );
	a->setShortcut(QKeySequence("F2"));
	addAction(a);
	
	setContextMenuPolicy(Qt::ActionsContextMenu);
}

CalcGrid::~CalcGrid()
{
	foreach ( CalcView *v, m_calcs )
		if ( !v->parent() )
			delete v;
}

void CalcGrid::dockCalc(CalcView *v)
{
	if ( !v->parent() )
	{
		m_grid->addWidget(v);
		v->docked();
		v->setFocus();
	}
}

void CalcGrid::floatCalc(CalcView *v)
{
	if ( v->parent() )
	{
		m_grid->removeWidget(v);
		v->setParent(0);
		v->undocked();
		v->show();
		v->setFocus();
	}
}

void CalcGrid::dockAllCalcs()
{
	foreach ( CalcView *v, m_calcs )
		dockCalc(v);
}

void CalcGrid::floatAllCalcs()
{
	foreach ( CalcView *v, m_calcs )
		floatCalc(v);
}

void CalcGrid::toggleDocking()
{
	CalcView *v = qobject_cast<CalcView*>(sender());
	
	if ( v )
	{
		if ( v->parent() )
			floatCalc(v);
		else
			dockCalc(v);
	}
}

void CalcGrid::pause()
{
	foreach ( CalcView *v, m_calcs )
		v->pause();
}

void CalcGrid::resume()
{
	foreach ( CalcView *v, m_calcs )
		v->resume();
}

int CalcGrid::calcCount() const
{
	return m_calcs.count();
}

CalcView* CalcGrid::calc(int idx) const
{
	return idx >= 0 && idx < m_calcs.count() ? m_calcs.at(idx) : 0;
}

int CalcGrid::addCalc(CalcView *c)
{
	if ( !c )
		return -1;
	
	m_calcs << c;
	
	m_grid->addWidget(c);
	
	c->setFocus();
	
	// latest created calc will always grab external link?
	c->link()->grabExternalLink();
	
	return m_calcs.count() - 1;
}

int CalcGrid::addCalc(const QString& romfile)
{
	CalcView *cv = new CalcView(romfile, this);
	cv->calc()->setName(tr("Calc %1").arg(m_calcs.count()));
	
	connect(cv, SIGNAL( toggleDocking() ), this, SLOT( toggleDocking() ));
	
	return addCalc(cv);
}

void CalcGrid::removeCalc(int idx, bool del)
{
	if ( idx < 0 && idx >= m_calcs.count() )
		return;
	
	CalcView *c = m_calcs.takeAt(idx);
	
	if ( c->parent() )
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
	
	if ( c->parent() )
		m_grid->removeWidget(c);
	else
		c->hide();
	
	if ( del )
		delete c;
}

void CalcGrid::paused()
{
	
}

void CalcGrid::resumed()
{
	
}

void CalcGrid::closeEvent(QCloseEvent *e)
{
	QScrollArea::closeEvent(e);
}

void CalcGrid::keyPressEvent(QKeyEvent *e)
{
	return QScrollArea::keyPressEvent(e);
}

void CalcGrid::contextMenuEvent(QContextMenuEvent *e)
{
	return QScrollArea::contextMenuEvent(e);
}

bool CalcGrid::focusNextPrevChild(bool next)
{
	int idx = (focusedCalc() + 1) % m_calcs.count();
	
	CalcView *c = m_calcs.at(idx);
	
	if ( c->parent() )
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
