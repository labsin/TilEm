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

#include <QQueue>
#include <QThread>
#include <QStringList>
#include <QApplication>
#include <QMutex>

#ifdef _TILEM_QT_HAS_LINK_
static int get_calc_model(TilemCalc* calc);
static CableHandle* external_link_handle_new();
static CableHandle* internal_link_handle_new(Calc *calc);
static void send_file(CalcHandle* ch, int first, int last, const char* filename);
#endif

/*!
	\internal
	\class FileSender
	\brief Utility class to send files to the calculator
	
	The point of this class is to abstract away file sending using tilibs
	and to thread it to avoid GUI freeze. 
*/
class FileSender : public QThread
{
	public:
		FileSender(CalcLink *l)
		 : QThread(0), m_link(l)
		{
		}
		
		void send(const QString& s)
		{
			m_lock.lock();
			m_files.enqueue(s);
			m_lock.unlock();
			
			if ( !isRunning() )
				start();
		}
		
	protected:
		virtual void run()
		{
			while ( m_files.count() )
			{
				m_lock.lock();
				QString f = m_files.dequeue();
				m_lock.unlock();
				
				#ifdef _TILEM_QT_HAS_LINK_
				// avoid conflicts with calc2calc direct connections
				bool broadcast = m_link->m_calc->isBroadcasting();
				bool extlink = m_link->hasExternalLink();
				
				// prevent extlink from randomly messing with transfer
				if ( extlink )
					m_link->releaseExternalLink();
				
				// TODO : first wait for any exchange using direct connection to end...
				m_link->m_calc->setBroadcasting(false);
				
				send_file(m_link->m_ch, 1, 0, f.toLocal8Bit().constData());
				
				m_link->m_calc->setBroadcasting(broadcast);
				
				if ( extlink )
					m_link->grabExternalLink();
				
				#endif
			}
		}
		
	private:
		CalcLink *m_link;
		
		QMutex m_lock;
		QQueue<QString> m_files;
};

/*!
	\class CalcLink
	\brief Utility class that wraps link emulation
	
	Abstracts away the use of libticalcs / libticables / libticonv and the way
	they are "bridged" to libtilemcore
*/

int CalcLink::m_count = 0;

#ifdef _TILEM_QT_HAS_LINK_
CableHandle* CalcLink::m_ext = 0;
int CalcLink::m_ext_cable_timer = -1;
#endif

CalcLink::CalcLink(Calc *c, QObject *p)
 : QObject(p), m_calc(0)
#ifdef _TILEM_QT_HAS_LINK_
 , m_ch(0), m_cbl(0)
#endif
{
	m_sender = new FileSender(this);
	
	if ( !m_count )
	{
		#ifdef _TILEM_QT_HAS_LINK_
		// init linking-related libs
		ticables_library_init();
		tifiles_library_init();
		ticalcs_library_init();
		
		// init external linking
		m_ext = external_link_handle_new();
		#endif
	}
	
	++m_count;
	
	setCalc(c);
}

CalcLink::~CalcLink()
{
	--m_count;
	
	setCalc(0);
	
	if ( !m_count )
	{
		#ifdef _TILEM_QT_HAS_LINK_
		ticables_cable_close(m_ext);
		ticables_handle_del(m_ext);
		
		ticalcs_library_exit();
		tifiles_library_exit();
		ticables_library_exit();
		#endif
	}
}

bool CalcLink::hasExternalLink() const
{
	#ifdef _TILEM_QT_HAS_LINK_
	return m_ext->priv == this;
	#else
	return false;
	#endif
}

void CalcLink::grabExternalLink()
{
	#ifdef _TILEM_QT_HAS_LINK_
	if ( m_ext->priv )
	{
		CalcLink *l = static_cast<CalcLink*>(m_ext->priv);
		
		l->killTimer(m_ext_cable_timer);
		m_ext_cable_timer = -1;
		
		if ( l->m_calc )
			l->m_calc->setBroadcasting(true);
		
		emit l->externalLinkGrabbed(false);
	}
	
	ticables_cable_reset(m_ext);
	m_ext->priv = this;
	
	if ( m_calc )
	{
		m_calc->setBroadcasting(false);
		
		m_ext_cable_timer = startTimer(5);
		
// 		qDebug("grabbed");
		emit externalLinkGrabbed(true);
	} else {
// 		qDebug("ungrabed");
		emit externalLinkGrabbed(false);
	}
	#endif
}

void CalcLink::releaseExternalLink()
{
	#ifdef _TILEM_QT_HAS_LINK_
	if ( !hasExternalLink() )
		return;
	
	killTimer(m_ext_cable_timer);
	m_ext_cable_timer = -1;
	m_ext->priv = 0;
	
	if ( m_calc )
	{
// 		qDebug("released");
		m_calc->setBroadcasting(true);
	}
	
	emit externalLinkGrabbed(false);
	#endif
}

void CalcLink::timerEvent(QTimerEvent *e)
{
	#ifdef _TILEM_QT_HAS_LINK_
	if ( (e->timerId() == m_ext_cable_timer) && hasExternalLink() )
	{
		int error;
		CableStatus stat;
		
		if ( (error = ticables_cable_check(m_ext, &stat)) )
		{
			qDebug("error : unable to retrieve cable status [%i]", error);
			
			// resetExtLink();
		} else if ( stat == STATUS_RX ) {
			byte b;
			
			while ( stat == STATUS_RX )
			{
				error = ticables_cable_get(m_ext, &b);
				
				if ( error )
				{
					qDebug("error : unable to get byte from cable [%i]", error);
					
					// resetExtLink();
					ticables_cable_reset(m_ext);
					break;
				} else {
					//qDebug("byte received from cable.");
					//qDebug("< 0x%02x", b);
					m_calc->sendByte(b);
					error = ticables_cable_check(m_ext, &stat);
				}
			}
		} else if ( stat == STATUS_TX ) {
			qDebug("busy?");
		} else if ( m_calc->byteCount() ) {
			int limit = 100;
			
			while ( m_calc->byteCount() && !error && limit )
			{
				byte b = m_calc->topByte();
				error = ticables_cable_put(m_ext, b);
				
				if ( error )
				{
					qDebug("error : unable to send byte through cable [%i]", error);
					
					// resetExtLink();
					ticables_cable_reset(m_ext);
					break;
				} else {
					//qDebug("byte sent through cable");
					//qDebug("> 0x%02x", b);
					m_calc->getByte();
				}
				
				--limit;
			}
		} else if ( stat ) {
			qDebug("%i ???", stat);
		}
	}
	#endif
	
	QObject::timerEvent(e);
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
	
	bool el = hasExternalLink();
	
	if ( el )
		releaseExternalLink();
	
	m_calc = c;
	
	if ( m_calc )
	{
		m_cbl = internal_link_handle_new(c);
		
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
	
	if ( el )
		grabExternalLink();
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
	m_sender->send(f);
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
	Calc *calc = static_cast<Calc*>(cbl->priv);
	
	calc->resetLink();
	
	return 0;
}

static int ilp_send(CableHandle* cbl, uint8_t* data, uint32_t count)
{
	Calc *calc = static_cast<Calc*>(cbl->priv);
	
	#ifdef TILEM_QT_LINK_DEBUG
	printf("<");
	
	for ( int i = 0; i < count; ++i )
		printf(" %02x", data[i]);
	
	printf("\n");
	
	fflush(stdout);
	#endif
	
	QByteArray d;
	d.resize(count);
	
	for ( uint32_t i = 0; i < count; ++i )
		d[i] = data[i];
	
	calc->sendBytes(d);
	
	// wait for bytes to be processed...
	while ( calc->isReceiving() )
	{
		usleep(1000);
	}
	
	return 0;
}

static int ilp_recv(CableHandle* cbl, uint8_t* data, uint32_t count)
{
	Calc *calc = static_cast<Calc*>(cbl->priv);
	
	uint32_t newc, oldc = calc->byteCount();
	
	while ( oldc < count )
	{
		qDebug("%i/%i [0x%x]", oldc, count, quintptr(calc));
		
		do
		{
			usleep(1000);
			newc = calc->byteCount();
		} while ( newc <= oldc );
		
		oldc = newc;
	}
	
	calc->getBytes(count, reinterpret_cast<char*>(data));
	
	#ifdef TILEM_QT_LINK_DEBUG
	printf(">");
	
	for ( int i = 0; i < count; ++i )
		printf(" %02x", data[i]);
	
	printf("\n");
	
	fflush(stdout);
	#endif
	
	return 0;
}

static int ilp_check(CableHandle* cbl, int* status)
{
	Calc *calc = static_cast<Calc*>(cbl->priv);
	
	*status = STATUS_NONE;
	
	if ( calc->isSending() )
		*status |= STATUS_TX;
	if ( calc->isReceiving() )
		*status |= STATUS_RX;
	
	return 0;
}

static CableHandle* internal_link_handle_new(Calc *calc)
{
	CableHandle* cbl = ticables_handle_new(CABLE_ILP, PORT_0);
	
	if ( cbl )
	{
		cbl->priv = calc;
		cbl->cable->reset = ilp_reset;
		cbl->cable->send = ilp_send;
		cbl->cable->recv = ilp_recv;
		cbl->cable->check = ilp_check;
	} else {
		qDebug("unable to create internal link handle");
	}
	
	return cbl;
}

static CableHandle* external_link_handle_new()
{
	CableHandle* cbl = ticables_handle_new(CABLE_TIE, PORT_0);
	
	if ( cbl )
	{
		ticables_options_set_timeout(cbl, DFLT_TIMEOUT);
		ticables_options_set_delay(cbl, DFLT_DELAY);
		
		cbl->priv = 0;
// 		cbl->cable->reset = ilp_reset;
// 		cbl->cable->send = ilp_send;
// 		cbl->cable->recv = ilp_recv;
// 		cbl->cable->check = ilp_check;
	} else {
		qDebug("unable to create external link handle");
	}
	
	ticables_cable_open(cbl);
	
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

static void send_file(CalcHandle* ch, int first, int last, const char* filename)
{
	Q_UNUSED(first)
	
	CalcScreenCoord sc;
	uint8_t *bitmap = NULL;
	
	switch ( tifiles_file_get_class(filename) )
	{
	case TIFILE_SINGLE:
	case TIFILE_GROUP:
	case TIFILE_REGULAR:
		print_tilibs_error(ticalcs_calc_send_var2(ch, static_cast<CalcMode>(last ? MODE_SEND_LAST_VAR : 0), filename));
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
