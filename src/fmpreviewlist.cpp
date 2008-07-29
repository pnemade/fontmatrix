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
#include "fmpreviewlist.h"

#include "typotek.h"
#include "fontitem.h"
#include "mainviewwidget.h"

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QScrollBar>
#include <QDesktopWidget>


FMPreviewList::FMPreviewList(QWidget* parent)
 : QGraphicsView(parent)
{
	m_scene = new QGraphicsScene;
	setScene(m_scene);
// 	m_scene->setBackgroundBrush(Qt::lightGray);
	m_select = m_scene->addRect(QRectF(), QPen(Qt::transparent ), QColor(255,216,158,100));
	m_select->setZValue(100.0);
	
	m_currentItem = 0;
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
	
	theWord = typotek::getInstance()->word();
	slotPleaseMakeItGoddLooking();
	
	connect( verticalScrollBar(), SIGNAL(valueChanged( int )), this, SLOT(slotChanged()) );
	connect( verticalScrollBar(), SIGNAL(sliderReleased()),this,SLOT(slotChanged()));
	
	connect( typotek::getInstance(),SIGNAL(previewDirectionHasChanged()),this,SLOT(slotPleaseMakeItGoddLooking()));
	
}


FMPreviewList::~FMPreviewList()
{
}

void FMPreviewList::slotRefill(QList<FontItem*> fonts, bool setChanged)
{
// 	qDebug()<<"FMPreviewList::slotRefill("<< fonts.count() <<","<< setChanged <<")";
	theWord = typotek::getInstance()->word();
	horizontalScrollBar()->setValue(0);
	QColor oddC(255,255,255);
	QColor evenC(242,242,242);
	bool colState (false);
	if(setChanged/* && !fonts.isEmpty()*/) 
	{
		trackedFonts.clear();
		
		QMap<QString, QMap<QString, FontItem*> > keyList;
		QMap<QString, QString> realFamilyName;
		QMap<int, QChar> initChars;
		for ( int i=0; i < fonts.count();++i )
		{
			QString family = fonts[i]->family();
			QString ordFamily = family.toUpper();
			QString variant = fonts[i]->variant();
			if( keyList.contains(ordFamily) && keyList[ordFamily].contains(variant) )
			{
				int unique = 2;
				QString uniString(variant +" -%1");
				while(keyList[ordFamily].contains(uniString.arg(unique,2)))
				{
					++unique;
				}
				variant = uniString.arg(unique,2);
			}
			keyList[ordFamily][variant] = ( fonts[i] );
			realFamilyName[ordFamily] = family;
			initChars[ordFamily[0].unicode()] = ordFamily[0] ;
		}
		QMap<QString, QMap<QString, FontItem*> >::const_iterator kit;
		QMap<QString, FontItem*>::const_iterator kit_value;
		QMap<int, QChar>::const_iterator ic = initChars.constBegin();
		while (  ic != initChars.constEnd() )
		{
			QChar firstChar ( ic.value() );
			for ( kit = keyList.begin(); kit != keyList.end(); ++kit )
			{
				if ( kit.key().at ( 0 ) == firstChar )
				{
					for(kit_value = kit.value().begin(); kit_value != kit.value().end(); ++kit_value)
					{
						trackedFonts << kit_value.value();
					}
				}
			}
			++ic;
		}
	}
	
	for(QMap<QString, FontPreviewItem>::const_iterator pit = m_pixItemList.begin() ; pit!= m_pixItemList.end(); ++pit)
	{
		if(pit.value().item)
		{
			m_scene->removeItem(pit.value().item);
			delete pit.value().item;
// 			qDebug()<<"remove from preview : "<< pit.value().name;
		}
	}
	
// 	qDebug()<<"PreviewScene contains now : "<< m_scene->items().count() << " items";
	
	m_pixItemList.clear();
	
	double theWidth = width();
	double theSize = typotek::getInstance()->getPreviewSize();
	double theLine = 1.3 * theSize * QApplication::desktop()->physicalDpiY() / 72.0;
	double indent = 0;
	
	QRect vvrect(visibleRegion().boundingRect());
	QRect vrect = mapToScene( vvrect ).boundingRect().toRect();
	int beginPos= vrect.top();
	int endPos = vrect.bottom();
	if(beginPos <0)
	{
		endPos += beginPos * -1;
		beginPos = 0;
	}
	
	m_scene->setSceneRect(0,0, width(), trackedFonts.count() * theLine);
	double visibilityAdjustment = theLine / 2.0;
	QString lastFamily;
	for(int i = 0 ; i < trackedFonts.count() ; ++i)
	{
		if(trackedFonts[i]->family() != lastFamily)
		{
			colState = !colState;
			lastFamily = trackedFonts[i]->family();
		}
		if( ((i + 1)*theLine) > (beginPos + visibilityAdjustment) && (i*theLine)  < (endPos - visibilityAdjustment))
		{ 
			FontItem *fit = trackedFonts.at(i);
			if(fit)
			{
				bool oldRaster = fit->rasterFreetype();
				fit->setFTRaster("true");
				QGraphicsPixmapItem *pit = m_scene->addPixmap(fit->oneLinePreviewPixmap(theWord,(colState ? oddC : evenC ), theWidth ));
				fit->setFTRaster(oldRaster);
				pit->setPos(indent ,theLine*i);
				pit->setData(1,fit->path());
				pit->setData(2,"preview");
				pit->setToolTip(fit->fancyName());
				pit->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
				m_pixItemList[fit->path()] = FontPreviewItem(fit->path(), QPointF(indent , theLine*i), true, pit) ;
				
			}
		}
		else
		{
			m_pixItemList[trackedFonts.at(i)->path()] = FontPreviewItem(trackedFonts.at(i)->path(),QPointF(0,theLine*i),false,0);
		}
	}
	slotPleaseMakeItGoddLooking();
}

void FMPreviewList::showEvent(QShowEvent * event)
{
	slotRefill(mvw->curFonts(), false);
}

void FMPreviewList::slotChanged( )
{
// 	qDebug()<<"FMPreviewList::slotChanged( )";
#ifndef HIGH_PERF
	// It waits you release the slider to draw items
	if(verticalScrollBar()->isSliderDown())
		return;
#endif
	slotRefill(trackedFonts, false);
}

void FMPreviewList::mousePressEvent(QMouseEvent * e)
{
// 	qDebug() << "FMPreviewList::mousePressEvent(QMouseEvent * "<<e<<")";
	
	QPointF inListPos = mapToScene( e->pos() );
	inListPos.rx() = 1;
	QList<QGraphicsItem*> items = m_scene->items(inListPos);
	
	if(items.isEmpty())
	{
		qDebug() << "\t No item under  "<<e->pos()<<")";
		return;
	}
	QGraphicsItem* it = 0;;
	for(int i = 0; i < items.count();++i)
	{
		if(items[i]->data(2).toString() == "preview")
		{
			it = items[i];
			break;
		}
	}
	if(it == 0)
		return;
	
	if(it == m_currentItem)
		return;
	
	QString fontname(it->data(1).toString());
	slotSelect(fontname);
	if(isVisible())
		mvw->slotFontSelectedByName(fontname);
	
}

void FMPreviewList::slotSelect(QString fontname)
{
	qDebug() << "FMPreviewList::slotSelect(QGraphicsItem * "<<fontname<<")";
	if(fontname.isEmpty())
		return;
	curFontName = fontname;
	FontPreviewItem it = m_pixItemList[fontname];
// 	qDebug() << it.dump();
	
	if(!it.visible)
	{
		double topShift = 0.0;
		verticalScrollBar()->setValue(it.pos.y() + topShift);
	}
// 	ensureVisible(QRectF(0,it.pos.y(), width(), height()));
// 	qDebug() << "scroll to " << it.pos.y() << "and it was " <<(it.pos.x() == 0 ? "invisible" : "visible");
	
	if(!m_pixItemList.contains(fontname))
	{
		qDebug()<< "m_pixItemList does not contain " << fontname;
		return;
	}
	FontPreviewItem nit = m_pixItemList[fontname];
// 	qDebug()<< nit.dump();
	if(!nit.item)
	{
		qDebug()<<"OOPS, the graphic item has not been properly instanciated "<<nit.item;
		return;
	}
	nit.item->setSelected(true);
	m_currentItem = nit.item;
// 	double theSize = typotek::getInstance()->getPreviewSize();
// 	double theLine = 1.4 * theSize * QApplication::desktop()->physicalDpiY() / 72.0;
// 	
	QRectF itRect;
	QPointF itPos ( nit.pos );
	itRect.setWidth(1000);
	itRect.setHeight(nit.item->boundingRect().height());
	itPos.rx() = mapToScene(0.0,0.0).x();
	
	m_select->setRect(itRect);
	m_select->setPos(itPos);
	
}

void FMPreviewList::slotClearSelect()
{
// 	qDebug()<<" FMPreviewList::slotClearSelect()";
	m_currentItem = 0;
	m_select->setRect(QRectF());
	verticalScrollBar()->setValue(0);
}

void FMPreviewList::resizeEvent(QResizeEvent * event)
{
	slotRefill(trackedFonts, false);
}

void FMPreviewList::keyPressEvent(QKeyEvent * e)
{
	QString ref;
	if( mvw->selectedFont())
		ref = mvw->selectedFont()->path() ;
	else
	{
		slotSelect(trackedFonts[0]->path());
		return;
	}
	
	if(e->key() == Qt::Key_Up)
	{
// 		verticalScrollBar()->setValue(verticalScrollBar()->value() - 32);
		
		QString target;
		for(int i = 1; i < trackedFonts.count(); ++i)
		{
			if(trackedFonts[i]->path() == ref)
			{
				target = trackedFonts[i-1]->path();
				break;
			}
		}
		if(!target.isEmpty())
		{
			slotSelect(target);
			mvw->slotFontSelectedByName(target);
		}
		
	}
	else if(e->key() == Qt::Key_Down)
	{
// 		verticalScrollBar()->setValue(verticalScrollBar()->value() + 32);	
		QString target;
		for(int i = 0; i < trackedFonts.count() - 1; ++i)
		{
			if(trackedFonts[i]->path() == ref)
			{
				target = trackedFonts[i+1]->path();
				break;
			}
		}
		if(!target.isEmpty())
		{
			slotSelect(target);
			mvw->slotFontSelectedByName(target);
		}
	}
}

void FMPreviewList::slotPleaseMakeItGoddLooking()
{
// 	typotek *t = typotek::getInstance();
// 	if(t->getPreviewRTL())
// 	{
// // 		qDebug()<< "Change alignment of preview list to RIGHT";
// 		setSceneRect( 0 ,0 ,t->getPreviewSize() * t->word().count() * QApplication::desktop()->physicalDpiX() / 72.0 * 0.8, m_scene->height() );
// // 		qDebug()<< m_scene->sceneRect();
// 		horizontalScrollBar()->setValue(m_scene->width());
// 		setAlignment( Qt::AlignRight |  Qt::AlignTop );
// 	}
// 	else
// 	{
// 		qDebug()<< "Change alignment of preview list to LEFT";
// 		setSceneRect( 0 ,0 ,t->getPreviewSize() * t->word().count(), m_scene->height() );
		setAlignment( Qt::AlignLeft |  Qt::AlignTop );
// 	}
	
}




FMPreviewModel::FMPreviewModel( QObject * pa , QListView * wPa )
	: QAbstractListModel(pa) , m_view(wPa)
{
}

QVariant FMPreviewModel::data(const QModelIndex & index, int role) const
{	
	if(!index.isValid())
		return QVariant();

	int row = index.row();
// 	qDebug()<<"D"<<row;
	FontItem *fit(typotek::getInstance()->getCurrentFonts().at(row));
	if(!fit)
		return QVariant();
	
	QColor bgColor(255,255,255);
	int width( m_view->width() );
	
	if(role == Qt::DisplayRole)
	{
		if( typotek::getInstance()->getPreviewSubtitled() )
			return fit->fancyName() ;
		else
			return QVariant();
	}
	else if(role == Qt::DecorationRole)
	{
			return QIcon( fit->oneLinePreviewPixmap(typotek::getInstance()->word(), bgColor, width ) );
	}
	
	// fall back
	return QVariant();
	
}

Qt::ItemFlags FMPreviewModel::flags(const QModelIndex & index) const
{
	return (Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}

int FMPreviewModel::rowCount(const QModelIndex & parent) const
{
	if(parent.isValid())
		return 0;
	QList< FontItem * > cl(typotek::getInstance()->getCurrentFonts());
	return cl.count();
}

void FMPreviewModel::dataChanged()
{
	emit layoutChanged();
}

FMPreviewView::FMPreviewView(QWidget * parent)
	:QListView(parent)
{
}

void FMPreviewView::resizeEvent(QResizeEvent * event)
{
	emit widthChanged(this->width());
}
