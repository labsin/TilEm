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

#include "calcview.h"

/*!
	\file calcview.cpp
	\brief Implementation of the CalcView class
*/

#include <QDir>
#include <QUrl>
#include <QMenu>
#include <QDebug>
#include <QImage>
#include <QThread>
#include <QBitmap>
#include <QPainter>
#include <QFileInfo>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QFileDialog>
#include <QApplication>

extern "C" {
#include <scancodes.h>
}

#include "calc.h"
#include "calclink.h"
#include "settings.h"

/*!
	\class CalcThread
	\brief Utility class to manage calc emulation
	
	Perform the emulation in a separate thread to reduce latency in both GUI and emulator.
*/
class CalcThread : public QThread
{
	public:
		CalcThread(Calc *c, QObject * p = 0)
		: QThread(p), m_calc(c), exiting(0)
		{
			
		}
		
		void stop()
		{
			// stop emulator thread before loading a new ROM
			exiting = 1;
			
			// wait for thread to terminate
			while ( isRunning() )
				usleep(1000);
			
		}
		
	protected:
		virtual void run()
		{
			forever
			{
				if ( m_calc->run_us(10000) )
					break;
				
// 				printf("  PC=%02X:%04X AF=%04X BC=%04X DE=%04X HL=%04X IX=%04X IY=%04X SP=%04X\n",
// 						m_calc->mempagemap[m_calc->z80.r.pc.b.h >> 6],
// 						m_calc->z80.r.pc.w.l,
// 						m_calc->z80.r.af.w.l,
// 						m_calc->z80.r.bc.w.l,
// 						m_calc->z80.r.de.w.l,
// 						m_calc->z80.r.hl.w.l,
// 						m_calc->z80.r.ix.w.l,
// 						m_calc->z80.r.iy.w.l,
// 						m_calc->z80.r.sp.w.l
// 					);
// 				
				
				// slightly slow down emulation (TODO : make delay adjustable)
				usleep(10000);
				
				if ( exiting )
					break;
			}
			
			exiting = 0;
		}
		
	private:
		Calc *m_calc;
		volatile int exiting;
};

/*!
	\class CalcView
	\brief Main UI class for emulation
	
	Displays calculator skin and forwards user interaction (mouse/keyboard) to emulator core.
	
	Accepts drops to send files to emulated calc.
*/

CalcView::CalcView(const QString& file, QWidget *p)
: QFrame(p), m_link(0), m_thread(0), m_skin(0), m_screen(0), m_keymask(0)
{
	setFrameShape(QFrame::StyledPanel);
	
	// accept drops for easy file download
	setAcceptDrops(true);
	
	// setup core objects
	m_calc = new Calc(this);
	
	// load ROM
	if ( QFile::exists(file) )
		load(file);
	else
		load();
	
	QAction *a;
	// setup context menu
	m_cxt = new QMenu(this);
	
	m_cxt->addAction("Load ROM/state...", this, SLOT( load() ));
	m_cxt->addAction("Save ROM/state...", this, SLOT( save() ));
	m_cxt->addSeparator();
	a = m_cxt->addAction("Pause", this, SLOT( pause() ) );
	a->connect(this, SIGNAL( paused(bool) ), SLOT( setDisabled(bool) ) );
	a = m_cxt->addAction("Resume", this, SLOT( resume() ) );
	a->setEnabled(false);
	a->connect(this, SIGNAL( paused(bool) ), SLOT( setEnabled(bool) ) );
	m_cxt->addSeparator();
	m_cxt->addAction("Change skin...", this, SLOT( selectSkin() ));
	m_cxt->addSeparator();
	m_cxt->addAction("Close", this, SLOT( close() ));
	
	// launch emulator thread
	resume();
	
	// start LCD update timer
	m_lcdTimerId = startTimer(10);
}

CalcView::~CalcView()
{
	m_thread->stop();
	
	// cleanup
	delete m_screen;
	delete m_keymask;
	delete m_skin;
}

bool CalcView::isPaused() const
{
	return m_thread->isRunning();
}

void CalcView::pause()
{
	if ( m_thread->isRunning() )
	{
		m_thread->stop();
		emit paused();
		emit paused(true);
	}
}

void CalcView::resume()
{
	if ( !m_thread->isRunning() )
	{
		m_thread->start();
		emit resumed();
		emit paused(false);
	}
}

Calc* CalcView::calc() const
{
	return m_calc;
}

CalcLink* CalcView::link() const
{
	return m_link;
}

void CalcView::load()
{
	QString romfile = QFileDialog::getOpenFileName(
										this,
										tr("Select ROM file"),
										QString(),
										tr("ROM files (*.rom);;All files (*)")
									);
	
	if ( QFile::exists(romfile)	)
		load(romfile);
	else if ( m_calc->romFile().isEmpty() )
		load();
}

void CalcView::save()
{
	save(m_calc->romFile());
}

void CalcView::load(const QString& file)
{
	/// 1) stop/cleanup phase
	startTimer(m_lcdTimerId);
	
	if ( m_thread )
		m_thread->stop();
	
	/// 2) load phase
	m_calc->load(file);
	
	if ( !m_link )
		m_link = new CalcLink(m_calc, this);
	else
		m_link->setCalc(m_calc);
	
	if ( !m_thread )
		m_thread = new CalcThread(m_calc, this);
	
	m_model = m_calc->modelString();
	
	// setup skin and keyboard layout
	setupSkin();
	setupKeyboardLayout();
	
	/// 3) restart phase
	
	// launch emulator thread
	m_thread->start();
	
	// start LCD update timer
	m_lcdTimerId = startTimer(10);
	
	setWindowTitle(file);
}

void CalcView::save(const QString& file)
{
	m_calc->save(file);
}

void CalcView::quit()
{
	QApplication::quit();
}

void CalcView::selectSkin()
{
	
}

void CalcView::setupSkin()
{
	// cleanup previous images
	delete m_keymask;
	delete m_skin;
	delete m_screen;
	
	QDir d("skins");
	Settings s;
	s.load(d.filePath(m_model + ".skin"));
	
	Settings::Entry *e = s.entry("lcd-coords");
	
	QList<int> l;
	
	if ( e )
		l = e->integerItems();
	
	if ( l.count() == 4 )
	{
		m_lcdX = l.at(0);
		m_lcdY = l.at(1);
		m_lcdW = l.at(2);
		m_lcdH = l.at(3);
	} else {
		qWarning("skin: Missing or malformed lcd-coords field.");
	}
	
	m_skin = new QPixmap(d.filePath(s.value("skin")));
	m_screen = new QImage(m_lcdW, m_lcdH, QImage::Format_RGB32);
	m_keymask = new QImage(QImage(d.filePath(s.value("keymask"))).createHeuristicMask());
	
	setFixedSize(m_skin->size());
	
	//qDebug() << s.value("skin") << m_skin->size();
	
	e = s.entry("key-coords");
	
	if ( e )
	{
		for ( int i = 0; i < e->children.count(); ++i )
		{
			Settings::Entry *m = e->children.at(i);
			QList<int> coords = m->integerItems();
			
			if ( coords.count() == 3 )
			{
				m_kCenter << QPoint(coords.at(0), coords.at(1));
				m_kScanCode << coords.at(2);
			} else {
				qWarning("skin: Malformed key-coords entry.");
			}
		}
	} else {
		qWarning("skin: Missing key-coords field.");
	}
}

void CalcView::setupKeyboardLayout()
{
	// keyboard -> keypad mapping
	// TODO : move to external file (slightly kbd layout-dependent)
	
	m_kbdMap[Qt::Key_Enter] = TILEM_KEY_ENTER;
	m_kbdMap[Qt::Key_Return] = TILEM_KEY_ENTER;
	
	m_kbdMap[Qt::Key_Left] = TILEM_KEY_LEFT;
	m_kbdMap[Qt::Key_Right] = TILEM_KEY_RIGHT;
	m_kbdMap[Qt::Key_Up] = TILEM_KEY_UP;
	m_kbdMap[Qt::Key_Down] = TILEM_KEY_DOWN;
	
	m_kbdMap[Qt::Key_F1] = TILEM_KEY_YEQU;
	m_kbdMap[Qt::Key_F2] = TILEM_KEY_WINDOW;
	m_kbdMap[Qt::Key_F3] = TILEM_KEY_ZOOM;
	m_kbdMap[Qt::Key_F4] = TILEM_KEY_TRACE;
	m_kbdMap[Qt::Key_F5] = TILEM_KEY_GRAPH;
	
	m_kbdMap[Qt::Key_0] = TILEM_KEY_0;
	m_kbdMap[Qt::Key_1] = TILEM_KEY_1;
	m_kbdMap[Qt::Key_2] = TILEM_KEY_2;
	m_kbdMap[Qt::Key_3] = TILEM_KEY_3;
	m_kbdMap[Qt::Key_4] = TILEM_KEY_4;
	m_kbdMap[Qt::Key_5] = TILEM_KEY_5;
	m_kbdMap[Qt::Key_6] = TILEM_KEY_6;
	m_kbdMap[Qt::Key_7] = TILEM_KEY_7;
	m_kbdMap[Qt::Key_8] = TILEM_KEY_8;
	m_kbdMap[Qt::Key_9] = TILEM_KEY_9;
	
	m_kbdMap[Qt::Key_Plus] = TILEM_KEY_ADD;
	m_kbdMap[Qt::Key_Minus] = TILEM_KEY_SUB;
	m_kbdMap[Qt::Key_Asterisk] = TILEM_KEY_MUL;
	m_kbdMap[Qt::Key_Slash] = TILEM_KEY_DIV;
	
	m_kbdMap[Qt::Key_Backspace] = TILEM_KEY_DEL;
	m_kbdMap[Qt::Key_Delete] = TILEM_KEY_CLEAR;
	
	m_kbdMap[Qt::Key_Alt] = TILEM_KEY_ON;
	m_kbdMap[Qt::Key_Control] = TILEM_KEY_2ND;
	m_kbdMap[Qt::Key_Shift] = TILEM_KEY_ALPHA;
	m_kbdMap[Qt::Key_Escape] = TILEM_KEY_MODE;
	
	
	m_kbdMap[Qt::Key_A] = TILEM_KEY_MATH;
	m_kbdMap[Qt::Key_B] = TILEM_KEY_MATRIX; //TILEM_KEY_APPS;
	m_kbdMap[Qt::Key_C] = TILEM_KEY_PRGM;
	m_kbdMap[Qt::Key_D] = TILEM_KEY_RECIP;
	m_kbdMap[Qt::Key_E] = TILEM_KEY_SIN;
	m_kbdMap[Qt::Key_F] = TILEM_KEY_COS;
	m_kbdMap[Qt::Key_G] = TILEM_KEY_TAN;
	m_kbdMap[Qt::Key_H] = TILEM_KEY_POWER;
	m_kbdMap[Qt::Key_I] = TILEM_KEY_SQUARE;
	m_kbdMap[Qt::Key_J] = TILEM_KEY_COMMA;
	m_kbdMap[Qt::Key_K] = TILEM_KEY_LPAREN;
	m_kbdMap[Qt::Key_L] = TILEM_KEY_RPAREN;
	m_kbdMap[Qt::Key_M] = TILEM_KEY_DIV;
	m_kbdMap[Qt::Key_N] = TILEM_KEY_LOG;
	m_kbdMap[Qt::Key_O] = TILEM_KEY_7;
	m_kbdMap[Qt::Key_P] = TILEM_KEY_8;
	m_kbdMap[Qt::Key_Q] = TILEM_KEY_9;
	m_kbdMap[Qt::Key_R] = TILEM_KEY_MUL;
	m_kbdMap[Qt::Key_S] = TILEM_KEY_LN;
	m_kbdMap[Qt::Key_T] = TILEM_KEY_4;
	m_kbdMap[Qt::Key_U] = TILEM_KEY_5;
	m_kbdMap[Qt::Key_V] = TILEM_KEY_6;
	m_kbdMap[Qt::Key_W] = TILEM_KEY_SUB;
	m_kbdMap[Qt::Key_X] = TILEM_KEY_STORE;
	m_kbdMap[Qt::Key_Y] = TILEM_KEY_1;
	m_kbdMap[Qt::Key_Z] = TILEM_KEY_2;
	
	m_kbdMap[Qt::Key_Space] = TILEM_KEY_0;
	
	m_kbdMap[Qt::Key_Period] = TILEM_KEY_DECPNT;
	m_kbdMap[Qt::Key_Colon] = TILEM_KEY_DECPNT;
	
	m_kbdMap[Qt::Key_QuoteDbl] = TILEM_KEY_ADD;
}

void CalcView::updateLCD()
{
	// update QImage and schedule widget repaint
	if ( m_calc->lcdUpdate() )
	{
		const int w = m_calc->lcdWidth();
		const int h = m_calc->lcdHeight();
		
		QRgb *d = reinterpret_cast<QRgb*>(m_screen->bits());
		const unsigned int *cd = m_calc->lcdData();
		
		// write LCD into skin image
		for ( int i = 0; i < m_lcdH; ++i )
		{
			for ( int j = 0; j < m_lcdW; ++j )
			{
				int y = (h * i) / m_lcdH;
				int x = (w * j) / m_lcdW;
				
				//qDebug("(%i, %i) maps (%i, %i)", j, i, x, y);
				//m_screen->setPixel(j, i, m_calc->lcdData()[y * w + x]);
				d[i * m_lcdW + j] = cd[y * w + x];
			}
		}
		
		update(m_lcdX, m_lcdY, m_lcdW, m_lcdH);
	}
}

int CalcView::mappedKey(int k) const
{
	const QHash<int, int>::const_iterator it = m_kbdMap.constFind(k);
	
	return it != m_kbdMap.constEnd() ? *it : 0;
}

int CalcView::mappedKey(const QPoint& pos) const
{
	return qGray(m_keymask->pixel(pos)) ? 0 : closestKey(pos);
}

int CalcView::closestKey(const QPoint& p) const
{
	unsigned int sk_min = 0, d_min = 0x7fffffff;
	
	for ( int i = 0; i < m_kCenter.count(); ++i )
	{
		unsigned int d = (p - m_kCenter.at(i)).manhattanLength();
		
		//qDebug("?%i", d);
		if ( d < d_min )
		{
			sk_min = m_kScanCode.at(i);
			d_min = d;
		}
	}
	
	return sk_min;
}

void CalcView::keyPressEvent(QKeyEvent *e)
{
	int k = mappedKey(e->key());
	
	if ( k )
		m_calc->keyPress(k);
	else
		QFrame::keyPressEvent(e);
}

void CalcView::keyReleaseEvent(QKeyEvent *e)
{
	int k = mappedKey(e->key());
	
	if ( k )
		m_calc->keyRelease(k);
	else
		QFrame::keyReleaseEvent(e);
}

void CalcView::mousePressEvent(QMouseEvent *e)
{
	int k = mappedKey(e->pos());
	
	if ( k )
		m_calc->keyPress(k);
	else
		QFrame::mousePressEvent(e);
}

void CalcView::mouseReleaseEvent(QMouseEvent *e)
{
	int k = mappedKey(e->pos());
	
	if ( k )
		m_calc->keyRelease(k);
	else
		QFrame::mouseReleaseEvent(e);
}

void CalcView::contextMenuEvent(QContextMenuEvent *e)
{
	m_cxt->exec(e->globalPos());
}

void CalcView::timerEvent(QTimerEvent *e)
{
	if ( e->timerId() == m_lcdTimerId )
		updateLCD();
	else
		QFrame::timerEvent(e);
}

void CalcView::paintEvent(QPaintEvent *e)
{
	QFrame::paintEvent(e);
	
	QPainter p(this);
	
	// smart update to reduce repaint overhead
	if ( e->rect() != QRect(m_lcdX, m_lcdY, m_lcdW, m_lcdH) )
	{
		if ( hasFocus() )
		{
			// TODO : draw focus marker
			
		}
		
		p.drawPixmap(0, 0, *m_skin);
	}
	
	p.drawImage(m_lcdX, m_lcdY, *m_screen);
}

void CalcView::focusInEvent(QFocusEvent *e)
{
	setFrameShadow(QFrame::Raised);
	
	setBackgroundRole(QPalette::AlternateBase);
	
	QFrame::focusInEvent(e);
	
	update();
}

void CalcView::focusOutEvent(QFocusEvent *e)
{
	setFrameShadow(QFrame::Plain);
	
	setBackgroundRole(QPalette::Base);
	
	QFrame::focusOutEvent(e);
	
	update();
}

static bool isSupported(const QMimeData *md, CalcLink *m_link)
{
	if ( md->hasUrls() )
	{
		QList<QUrl> l = md->urls();
		
		foreach ( QUrl url, l )
			if ( m_link->isSupportedFile(url.toLocalFile()) )
				return true;
	}
	
	return false;
}

void CalcView::dropEvent(QDropEvent *e)
{
	QList<QUrl> l = e->mimeData()->urls();
	
	foreach ( QUrl url, l )
	{
		QString file = url.toLocalFile();
		
		if ( m_link->isSupportedFile(file) )
			m_link->send(file);
	}
	
	QFrame::dropEvent(e);
}

void CalcView::dragEnterEvent(QDragEnterEvent *e)
{
	if ( isSupported(e->mimeData(), m_link) )
		e->acceptProposedAction();
	else
		QFrame::dragEnterEvent(e);
}

void CalcView::dragMoveEvent(QDragMoveEvent *e)
{
	QFrame::dragMoveEvent(e);
}

void CalcView::dragLeaveEvent(QDragLeaveEvent *e)
{
	QFrame::dragLeaveEvent(e);
}
