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

#ifndef _LINK_BUFFER_H_
#define _LINK_BUFFER_H_

/*!
	\file linkbuffer.h
	\brief Definition of the LinkBuffer class
*/

extern "C" {
#include <stdint.h>
}

#include <QByteArray>

class LinkBuffer
{
	public:
		LinkBuffer()
		{
			m_base = m_count = 0;
		}
		
		~LinkBuffer()
		{
			
		}
		
		bool isEmpty() const
		{
			return !m_count;
		}
		
		uint32_t count() const
		{
			return m_count;
		}
		
		void append(char c)
		{
			if ( m_count == 1024 )
			{
				// keep overflowing data
				m_overflow += c;
			} else {
				m_d[(m_base + m_count) & 1023] = c;
			}
			
			++m_count;
		}
		
		QByteArray take(uint32_t count)
		{
			QByteArray b;
			b.resize(count);
			
			for ( uint32_t i = 0; i < count; ++i )
				b[i] = at(i);
			
			remove(count);
			
			return b;
		}
		
		void remove(uint32_t count)
		{
			m_base = (m_base + count) & 1023;
			m_count = qMax(uint32_t(0), m_count - count);
			
			const uint32_t m = qMin(count, uint32_t(m_overflow.count()));
			
			if ( m )
			{
				// bring back overflow into data
				
				for ( uint32_t i = 0; i < m; ++i )
					append(m_overflow.at(i));
				
				m_overflow.remove(0, m);
			}
		}
		
		char at(uint32_t idx) const
		{
			return idx < 1024 ? m_d[(m_base + idx) & 1023] : m_overflow.at(idx - 1024);
		}
		
		LinkBuffer& operator += (const QByteArray& ba)
		{
			const int c = ba.count();
			for ( int i = 0; i < c; ++i )
				append(ba.at(i));
			
			return *this;
		}
		
	private:
		uint32_t m_base, m_count;
		char m_d[1024];
		QByteArray m_overflow;
};

#endif
