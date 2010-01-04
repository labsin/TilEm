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

#include "calcdebuger.h"

/*!
	\file calcdebuger.cpp
	\brief Implementation of the CalcDebuger class
*/

#include "calc.h"

/*!
	\class CalcDebuger
	\brief A widget offering debuging capabilities for an emulated calc
*/

CalcDebuger::CalcDebuger(QWidget *p)
 : QWidget(p)
{
	setupUi(this);
}

CalcDebuger::~CalcDebuger()
{
	
}

QSize CalcDebuger::sizeHint() const
{
	return QSize(300, 800);
}

void CalcDebuger::setCalc(Calc *c)
{
	if ( m_calc )
	{
		
	}
	
	m_calc = c;
	
	if ( m_calc )
	{
		
	}
}