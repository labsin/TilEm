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

#include "calc.h"

/*!
	\file calc.cp
	\brief Implementation of the Calc class
*/

extern "C" {
#include <scancodes.h>
}

#include <QDir>
#include <QColor>
#include <QFileInfo>
#include <QInputDialog>
#include <QScriptEngine>

QHash<TilemCalc*, Calc*> Calc::m_table;

int Calc::breakpointDispatch(TilemCalc *tc, dword a, void *d)
{
	//Calc *c = m_table.value(tc, 0);
	Calc::Breakpoint *bp = static_cast<Calc::Breakpoint*>(d);
	
	QScriptValueList args;
	args << QScriptValue(a);
	
	return bp->test.isFunction() ? bp->test.call(QScriptValue(), args).toInt32() : 1;
}

/*!
	\class Calc
	\brief Core class to manage calc emulation
	
	Abstracts away the use of libtilemcore and load/save of roms
*/

Calc::Calc(QObject *p)
 : QObject(p), m_calc(0), m_lcd(0), m_lcd_comp(0)
{
	m_script = new QScriptEngine(this);
}

Calc::~Calc()
{
	QMutexLocker lock(&m_run);
	
	m_table.remove(m_calc);
	
	// release memory
	tilem_calc_free(m_calc);
	
	delete m_lcd_comp;
	delete m_lcd;
}

QString Calc::name() const
{
	return m_name;
}

void Calc::setName(const QString& n)
{
	if ( n == m_name )
		return;
	
	m_name = n;
	
	emit nameChanged(n);
}

QString Calc::romFile() const
{
	return m_romFile;
}

int Calc::model() const
{
	return m_calc ? m_calc->hw.model_id : 0;
}

static const QHash<char, QString>& calc_models()
{
	static QHash<char, QString> _h;
	
	if ( _h.isEmpty() )
	{
		_h[TILEM_CALC_TI73] = "73";
		_h[TILEM_CALC_TI76] = "76";
		_h[TILEM_CALC_TI81] = "81";
		_h[TILEM_CALC_TI82] = "82";
		_h[TILEM_CALC_TI83] = "83";
		_h[TILEM_CALC_TI83P] = "83+";
		_h[TILEM_CALC_TI83P_SE] = "83+SE";
		_h[TILEM_CALC_TI84P] = "84+";
		_h[TILEM_CALC_TI84P_NSPIRE] = "84+nspire";
		_h[TILEM_CALC_TI84P_SE] = "84+SE";
		_h[TILEM_CALC_TI85] = "85";
		_h[TILEM_CALC_TI86] = "86";
	}
	
	return _h;
}

QString Calc::modelString() const
{
	return calc_models().value(model(), QString());
}

void Calc::resetLink()
{
	tilem_linkport_graylink_reset(m_calc);
}

bool Calc::isBroadcasting() const
{
	return m_broadcast;
}

void Calc::setBroadcasting(bool y)
{
	m_broadcast = y;
}

/*!
	\return whether the *calc* is writing data through the linkport
*/
bool Calc::isSending() const
{
	return m_output.count();
}

/*!
	\return whether the *calc* has data to read from the linkport
*/
bool Calc::isReceiving() const
{
	return m_input.count();
}

/*!
	\return amount of data written to the linkport by the *calc* that can be retrieved using getByte()
*/
uint32_t Calc::byteCount() const
{
	return m_output.count();
}

/*!
	\brief get a byte from the buffer in which calc linkport writes are stored but does not remove it from the buffer
*/
char Calc::topByte()
{
	if ( m_output.isEmpty() )
		return 0;
	
	char c = m_output.at(0);
	
	return c;
}

/*!
	\brief get a byte from the buffer in which calc linkport writes are stored
*/
char Calc::getByte()
{
	if ( m_output.isEmpty() )
		return 0;
	
	char c = m_output.at(0);
	
	m_output.remove(1);
	
	return c;
}

/*!
	\brief send a byte to the calc
*/
void Calc::sendByte(char c)
{
	m_input += c;
}

/*!
	\brief get up to n bytes from the buffer in which calc linkport writes are stored
*/
QByteArray Calc::getBytes(int n)
{
	if ( n < 0 || m_output.isEmpty() )
		return QByteArray();
	
	n = qMin(n, (int)m_output.count());
	
	return m_output.take(n);
}

/*!
	\brief get up to n bytes from the buffer in which calc linkport writes are stored
*/
int Calc::getBytes(int n, char *d)
{
	if ( n < 0 || m_output.isEmpty() )
		return 0;
	
	n = qMin(n, (int)m_output.count());
	
	m_output.take(n, d);
	
	return n;
}

/*!
	\brief send bytes to the calc
*/
void Calc::sendBytes(const QByteArray& d)
{
	m_input += d;
}

/*!
	\brief Rest calc (simulate battery pull)
*/
void Calc::reset()
{
	m_calc->hw.reset(m_calc);
}

void Calc::addBreakpoint(Breakpoint *b)
{
	if ( b->id == -1 )
	{
		b->id = tilem_z80_add_breakpoint(m_calc, b->type, b->start, b->end, b->mask, breakpointDispatch, b);
	}
}

void Calc::removeBreakpoint(Breakpoint *b)
{
	if ( b->id != -1 )
	{
		tilem_z80_remove_breakpoint(m_calc, b->id);
		b->id = -1;
	}
}

void Calc::load(const QString& file)
{
	QMutexLocker lock(&m_run);
	
	QFileInfo info(file);
	
	FILE *romfile, *savefile;
	
	romfile = fopen(qPrintable(file), "rb");
	
	if ( !romfile )
	{
		qWarning("Unable to open romfile : %s", qPrintable(file));
		return;
	} else {
		//qDebug("successfully opened %s", qPrintable(file));
	}
	
	savefile = fopen(qPrintable(QDir(info.path()).filePath(info.baseName() + ".sav")), "rt");
	
	if ( m_calc )
	{
		//qDebug("cleanin up previous state.");
		m_table.remove(m_calc);
		
		tilem_calc_free(m_calc);
		delete m_lcd_comp;
		delete m_lcd;
	}
	
	m_romFile = file;
	
	char rom_type = tilem_guess_rom_type(romfile);
	
	QStringList l = calc_models().values();
	qSort(l);
	
	QString t =
	QInputDialog::getItem(
						0,
						tr("Select ROM type"),
						tr("Select ROM type (keep default if unsure)"),
						l,
						l.indexOf(calc_models().value(rom_type)),
						false
					);
	
	rom_type = calc_models().key(t);
	
	qDebug("%c", rom_type);
	
	m_calc = tilem_calc_new(rom_type);
	m_table[m_calc] = this;
	tilem_calc_load_state(m_calc, romfile, savefile);
	
	// some link emulation magic...
	m_calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;
	m_calc->z80.stop_mask &= ~(TILEM_STOP_LINK_READ_BYTE | TILEM_STOP_LINK_WRITE_BYTE | TILEM_STOP_LINK_ERROR);
	
	m_broadcast = true;
	m_link_lock = false;
	
	// instant LCD state and "composite" LCD state (grayscale is a bitch...)
	m_lcd = new unsigned char[m_calc->hw.lcdwidth * m_calc->hw.lcdheight / 8];
	m_lcd_comp = new unsigned int[m_calc->hw.lcdwidth * m_calc->hw.lcdheight];
	
	m_calc->lcd.emuflags = TILEM_LCD_REQUIRE_DELAY;
	m_calc->flash.emuflags = TILEM_FLASH_REQUIRE_DELAY;
	
	fclose(romfile);
	
	if ( savefile )
		fclose(savefile);
	
}

void Calc::save(const QString& file)
{
	QMutexLocker lock(&m_run);
	
	QFileInfo info(file);
	
	FILE *romfile, *savefile;
	
	romfile = fopen(qPrintable(file), "wb");
	savefile = fopen(qPrintable(QDir(info.path()).filePath(info.baseName() + ".sav")), "wt");
	
	if ( romfile && savefile )
	{
		// save state
		tilem_calc_save_state(m_calc, romfile, savefile);
	} else {
		qWarning("Unable to save state.");
	}
	
	if ( romfile )
		fclose(romfile);
	
	if ( savefile )
		fclose(savefile);
	
}

int Calc::run_us(int usec)
{
	QMutexLocker lock(&m_run);
	
	if ( !m_calc )
		return -1;
	
	int remaining = usec;
	
	while ( remaining > 0 )
	{
		// try to forward data written into input buffer to link port
		
		if ( !m_link_lock )
		{
			if ( m_input.count() )
			{
				m_calc->z80.stop_reason = 0;
				
				if ( !tilem_linkport_graylink_send_byte(m_calc, m_input.at(0)) )
				{
					#ifdef TILEM_QT_LINK_DEBUG
					printf("@> %02x", static_cast<unsigned char>(m_input.at(0)));
					#endif
					
					m_input.remove(1);
					
					if ( !(m_calc->z80.stop_reason & TILEM_STOP_LINK_WRITE_BYTE) )
					{
						m_link_lock = true;
					} else {
						// here's the trick to speed things up : batch processing whenever possible
						while ( m_input.count() && (m_calc->z80.stop_reason & TILEM_STOP_LINK_WRITE_BYTE) )
						{
							m_calc->z80.stop_reason = 0;
							
							if ( !tilem_linkport_graylink_send_byte(m_calc, m_input.at(0)) )
							{
								//qDebug("@byte successfully written");
								#ifdef TILEM_QT_LINK_DEBUG
								printf(" %02x", static_cast<unsigned char>(m_input.at(0)));
								#endif
								m_input.remove(1);
							}
						}
					}
					
					#ifdef TILEM_QT_LINK_DEBUG
					printf("\n");
					fflush(stdout);
					#endif
				}
			} else {
				int b = tilem_linkport_graylink_get_byte(m_calc);
				
				if ( b != -1 )
				{
					// one byte successfully read yay!
					
					m_output += b;
					
					#ifdef TILEM_QT_LINK_DEBUG
					qDebug("@< %02x [%i] [0x%x]", static_cast<unsigned char>(b), m_output.count(), this);
					#endif
					
					if ( m_broadcast )
						emit bytesAvailable();
				}
			}
		}
		
		int res = tilem_z80_run_time(m_calc, remaining, &remaining);
		
		/*
			some link emulation magic : seamlessly transfer
			data from buffers to the calc using a virtual
			graylink to allow asynchronous transfers
		*/
		if ( res & TILEM_STOP_LINK_WRITE_BYTE )
		{
			//qDebug("@byte successfully written");
			m_link_lock = false;
			remaining = qMax(1, remaining);
		}
		
		if ( res & TILEM_STOP_LINK_ERROR )
		{
			qWarning("@link error.");
			tilem_linkport_graylink_reset(m_calc);
			break;
		}
		
// 		if ( res & TILEM_STOP_INVALID_INST )
// 		{
// 			break;
// 		}
// 		
// 		if ( res & TILEM_STOP_UNDOCUMENTED_INST )
// 		{
// 			break;
// 		}
// 		
// 		if ( res & TILEM_STOP_BREAKPOINT )
// 		{
// 			break;
// 		}
	}
	
	if ( m_calc->z80.stop_reason )
		qDebug("stop:%i", m_calc->z80.stop_reason);
	
	
// 	printf("  PC=%02X:%04X AF=%04X BC=%04X DE=%04X HL=%04X IX=%04X IY=%04X SP=%04X\n",
// 			m_calc->mempagemap[m_calc->z80.r.pc.b.h >> 6],
// 			m_calc->z80.r.pc.w.l,
// 			m_calc->z80.r.af.w.l,
// 			m_calc->z80.r.bc.w.l,
// 			m_calc->z80.r.de.w.l,
// 			m_calc->z80.r.hl.w.l,
// 			m_calc->z80.r.ix.w.l,
// 			m_calc->z80.r.iy.w.l,
// 			m_calc->z80.r.sp.w.l
// 		);
	
	
	return m_calc->z80.stop_reason;
}

int Calc::run_cc(int clock)
{
	QMutexLocker lock(&m_run);
	
	if ( !m_calc )
		return -1;
	
	int remaining = 0;
	
	tilem_z80_run(m_calc, clock, &remaining);
				
	return m_calc->z80.stop_reason;
}

void Calc::stop(int reason)
{
	// QMutexLocker lock(&m_run);
	
	if ( !m_calc )
		return;
	
	tilem_z80_stop(m_calc, reason);
}

void Calc::keyPress(int sk)
{
	tilem_keypad_press_key(m_calc, sk);
}

void Calc::keyRelease(int sk)
{
	tilem_keypad_release_key(m_calc, sk);
}

bool Calc::lcdUpdate()
{
	if ( !m_calc )
		return false;
	
	// low : black, high : white
	unsigned int low, high;
	
	// contrast determination
	if ( m_calc->lcd.active && !(m_calc->z80.halted && !m_calc->poweronhalt) )
	{
		// update "plain" LCD data
		(*m_calc->hw.get_lcd)(m_calc, m_lcd);
		
		/*
			LCD behaves roughly as follows :
			
			0->31 : high is white, low goes from white to black
			32->63 : low is black, high goes from white to black
		*/
		const int c = 63 - int(m_calc->lcd.contrast);
		//low = qMin(31, c) / 8  + qMax(0, c - 31) * 4;
		//high = qMax(31, c) * 4 + qMin(0, c - 31) / 8;
		
		low = 0x00;
		high = 0xff;
		
// 		if ( c < 32 ) {
// 			low = 0;
// 			high = c * 8;
// 		} else {
// 			low = (c - 32) * 8;
// 			high = 255;
// 		}
	} else {
		low = high = 0xff;
	}
	
	// update "composite" LCD data
	bool changed = false;
	
	const unsigned int end = m_calc->hw.lcdheight * m_calc->hw.lcdwidth;
	
	for ( unsigned int idx = 0; idx < end; ++idx )
	{
		unsigned int v = m_lcd[idx >> 3] & (0x80 >> (idx & 7)) ? low : high;
		
		// TODO : blending for nice grayscale
		//unsigned int g = (qRed(m_lcd_comp[idx]) + 2*v) / 3;
		unsigned int g = v + ((qRed(m_lcd_comp[idx]) - v) * 7) / 8;
		
		v = qRgb(g, g, g);
		
		if ( v != m_lcd_comp[idx] )
		{
			changed = true;
			m_lcd_comp[idx] = v;
		}
	}
	
	return changed;
}

int Calc::lcdWidth() const
{
	return m_calc ? m_calc->hw.lcdwidth : 0;
}

int Calc::lcdHeight() const
{
	return m_calc ? m_calc->hw.lcdheight : 0;
}

const unsigned int* Calc::lcdData() const
{
	return m_lcd_comp;
}

/*
	Implementation of libtilemcore memory allocation and debug output routines
*/

extern "C" {
void tilem_free(void* p)
{
	free(p);
}

void* tilem_malloc(size_t s)
{
	return malloc(s);
}

void* tilem_realloc(void* p, size_t s)
{
	return realloc(p, s);
}

void* tilem_try_malloc(size_t s)
{
	return malloc(s);
}

void* tilem_malloc0(size_t s)
{
	return calloc(s, 1);
}

void* tilem_try_malloc0(size_t s)
{
	return calloc(s, 1);
}

void* tilem_malloc_atomic(size_t s)
{
	return malloc(s);
}

void* tilem_try_malloc_atomic(size_t s)
{
	return malloc(s);
}

/* Logging */

void tilem_message(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
}

void tilem_warning(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: WARNING: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
}

void tilem_internal(TilemCalc* calc, const char* msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	fprintf(stderr, "x%c: INTERNAL ERROR: ", calc->hw.model_id);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
	va_end(ap);
}
}
