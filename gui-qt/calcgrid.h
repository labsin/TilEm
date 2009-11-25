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
class ConnectionManager;

class CalcGrid : public QScrollArea
{
	public:
		enum DisplayMode
		{
			MergedWindows,
			FloatingWindows
		};
		
		CalcGrid(QWidget *p = 0);
		~CalcGrid();
		
		DisplayMode displayMode() const;
		void setDisplayMode(DisplayMode m);
		
	public slots:
		int addCalc(CalcView *c);
		int addCalc(const QString& romfile);
		
		void removeCalc(int idx, bool del = true);
		void removeCalc(CalcView *c, bool del = true);
		
	protected:
		virtual void closeEvent(QCloseEvent *e);
		virtual void keyPressEvent(QKeyEvent *e);
		
		virtual bool focusNextPrevChild(bool next);
		
		int focusedCalc() const;
		
	private:
		QLayout *m_grid;
		DisplayMode m_mode;
		
		QList<CalcView*> m_calcs;
		
		ConnectionManager *m_manager;
};

#endif
