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

#include "calclink.h"

/*!
	\file calclink.cpp
	\brief Implementation of the CalcLink class
*/

#include "calc.h"

#include <QStringList>

#ifdef _TILEM_QT_HAS_LINK_
static int get_calc_model(TilemCalc* calc);
static CableHandle* internal_link_handle_new(TilemCalc *calc);
static void send_file(TilemCalc *calc, CalcHandle* ch, int first, int last, const char* filename);
#endif

/*!
	\class CalcLink
	\brief Utility class that wraps link emulation
	
	Abstracts away the use of libticalcs / libticables / libticonv and the way
	they are "bridged" to libtilemcore
*/

CalcLink::CalcLink(Calc *c, QObject *p)
 : QObject(p), m_calc(0)
{
	#ifdef _TILEM_QT_HAS_LINK_
	// init linking-related libs
	ticables_library_init();
	tifiles_library_init();
	ticalcs_library_init();
	#endif
	
	setCalc(c);
}

CalcLink::~CalcLink()
{
	#ifdef _TILEM_QT_HAS_LINK_
	ticalcs_library_exit();
	tifiles_library_exit();
	ticables_library_exit();
	#endif
}

void CalcLink::setCalc(Calc *c)
{
	#ifdef _TILEM_QT_HAS_LINK_
	if ( m_calc )
	{
		ticalcs_cable_detach(m_ch);
		ticalcs_handle_del(m_ch);
		ticables_handle_del(m_cbl);
	}
	
	m_calc = c;
	
	if ( m_calc )
	{
		m_cbl = internal_link_handle_new(c->m_calc);
		
		if ( !m_cbl )
		{
			qFatal("Cannot create ilp handle");
		}
		
		m_ch = ticalcs_handle_new(static_cast<CalcModel>(get_calc_model(c->m_calc)));
		
		if ( !m_ch )
		{
			qFatal("Cannot create calc handle");
		}
		
		ticalcs_cable_attach(m_ch, m_cbl);
	}
	#endif
}

bool CalcLink::isSupportedFile(const QString& file) const
{
	#ifdef _TILEM_QT_HAS_LINK_
	static const QList<int> supportedClasses = QList<int>()
		<< TIFILE_SINGLE << TIFILE_GROUP   << TIFILE_REGULAR << TIFILE_BACKUP
		<< TIFILE_FLASH  << TIFILE_TIGROUP << TIFILE_OS      << TIFILE_APP
		;
	
	const char *d = file.toLocal8Bit().constData();
	
	int fileModel = tifiles_file_get_model(d);
	int fileClass = tifiles_file_get_class(d);
	
	return
			tifiles_calc_are_compat(
				static_cast<CalcModel>(fileModel),
				static_cast<CalcModel>(get_calc_model(m_calc->m_calc))
			)
		&&
			supportedClasses.contains(fileClass)
		;
	
	#endif
	
	return false;
}

void CalcLink::send(const QString& f)
{
	#ifdef _TILEM_QT_HAS_LINK_
	// mutex lock to avoid simulation to kick back in during linking...
	QMutexLocker lock(&m_calc->m_run);
	
	send_file(m_calc->m_calc, m_ch, 1, 0, f.toLocal8Bit().constData());
	#endif
}

#ifdef _TILEM_QT_HAS_LINK_
int get_calc_model(TilemCalc* calc)
{
	switch ( calc->hw.model_id ) {
	case TILEM_CALC_TI73:
		return CALC_TI73;

	case TILEM_CALC_TI81:
	case TILEM_CALC_TI82:
		return CALC_TI82;

	case TILEM_CALC_TI83:
		return CALC_TI83;

	case TILEM_CALC_TI83P:
	case TILEM_CALC_TI83P_SE:
		return CALC_TI83P;

	case TILEM_CALC_TI84P:
	case TILEM_CALC_TI84P_SE:
		return CALC_TI84P;

	case TILEM_CALC_TI85:
		return CALC_TI85;

	case TILEM_CALC_TI86:
		return CALC_TI86;

	default:
		return CALC_NONE;
	}
}

// linking emulation

static int ilp_reset(CableHandle* cbl)
{
	TilemCalc *calc = static_cast<TilemCalc*>(cbl->priv);
	
	tilem_linkport_graylink_reset(calc);
	
	return 0;
}

static int ilp_send(CableHandle* cbl, uint8_t* data, uint32_t count)
{
	TilemCalc *calc = static_cast<TilemCalc*>(cbl->priv);
	
	int status = 0;
	unsigned int i;
	dword prevmask;
	
	calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;
	prevmask = calc->z80.stop_mask;
	calc->z80.stop_mask = ~(TILEM_STOP_LINK_READ_BYTE
				      | TILEM_STOP_LINK_WRITE_BYTE
				      | TILEM_STOP_LINK_ERROR);
	
	tilem_z80_run_time(calc, 1000, NULL);
	
	#ifdef _TILEM_LINK_DEBUG_
	printf(" >>");
	#endif
	
	while ( count > 0 )
	{
		if ( !tilem_linkport_graylink_send_byte(calc, data[0]) )
		{
			#ifdef _TILEM_LINK_DEBUG_
			printf(" %02X", data[0]);
			#endif
			++data;
			--count;
		}
		
		for ( i = 0; i < cbl->timeout; i++)
			if ( tilem_linkport_graylink_ready(calc)
			    || tilem_z80_run_time(calc, 100000, NULL))
				break;
		
		if ( i == cbl->timeout || (calc->z80.stop_reason & TILEM_STOP_LINK_ERROR))
		{
			tilem_linkport_graylink_reset(calc);
			status = ERROR_WRITE_TIMEOUT;
			break;
		}
	}
	#ifdef _TILEM_LINK_DEBUG_
	printf("\n");
	#endif

	calc->linkport.linkemu = TILEM_LINK_EMULATOR_NONE;
	calc->z80.stop_mask = prevmask;
	
	return status;
}

static int ilp_recv(CableHandle* cbl, uint8_t* data, uint32_t count)
{
	TilemCalc *calc = static_cast<TilemCalc*>(cbl->priv);
	
	int status = 0;
	int value;
	unsigned int i;
	dword prevmask;
	
	calc->linkport.linkemu = TILEM_LINK_EMULATOR_GRAY;
	prevmask = calc->z80.stop_mask;
	calc->z80.stop_mask = ~(TILEM_STOP_LINK_READ_BYTE
				      | TILEM_STOP_LINK_WRITE_BYTE
				      | TILEM_STOP_LINK_ERROR);
	
	tilem_z80_run_time(calc, 1000, NULL);
	
	#ifdef _TILEM_LINK_DEBUG_
	printf(" <<");
	#endif
	
	while ( count > 0 )
	{
		value = tilem_linkport_graylink_get_byte(calc);
		
		if ( value != -1 )
		{
			#ifdef _TILEM_LINK_DEBUG_
			printf(" %02X", value);
			#endif
			
			data[0] = value;
			++data;
			--count;
			
			if (!count)
				break;
		}
		
		for ( i = 0; i < cbl->timeout; ++i )
			if (tilem_z80_run_time(calc, 100000, NULL))
				break;
		
		if ( i == cbl->timeout || (calc->z80.stop_reason & TILEM_STOP_LINK_ERROR) )
		{
			tilem_linkport_graylink_reset(calc);
			status = ERROR_READ_TIMEOUT;
			break;
		}
	}
	#ifdef _TILEM_LINK_DEBUG_
	printf("\n");
	#endif
	
	calc->linkport.linkemu = TILEM_LINK_EMULATOR_NONE;
	calc->z80.stop_mask = prevmask;
	
	return status;

}

static int ilp_check(CableHandle* cbl, int* status)
{
	TilemCalc *calc = static_cast<TilemCalc*>(cbl->priv);
	
	*status = STATUS_NONE;
	if ( calc->linkport.lines )
		*status |= STATUS_RX;
	if ( calc->linkport.extlines )
		*status |= STATUS_TX;
	
	return 0;
}

static CableHandle* internal_link_handle_new(TilemCalc *calc)
{
	CableHandle* cbl;

	cbl = ticables_handle_new(CABLE_ILP, PORT_0);
	
	if ( cbl )
	{
		cbl->priv = calc;
		cbl->cable->reset = ilp_reset;
		cbl->cable->send = ilp_send;
		cbl->cable->recv = ilp_recv;
		cbl->cable->check = ilp_check;
	}
	
	return cbl;
}

static int print_tilibs_error(int errcode)
{
	char* p = NULL;
	if (errcode) {
		if (ticalcs_error_get(errcode, &p)
		    && tifiles_error_get(errcode, &p)
		    && ticables_error_get(errcode, &p)) {
			fprintf(stderr, "Unknown error: %d\n", errcode);
		}
		else {
			fprintf(stderr, "%s\n", p);
			free(p);
		}
	}
	return errcode;
}

static void send_file(TilemCalc *calc, CalcHandle* ch,
		      int first, int last, const char* filename)
{
	CalcScreenCoord sc;
	uint8_t *bitmap = NULL;
	//int tmr, k;

	if (first) {
		//tilem_z80_run_time(calc, 500000, NULL);
		//tilem_keypad_press_key(calc, TILEM_KEY_ON);
		//tilem_z80_run_time(calc, 1000000, NULL);
		//tilem_keypad_release_key(calc, TILEM_KEY_ON);
		//tilem_z80_run_time(calc, 500000, NULL);
	}

	switch ( tifiles_file_get_class(filename) )
	{
	case TIFILE_SINGLE:
	case TIFILE_GROUP:
	case TIFILE_REGULAR:
		print_tilibs_error(ticalcs_calc_send_var2(ch, static_cast<CalcMode>(last ? MODE_SEND_LAST_VAR : 0), filename));
		break;

	case TIFILE_BACKUP:
		/* press enter to accept backup */
// 		if (calc->hw.model_id == TILEM_CALC_TI85
// 		    || calc->hw.model_id == TILEM_CALC_TI86) {
// 			k = TILEM_KEY_YEQU;
// 		}
// 		else {
// 			k = TILEM_KEY_ENTER;
// 		}
// 
// 		tmr = tilem_z80_add_timer(calc, 1000000, 0, 1,
// 					  tmr_press_key,
// 					  TILEM_DWORD_TO_PTR(k));
// 
// 		print_tilibs_error(ticalcs_calc_send_backup2(ch, filename));
// 		tilem_z80_remove_timer(calc, tmr);
// 		tilem_keypad_release_key(calc, k);
// 
// 		if (!last) {
// 			//prepare_for_link(calc);
// 		}
		break;

	case TIFILE_FLASH:
	case TIFILE_OS:
	case TIFILE_APP:
		if (tifiles_file_is_os(filename))
			print_tilibs_error(ticalcs_calc_send_os2(ch, filename));
		else if (tifiles_file_is_app(filename))
			print_tilibs_error(ticalcs_calc_send_app2(ch, filename));
		break;

	case TIFILE_TIGROUP:
		print_tilibs_error(ticalcs_calc_send_tigroup2(ch, filename, TIG_ALL));
		break;

	default:
		if (1)
			print_tilibs_error(ticalcs_calc_send_key(ch, 0xA0));
		else {
			print_tilibs_error(ticalcs_calc_recv_screen(ch, &sc, &bitmap));
			free(bitmap);
		}
		break;
	}
}
#endif
