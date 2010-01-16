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

#include "calcdebuger.h"

/*!
	\file calcdebuger.cpp
	\brief Implementation of the CalcDebuger class
*/

#include "calc.h"
#include "calcview.h"
#include "calcgrid.h"

#include <QAbstractListModel>

int breakpointDispatch(TilemCalc *tc, dword a, void *d)
{
// 	Calc::Breakpoint *bp = static_cast<Calc::Breakpoint*>(d);
// 	
// 	QScriptValueList args;
// 	args << QScriptValue(a);
// 	
// 	return bp->test.isFunction() ? bp->test.call(QScriptValue(), args).toInt32() : 1;
	return 0;
}

class BreakpointModel : public QAbstractListModel
{
	Q_OBJECT
	
	public:
		BreakpointModel(QObject *p = 0)
		 : QAbstractListModel(p), m_calc(0)
		{
			
		}
		
		void setCalc(TilemCalc *c)
		{
// 			QAbstractItemModel::beginResetModel();
			
			m_calc = c;
			reset();
// 			QAbstractItemModel::endResetModel();
		}
		
		virtual int rowCount(const QModelIndex& i) const
		{
			int n = 0;
			
			for ( int i = 0; i < m_calc->z80.nbreakpoints; ++i )
				if ( m_calc->z80.breakpoints[i].type )
					++n;
			
			return n;
		}
		
		virtual QVariant data(const QModelIndex& i, int r) const
		{
			int id = nth(i.row());
			
			return (r == Qt::DisplayRole && id != -1) ? QString::number(m_calc->z80.breakpoints[id].start) : QVariant();
		}
		
		int nth(int n) const
		{
			int k = 0;
			
			for ( int i = 0; i < m_calc->z80.nbreakpoints; ++i )
				if ( m_calc->z80.breakpoints[i].type )
					if ( k++ == n )
						return i;
			
			return -1;
		}
		
	private:
		TilemCalc *m_calc;
};

class CalcListModel : public QAbstractListModel
{
	Q_OBJECT
	
	public:
		CalcListModel(CalcGrid *g, QObject *p = 0)
		 : QAbstractListModel(p), m_grid(g)
		{
			connect(g, SIGNAL( beginAddCalc(int) ), this, SLOT( beginAddCalc(int) ));
			connect(g, SIGNAL( endAddCalc() ), this, SLOT( endAddCalc() ));
			connect(g, SIGNAL( beginRemoveCalc(int) ), this, SLOT( beginRemoveCalc(int) ));
			connect(g, SIGNAL( endRemoveCalc() ), this, SLOT( endRemoveCalc() ));
		}
		
		virtual int rowCount(const QModelIndex& i) const
		{
			return i.isValid() ? 0 : m_grid->calcCount();
		}
		
		virtual QVariant data(const QModelIndex& i, int r) const
		{
			return r == Qt::DisplayRole ? m_grid->calc(i.row())->calc()->name() : QVariant();
		}
		
	private slots:
		void beginAddCalc(int i)
		{
			beginInsertRows(QModelIndex(), i, i);
		}
		
		void endAddCalc()
		{
			endInsertRows();
		}
		
		void beginRemoveCalc(int i)
		{
			beginRemoveRows(QModelIndex(), i, i);
		}
		
		void endRemoveCalc()
		{
			endRemoveRows();
		}
		
	private:
		CalcGrid *m_grid;
};

#include "calcdebuger.moc"

/*!
	\class CalcDebuger
	\brief A widget offering debuging capabilities for an emulated calc
*/

CalcDebuger::CalcDebuger(CalcGrid *g, QWidget *p)
 : QWidget(p), m_calcGrid(g)
{
	setupUi(this);
	
	cbTarget->setModel(new CalcListModel(g, this));
	lwBreakpoints->setModel(new BreakpointModel(this));
	
	tbPages->setEnabled(false);
	
	m_refreshId = startTimer(spnRefresh->value());
}

CalcDebuger::~CalcDebuger()
{
	
}

QSize CalcDebuger::sizeHint() const
{
	return QSize(300, 800);
}

void CalcDebuger::timerEvent(QTimerEvent *e)
{
	if ( isVisible() && m_calc && (e->timerId() == m_refreshId) )
	{
		const byte flags = m_calc->m_calc->z80.r.af.b.l;
		
		switch ( tbPages->currentIndex() )
		{
			case 1:
				// update flags
				chk_flag_S->setChecked(flags & FLAG_S);
				chk_flag_Z->setChecked(flags & FLAG_Z);
				chk_flag_5->setChecked(flags & FLAG_Y);
				chk_flag_H->setChecked(flags & FLAG_H);
				chk_flag_3->setChecked(flags & FLAG_X);
				chk_flag_P->setChecked(flags & FLAG_P);
				chk_flag_N->setChecked(flags & FLAG_N);
				chk_flag_C->setChecked(flags & FLAG_C);
				
				/*
					force repaint of all the watch widgets for changes to be visible
					
					optimization : directly repaint the parent widget to reduce overhead...
				*/
				pageCPU->repaint();
				
				break;
				
			case 2:
			{
				chk_flash_lock->setChecked(!m_calc->m_calc->flash.unlock);
				
				pageMemory->repaint();
				break;
			}
				
			default:
				break;
		}
	}
}

void CalcDebuger::on_cbTarget_currentIndexChanged(int idx)
{
	CalcView *v = idx != -1 ? m_calcGrid->calc(idx) : 0;
	Calc *c = v ? v->calc() : 0;
	
	if ( m_calc == c )
		return;
	
	if ( m_calc )
	{
		
	}
	
	m_calc = c;
	
	if ( m_calc )
	{
		tbPages->setEnabled(true);
		
		// breakpoints page
		qobject_cast<BreakpointModel*>(lwBreakpoints->model())->setCalc(m_calc->m_calc);
		
		// CPU page
		
		TilemZ80Regs& regs = m_calc->m_calc->z80.r;
		
		le_af->setPointer(&regs.af.w.l);
		le_bc->setPointer(&regs.bc.w.l);
		le_de->setPointer(&regs.de.w.l);
		le_hl->setPointer(&regs.hl.w.l);
		
		le_af_shadow->setPointer(&regs.af2.w.l);
		le_bc_shadow->setPointer(&regs.bc2.w.l);
		le_de_shadow->setPointer(&regs.de2.w.l);
		le_hl_shadow->setPointer(&regs.hl2.w.l);
		
		le_ix->setPointer(&regs.ix.w.l);
		le_iy->setPointer(&regs.iy.w.l);
		
		le_pc->setPointer(&regs.pc.w.l);
		le_sp->setPointer(&regs.sp.w.l);
		
		le_ir->setPointer(&regs.ir.w.l);
		
		
		// memory page
		
		le_bankA->setPointer(m_calc->m_calc->mempagemap + 1);
		le_bankB->setPointer(m_calc->m_calc->mempagemap + 2);
		le_bankC->setPointer(m_calc->m_calc->mempagemap + 3);
		
		// ports page
		
		
	} else {
		tbPages->setEnabled(false);
	}
}

void CalcDebuger::on_spnRefresh_valueChanged(int val)
{
	if ( m_refreshId != -1 )
		killTimer(m_refreshId);
	
	m_refreshId = val > 0 ? startTimer(val) : -1;
}
