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

#include "linkbuffer.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>

#include <tilem.h>
#include <z80.h>
}

#include <QHash>
#include <QObject>
#include <QMutex>
#include <QByteArray>
#include <QScriptValue>
#include <QReadWriteLock>

class QScriptEngine;

class Calc : public QObject
{
	friend class CalcLink;
	friend class CalcDebuger;
	
	Q_OBJECT
	
	public:
		struct Breakpoint
		{
			inline Breakpoint() : id(-1), mask(0xffff) {}
			
			int id;
			int type;
			quint16 start, end, mask;
			QScriptValue test;
		};
		
		Calc(QObject *p = 0);
		~Calc();
		
		QString name() const;
		
		QString romFile() const;
		
		int model() const;
		QString modelString() const;
		
		bool lcdUpdate();
		int lcdWidth() const;
		int lcdHeight() const;
		const unsigned int* lcdData() const;
		
		void resetLink();
		
		bool isBroadcasting() const;
		void setBroadcasting(bool y);
		
		bool isSending() const;
		bool isReceiving() const;
		
		uint32_t byteCount() const;
		
		char topByte();
		char getByte();
		void sendByte(char c);
		
		QByteArray getBytes(int n);
		int getBytes(int n, char *d);
		void sendBytes(const QByteArray& d);
		
		void addBreakpoint(Breakpoint *b);
		void removeBreakpoint(Breakpoint *b);
		
		static int breakpointDispatch(TilemCalc *c, dword a, void *d);
		
	public slots:
		void load(const QString& file);
		void save(const QString& file);
		
		int run_us(int usec);
		int run_cc(int clock);
		
		void reset();
		
		void stop(int reason = 0);
		
		void keyPress(int sk);
		void keyRelease(int sk);
		
		void setName(const QString& n);
		
	signals:
		void nameChanged(const QString& n);
		
		void bytesAvailable();
		
	private:
		QString m_romFile, m_name;
		
		QMutex m_run;
		TilemCalc *m_calc;
		
		unsigned char *m_lcd;
		unsigned int *m_lcd_comp;
		
		volatile bool m_link_lock, m_broadcast;
		
		LinkBuffer m_input, m_output;
		
		QScriptEngine *m_script;
		QList<Breakpoint*> m_breakpoints;
		
		static QHash<TilemCalc*, Calc*> m_table;
};

#endif
