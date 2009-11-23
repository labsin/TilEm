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

#ifndef _CALC_LINK_H_
#define _CALC_LINK_H_

/*!
	\file calclink.h
	\brief Definition of the CalcLink class
*/

extern "C" {
#ifdef _TILEM_QT_HAS_LINK_
#include <ticalcs.h>
#endif
}

#include <QObject>

class Calc;

class CalcLink : public QObject
{
	Q_OBJECT
	
	public:
		CalcLink(Calc *c, QObject *p = 0);
		~CalcLink();
		
		bool isSupportedFile(const QString& file) const;
		
	public slots:
		void setCalc(Calc *c);
		
		void send(const QString& file);
		
	private:
		Calc *m_calc;
		
		#ifdef _TILEM_QT_HAS_LINK_
		CalcHandle *m_ch;
		CableHandle *m_cbl;
		#endif
};

#endif
