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
	if ( e->timerId() == m_refreshId )
	{
		if ( tbPages->currentIndex() == 1 )
		{
// 			le_af->update();
// 			le_bc->update();
// 			le_de->update();
// 			le_hl->update();
// 			
// 			le_af_shadow->update();
// 			le_bc_shadow->update();
// 			le_de_shadow->update();
// 			le_hl_shadow->update();
// 			
// 			le_ix->update();
// 			le_iy->update();
// 			
// 			le_pc->update();
// 			le_sp->update();
// 			
// 			le_ir->update();
			pageCPU->repaint();
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
	}
}

void CalcDebuger::on_spnRefresh_valueChanged(int val)
{
	killTimer(m_refreshId);
	m_refreshId = startTimer(val);
}
