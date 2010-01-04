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
		
// 		virtual QModelIndex index(int row, int col, const QModelIndex& p) const
// 		{
// 			return (p.isValid() || col < 0 || col >= 2 || row < 0 || row >= m_grid->calcCount())
// 					? QModelIndex()
// 					: createIndex(row, col, 0);
// 		}
		
// 		virtual QModelIndex parent(const QModelIndex& i) const
// 		{
// 			return QModelIndex();
// 		}
		
		virtual int rowCount(const QModelIndex& i) const
		{
			return i.isValid() ? 0 : m_grid->calcCount();
		}
		
// 		virtual int columnCount(const QModelIndex& i) const
// 		{
// 			return 1;
// 		}
		
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
}

CalcDebuger::~CalcDebuger()
{
	
}

QSize CalcDebuger::sizeHint() const
{
	return QSize(300, 800);
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
		
	}
}
