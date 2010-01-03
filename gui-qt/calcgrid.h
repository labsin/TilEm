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

#ifndef _CALC_GRID_H_
#define _CALC_GRID_H_

/*!
	\file calcgrid.h
	\brief Definition of the CalcGrid class
*/

#include <QScrollArea>

class QLayout;

class CalcView;

class CalcGrid : public QScrollArea
{
	Q_OBJECT
	
	public:
		enum DisplayMode
		{
			MergedWindows,
			FloatingWindows
		};
		
		CalcGrid(QWidget *p = 0);
		~CalcGrid();
		
		int calcCount() const;
		CalcView* calc(int idx) const;
		
	public slots:
		void pause();
		void resume();
		
		int addCalc(CalcView *c);
		int addCalc(const QString& romfile);
		
		void removeCalc(int idx, bool del = true);
		void removeCalc(CalcView *c, bool del = true);
		
		void dockCalc(CalcView *v);
		void floatCalc(CalcView *v);
		
		void dockAllCalcs();
		void floatAllCalcs();
		
	protected:
		void contextMenuEvent(QContextMenuEvent *e);
		
	protected slots:
		void paused();
		void resumed();
		
		void toggleDocking();
		
	protected:
		virtual void closeEvent(QCloseEvent *e);
		virtual void keyPressEvent(QKeyEvent *e);
		
		virtual bool focusNextPrevChild(bool next);
		
		int focusedCalc() const;
		
	private:
		QLayout *m_grid;
		QList<CalcView*> m_calcs;
};

#endif
