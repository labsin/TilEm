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

#ifndef _CALC_H_
#define _CALC_H_

/*!
	\file calc.h
	\brief Definition of the Calc class
*/

extern "C" {
#include <stdio.h>
#include <stdlib.h>

#include <tilem.h>
}

#include <QObject>
#include <QMutex>

class Calc : public QObject
{
	friend class CalcLink;
	
	Q_OBJECT
	
	public:
		Calc(QObject *p = 0);
		~Calc();
		
		QString romFile() const;
		
		int model() const;
		QString modelString() const;
		
		bool lcdUpdate();
		int lcdWidth() const;
		int lcdHeight() const;
		const unsigned int* lcdData() const;
		
	public slots:
		void load(const QString& file);
		void save(const QString& file);
		
		int run_us(int usec);
		int run_cc(int clock);
		
		void stop(int reason = 0);
		
		void keyPress(int sk);
		void keyRelease(int sk);
		
	private:
		QMutex m_run;
		
		QString m_romFile;
		
		TilemCalc *m_calc;
		
		unsigned char *m_lcd;
		unsigned int *m_lcd_comp;
};

#endif
