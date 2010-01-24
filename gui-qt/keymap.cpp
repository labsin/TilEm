/****************************************************************************
**
** Copyright (C) 2010 Hugues Luc BRUANT aka fullmetalcoder 
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

#include "keymap.h"

/*!
	\file keymap.cpp
	\brief Implementation of the KeyMap class
*/

#include <QWidget>
#include <QKeyEvent>
#include <QTimerEvent>

/*!
	\class KeyMap
	\brief A simple keymap management class
	
	This class allows to create centralized mapping of arbitrary keysequences to
	an integer id (of quinptr type to ensure back-and-forth conversion to any
	pointer type if needed).
*/

KeyMap::KeyMap(QWidget *w)
 : QObject(w), m_widget(w), m_active(0), m_matchStep(0)
{
	w->installEventFilter(this);
}

KeyMap::KeyMap(QWidget *w, QObject *p)
 : QObject(p), m_widget(w), m_active(0), m_matchStep(0)
{
	w->installEventFilter(this);
}

KeyMap::~KeyMap()
{
	
}

int KeyMap::keyCount() const
{
	return m_map.count();
}

QList<quintptr> KeyMap::ids() const
{
	return m_map.keys();
}

QList<int> KeyMap::keys(quintptr id) const
{
	return m_map.value(id);
}

void KeyMap::setKeys(const QList<int>& keys, quintptr id)
{
	m_map[id] = keys;
	
	m_curMatch.removeAll(id);
}

static int key(const QKeyEvent *e)
{
	int k = e->key();
	int m = e->modifiers();
	
	if ( m & Qt::ControlModifier )
		k |= Qt::CTRL;
	if ( m & Qt::AltModifier )
		k |= Qt::ALT;
	if ( m & Qt::ShiftModifier )
		k |= Qt::SHIFT;
	if ( m & Qt::MetaModifier )
		k |= Qt::META;
	
	return k;
}

bool KeyMap::eventFilter(QObject *o, QEvent *e)
{
	if ( o == m_widget )
	{
		int type = e->type();
		QKeyEvent *ke = static_cast<QKeyEvent*>(e);
		
		if ( type == QEvent::KeyPress )
		{
			if ( ke->isAutoRepeat() )
			{
				e->accept();
				return true;
			}
			
			int k = key(ke);
			
			//qDebug("kp:%i", k);
			
			if ( m_active )
				deactivate(m_active);
			
			QList<quintptr> match;
			
			if ( m_matchStep )
			{
				QList<quintptr> rem;
				
				for ( int i = 0; i < m_curMatch.count(); ++i )
				{
					QList<int> seq = m_map.value(m_curMatch.at(i));
					
					if ( seq.at(m_matchStep) == k )
					{
						if ( seq.count() == m_matchStep + 1 )
							match << m_curMatch.at(i);
						else
							rem << m_curMatch.at(i);
					}
				}
				
				m_curMatch = rem;
			} else {
				QHash< quintptr, QList<int> >::const_iterator it, end;
				it = m_map.constBegin();
				end = m_map.constEnd();
				
				while ( it != end )
				{
					if ( it->first() == k )
					{
						if ( it->count() > 1 )
							m_curMatch << it.key();
						else
							match << it.key();
					}
					
					++it;
				}
			}
			
			if ( m_curMatch.count() )
				++m_matchStep;
			else if ( match.count() == 1 )
				activate(match.first());
			else
				m_matchStep = 0;
			
			e->accept();
			return true;
		} else if ( type == QEvent::KeyRelease ) {
			if ( ke->isAutoRepeat() )
			{
				e->accept();
				return true;
			}
			
			int k = key(ke);
			
			//qDebug("kr:%i", k);
			
			if ( m_active )
				deactivate(m_active);
			
			e->accept();
			return true;
		}
	}
	
	return QObject::eventFilter(o, e);
}

void KeyMap::activate(quintptr id)
{
	m_active = id;
	emit activated(m_active);
}

void KeyMap::deactivate(quintptr id)
{
	emit deactivated(m_active);
	m_active = 0;
}
