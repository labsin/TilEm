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

#include "calcgridmanager.h"

/*!
	\file calcgridmanager.cpp
	\brief Implementation of the CalcGridManager class
*/

#include "calc.h"
#include "calclink.h"
#include "calcview.h"
#include "calcgrid.h"
#include "connectionmanager.h"

/*!
	\internal
	\class CalcTreeModel
	\brief Internal mode of the CalcGridManager
	
	Internal id of indices :
	* 8 bits : unused
	* 8 bits : depth (at this stage either 0 or 1...)
	* 16 bits : calc index
	
	Data to make accessible :
	
	Calc name | ?
		rom file ?
		status : running/stopped
		visibility : in grid/floating/hidden
		has external link?
		calc2calc link?
		
*/
class CalcTreeModel : public QAbstractItemModel
{
	Q_OBJECT
	
	public:
		enum Properties
		{
			RomFile,
			Status,
			Visibility,
			ExternalLink,
			Calc2CalcLink,
			PropCount
		};
		
		CalcTreeModel(CalcGrid *g, QObject *p = 0)
		 : QAbstractItemModel(p), m_grid(g)
		{
			connect(g, SIGNAL( beginAddCalc(int) ), this, SLOT( beginAddCalc(int) ));
			connect(g, SIGNAL( endAddCalc() ), this, SLOT( endAddCalc() ));
			connect(g, SIGNAL( beginRemoveCalc(int) ), this, SLOT( beginRemoveCalc(int) ));
			connect(g, SIGNAL( endRemoveCalc() ), this, SLOT( endRemoveCalc() ));
		}
		
		inline int depth(const QModelIndex& i) const
		{
			return i.isValid() ? ((i.internalId() >> 16) & 0x000000ff) : 0;
		}
		
		inline CalcView* calc(const QModelIndex& i) const
		{
			return i.isValid() ? m_grid->m_calcs.at(i.internalId() & 0x0000ffff) : 0;
		}
		
		virtual QModelIndex index(int row, int col, const QModelIndex& p) const
		{
			if ( col < 0 || col >= 2 || row < 0 )
				return QModelIndex();
			
			if ( !p.isValid() )
			{
				// top level
				if ( row < m_grid->m_calcs.count() )
					return createIndex(row, col, row & 0x0000ffff);
			} else if ( depth(p) == 0 ) {
				// prop level
				if ( row < PropCount )
					return createIndex(row, col, (p.row() & 0x0000ffff) | (1 << 16));
			} else {
				// problem...
			}
			
			return QModelIndex();
		}
		
		virtual QModelIndex parent(const QModelIndex& i) const
		{
			int idx = i.internalId() & 0x0000ffff;
			
			// if depth somehow is greater than one we're up for troubles...
			return depth(i) == 1 ? createIndex(idx, 0, idx) : QModelIndex();
		}
		
		virtual int rowCount(const QModelIndex& i) const
		{
			return i.isValid() ? (depth(i) ? 0 : PropCount) : m_grid->m_calcs.count();
		}
		
		virtual int columnCount(const QModelIndex& i) const
		{
			return 2;
		}
		
		virtual QVariant data(const QModelIndex& i, int r) const
		{
			CalcView *v = calc(i);
			
			if ( !v )
				return QVariant();
			
			Calc *c = v->calc();
			
			switch ( depth(i) )
			{
				case 0 :
					if ( r == Qt::DisplayRole )
						return i.column() ? QString() : c->name();
					
					
					break;
					
				case 1:
					if ( r == Qt::DisplayRole )
					{
						switch ( i.row() )
						{
							case RomFile:
								return i.column() ? c->romFile() : tr("ROM file");
								break;
								
							case Status:
								return i.column() ? (v->isPaused() ? tr("Paused") : tr("Running")) : tr("Status");
								break;
								
							case Visibility:
								return i.column() ? (v->isVisible() ? (v->parent() ? tr("Docked") : tr("Floating")) : tr("Hidden")) : tr("Visibility");
								break;
								
							case ExternalLink:
								return i.column() ? (v->link()->hasExternalLink() ? tr("Yes") : tr("No")) : tr("External link");
								break;
								
							case Calc2CalcLink:
								
								break;
								
							default:
								break;
						}
					}
					
					break;
					
				default:
					break;
			}
			
			return QVariant();
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

#include "calcgridmanager.moc"

/*!
	\class CalcGridManager
	\brief Widget for managing properties of CalcViews in a CalcGrid
*/

CalcGridManager::CalcGridManager(CalcGrid *g)
 : QTreeView(g), m_grid(g)
{
	//setHeaderHidden(true);
	
	m_model = new CalcTreeModel(g, this);
	
	setModel(m_model);
}
