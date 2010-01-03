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

#ifndef _TILEM_QT_H_
#define _TILEM_QT_H_

/*!
	\file tilemqt.h
	\brief Definition of the TilEmQt class
*/

#include <QMainWindow>

class QAction;

class CalcGrid;
class CalcDebuger;
class CalcGridManager;
class ConnectionManager;

class TilEmQt : public QMainWindow
{
	Q_OBJECT
	
	public:
		TilEmQt(QWidget *p = 0);
		~TilEmQt();
		
	public slots:
		void addCalc();
		void addCalc(const QString& rom);
		
	private:
		CalcGrid *m_calcGrid;
		CalcDebuger *m_calcDebuger;
		CalcGridManager *m_calcManager;
		ConnectionManager *m_connectionManager;
};

#endif
