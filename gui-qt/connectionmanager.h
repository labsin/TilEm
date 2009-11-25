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

#ifndef _CONNECTION_MANAGER_H_
#define _CONNECTION_MANAGER_H_

/*!
	\file connectionmanager.h
	\brief Definition of the ConnectionManager class
*/

#include <QObject>

#include <QHash>

class Calc;

class ConnectionManager : public QObject
{
	Q_OBJECT
	
	public:
		ConnectionManager(QObject *p = 0);
		~ConnectionManager();
		
		void addConnection(Calc *c1, Calc *c2);
		void removeConnection(Calc *c);
		
	private slots:
		void bytesAvailable();
		
	private:
		QHash<Calc*, Calc*> m_connections;
};

#endif
