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

/*!
	\class Calc
	\brief Core class to manage calc emulation
	
	Abstracts away the use of libtilemcore and load/save of roms
*/

Calc::Calc(QObject *p)
 : QObject(p), m_calc(0), m_lcd(0), m_lcd_comp(0)
{
	
}

Calc::~Calc()
{
	QMutexLocker lock(&m_run);
	
	// release memory
	tilem_calc_free(m_calc);
	
	delete m_lcd_comp;
	delete m_lcd;
}

QString Calc::romFile() const
{
	return m_romFile;
}

int Calc::model() const
{
	return m_calc ? m_calc->hw.model_id : 0;
}

QString Calc::modelString() const
{
	switch ( model() )
	{
		case TILEM_CALC_TI73:
			return "73";
		case TILEM_CALC_TI76:
			return "76";
		case TILEM_CALC_TI81:
			return "81";
		case TILEM_CALC_TI82:
			return "82";
		case TILEM_CALC_TI83:
			return "83";
		case TILEM_CALC_TI83P_SE:
			return "83pSE";
		case TILEM_CALC_TI84P:
			return "84pBE";
		case TILEM_CALC_TI84P_SE:
			return "84pSE";
		case TILEM_CALC_TI85:
			return "85";
		case TILEM_CALC_TI86:
			return "86";
		default:
			break;
	}
	
	return "83pBE";
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
		
		tilem_calc_free(m_calc);
		delete m_lcd_comp;
		delete m_lcd;
	}
	
	m_romFile = file;
	
	m_calc = tilem_calc_new(tilem_guess_rom_type(romfile));
	tilem_calc_load_state(m_calc, romfile, savefile);
	
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
	
	int remaining = 0;
	
	tilem_z80_run_time(m_calc, usec, &remaining);
				
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
	
	unsigned int low = qRgb(0xff, 0xff, 0xff), high = qRgb(0xff, 0xff, 0xff);
	
	// contrast determination (FIXME)
	if ( m_calc->lcd.active && !(m_calc->z80.halted && !m_calc->poweronhalt) )
	{
		// update "plain" LCD data
		(*m_calc->hw.get_lcd)(m_calc, m_lcd);
		
		// adjust "base" levels to tkae ocntrast into account
		if (m_calc->lcd.contrast < 32) {
			unsigned char a = m_calc->lcd.contrast * 4;
			high = qRgb(a, a, a);
		} else {
			unsigned char a = (m_calc->lcd.contrast - 32) * 4;
			low = qRgb(a, a, a);
			high = qRgb(0xff, 0xff, 0xff);
		}
	}
	
	// update "composite" LCD data
	bool changed = false;
	
	const unsigned int end = m_calc->hw.lcdheight * m_calc->hw.lcdwidth;
	
	for ( unsigned int idx = 0; idx < end; ++idx )
	{
		unsigned int v = m_lcd[idx >> 3] & (0x80 >> (idx & 7)) ? low : high;
		
		// TODO : blending for nice grayscale
		//v += ((m_lcd_comp[idx] - v) * 3) / 4;
		
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
