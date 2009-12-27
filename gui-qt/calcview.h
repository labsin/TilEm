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

#ifndef _CALC_VIEW_H_
#define _CALC_VIEW_H_

/*!
	\file calcview.h
	\brief Definition of the CalcView class
*/

#include <QHash>
#include <QFrame>
#include <QString>
#include <QRegion>

class QMenu;
class QImage;

class Calc;
class CalcLink;
class CalcThread;

class CalcView : public QFrame
{
	Q_OBJECT
	
	public:
		CalcView(const QString& file, QWidget *p = 0);
		virtual ~CalcView();
		
		Calc* calc() const;
		CalcLink* link() const;
		
		bool isPaused() const;
		
	public slots:
		void pause();
		void resume();
		
		void load();
		void save();
		
		void load(const QString& f);
		void save(const QString& f);
		
		void quit();
		
		void selectSkin();
		
		void grabExternalLink();
		
	signals:
		void paused();
		void resumed();
		
		void paused(bool y);
		
		void externalLinkGrabbed(bool y);
		
	protected:
		virtual void keyPressEvent(QKeyEvent *e);
		virtual void keyReleaseEvent(QKeyEvent *e);
		
		virtual void mouseMoveEvent(QMouseEvent *e);
		virtual void mousePressEvent(QMouseEvent *e);
		virtual void mouseReleaseEvent(QMouseEvent *e);
		
		virtual void timerEvent(QTimerEvent *e);
		
		virtual void paintEvent(QPaintEvent *e);
		
		virtual void contextMenuEvent(QContextMenuEvent *e);
		
		virtual void dropEvent(QDropEvent *e);
		virtual void dragEnterEvent(QDragEnterEvent *e);
		virtual void dragMoveEvent(QDragMoveEvent *e);
		virtual void dragLeaveEvent(QDragLeaveEvent *e);
		
		virtual void focusInEvent(QFocusEvent *e);
		virtual void focusOutEvent(QFocusEvent *e);
		
		void setupSkin();
		void setupKeyboardLayout();
		
		void updateLCD();
		
		int mappedKey(int k) const;
		int mappedKey(const QPoint& pos) const;
		int closestKey(const QPoint& pos) const;
		
		int keyIndex(const QPoint& p) const;
		QPolygon keyBoundaries(const QPoint& p) const;
		//QRegion keyClip(const QPoint& p) const;
		
	private:
		QString m_model;
		
		QMenu *m_cxt;
		
		Calc *m_calc;
		CalcLink *m_link;
		CalcThread *m_thread;
		
		int m_lcdTimerId;
		
		QHash<int, int> m_kbdMap;
		
		int m_kThresold;
		QList<int> m_kScanCode;
		QList<QPoint> m_kCenter;
		//QList<QRegion> m_kClip;
		QList<QPolygon> m_kBoundaries;
		
		int m_lcdX, m_lcdY, m_lcdW, m_lcdH;
		
		int m_hovered;
		QList<int> m_pressed;
		
		QPixmap *m_skin;
		//QRegion m_keyregion;
		QImage *m_screen, *m_keymask;
}; 

#endif // _CALC_VIEW_H_
