/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
 *   pierre@oep-h.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FMPREVIEWLIST_H
#define FMPREVIEWLIST_H

#include <QGraphicsView>
#include <QMap>

#include <QListView>
#include <QAbstractListModel>

class QGraphicsPixmapItem;
class QGraphicsScene;
class FontItem;
class MainViewWidget;
class QGraphicsRectItem;
class QGraphicsItem;
class QListView;

struct FontPreviewItem
{
	QString name;
	QPointF pos;
	bool visible;
	QGraphicsPixmapItem* item;
	FontPreviewItem(){};
	FontPreviewItem(QString n, QPointF p, bool v, QGraphicsPixmapItem* i)
	:name(n), pos(p), visible(v), item(i) {};
	
// 	QString dump(){return QString("::::::::::%1::\n\tpos(%2)\n\tvisible(%3)\n\titem(%4)").arg(name).arg(pos.y()).arg(visible).arg((int)item);}
};

/**
	@author Pierre Marchand <pierre@oep-h.com>
	@brief FMPreviewList, as QGraphicsView descendant, is a facility to manage the preview list with QtDesigner
*/
class FMPreviewList : public QGraphicsView
{
		Q_OBJECT
	public:
		FMPreviewList ( QWidget* parent);

		~FMPreviewList();
		void setRefWidget(MainViewWidget* m){mvw = m;};
// 		void searchAndSelect(QString fname);
		
		
	public slots:
		void slotRefill ( QList<FontItem*> fonts , bool setChanged );
		void slotSelect ( QString fontname );
		void slotClearSelect();
		void slotPleaseMakeItGoddLooking();
	private slots:
		void slotChanged();
		
	private:
		
		QList<FontItem*> trackedFonts;
		QGraphicsScene *m_scene;
		QMap<QString, FontPreviewItem> m_pixItemList;
		QGraphicsRectItem* m_select;
		MainViewWidget *mvw;
		QGraphicsItem *m_currentItem;
		QString theWord;
		QString curFontName;
		
	protected:
		void showEvent ( QShowEvent * event ) ;
		void mousePressEvent ( QMouseEvent * e );
		void resizeEvent ( QResizeEvent * event );
		void keyPressEvent ( QKeyEvent * e );
};

/**
	It’s time to setup a proper Model/view for this list.
*/

class FMPreviewModel : public QAbstractListModel
{
	public:
		FMPreviewModel ( QObject * pa , QListView * wPa);
		//returns a preview
		QVariant data ( const QModelIndex &index, int role = Qt::DisplayRole ) const;
		//returns flags for items
		Qt::ItemFlags flags ( const QModelIndex &index ) const;
		//returns the number of items
		int rowCount ( const QModelIndex &parent ) const;
		
		void dataChanged();
		
	private:
		QListView *m_view;
};

class FMPreviewView : public QListView
{
	Q_OBJECT
	public:
		FMPreviewView(QWidget * parent = 0);
		~FMPreviewView(){};
		
	protected:
		void resizeEvent ( QResizeEvent * event );
	signals:
		void widthChanged(int);
};

#endif
