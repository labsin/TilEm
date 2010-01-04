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

#ifndef _CALC_DEBUGER_H_
#define _CALC_DEBUGER_H_

/*!
	\file calcdebuger.h
	\brief Definition of the CalcDebuger class
*/

#include <QWidget>
#include "ui_calcdebuger.h"

#include <QPointer>

class Calc;
class CalcGrid;

class CalcDebuger : public QWidget, private Ui::CalcDebuger
{
	Q_OBJECT
	
	public:
		CalcDebuger(CalcGrid *g, QWidget *p = 0);
		~CalcDebuger();
		
		virtual QSize sizeHint() const;
		
	private slots:
		void on_cbTarget_currentIndexChanged(int idx);
		
	private:
		QPointer<Calc> m_calc;
		CalcGrid *m_calcGrid;
};

#endif
