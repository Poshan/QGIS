/***************************************************************************
                         qgsdxfexport.h
                         --------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFEXPORT_H
#define QGSDXFEXPORT_H

#include "qgsgeometry.h"
#include "qgssymbolv2.h"
#include <QColor>
#include <QList>
#include <QTextStream>

class QgsMapLayer;
class QgsPoint;
class QgsSymbolLayerV2;
class QIODevice;

class CORE_EXPORT QgsDxfExport
{
  public:
    enum SymbologyExport
    {
      NoSymbology = 0, //export only data
      FeatureSymbology, //Keeps the number of features and export symbology per feature (using the first symbol level)
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    QgsDxfExport();
    QgsDxfExport( const QgsDxfExport& dxfExport );
    ~QgsDxfExport();
    QgsDxfExport& operator=( const QgsDxfExport& dxfExport );

    void addLayers( QList< QgsMapLayer* >& layers ) { mLayers = layers; }
    int writeToFile( QIODevice* d );  //maybe add progress dialog? //other parameters (e.g. scale, dpi)?

    void setSymbologyScaleDenominator( double d ) { mSymbologyScaleDenominator = d; }
    double symbologyScaleDenominator() const { return mSymbologyScaleDenominator; }

    void setMapUnits( QGis::UnitType u ) { mMapUnits = u; }
    QGis::UnitType mapUnits() const { return mMapUnits; }

    void setSymbologyExport( SymbologyExport e ) { mSymbologyExport = e; }
    SymbologyExport symbologyExport() const { return mSymbologyExport; }

    void setExtent( const QgsRectangle& r ) { mExtent = r; }
    QgsRectangle extent() const { return mExtent; }

    //get closest entry in dxf palette
    static int closestColorMatch( QRgb pixel );

    void writeGroup( int code, int i );
    void writeGroup( int code, double d );
    void writeGroup( int code, const QString& s );
    void writeGroupCode( int code );
    void writeInt( int i );
    void writeDouble( double d );
    void writeString( const QString& s );

    //draw dxf primitives
    void writePolyline( const QgsPolyline& line, const QString& layer, const QString& lineStyleName, int color,
                        double width = -1, bool polygon = false );

    void writeSolid( const QString& layer, int color, const QgsPoint& pt1, const QgsPoint& pt2, const QgsPoint& pt3, const QgsPoint& pt4 );

    //write line (as a polyline)
    void writeLine( const QgsPoint& pt1, const QgsPoint& pt2, const QString& layer, const QString& lineStyleName, int color, double width = -1 );

    void writePoint( const QString& layer, int color, const QgsPoint& pt );

    void writeCircle( const QString& layer, int color, const QgsPoint& pt, double radius );

  private:

    QList< QgsMapLayer* > mLayers;
    /**Extent for export, only intersecting features are exported. If the extent is an empty rectangle, all features are exported*/
    QgsRectangle mExtent;
    /**Scale for symbology export (used if symbols units are mm)*/
    double mSymbologyScaleDenominator;
    SymbologyExport mSymbologyExport;
    QGis::UnitType mMapUnits;

    QTextStream mTextStream;

    static double mDxfColors[][3];

    int mSymbolLayerCounter; //internal counter
    int mNextHandleId;
    int mBlockCounter;

    QHash< const QgsSymbolLayerV2*, QString > mLineStyles; //symbol layer name types
    QHash< const QgsSymbolLayerV2*, QString > mPointSymbolBlocks; //reference to point symbol blocks

    //AC1009
    void writeHeader();
    void writeTables();
    void writeBlocks();
    void writeEntities();
    void writeEntitiesSymbolLevels( QgsVectorLayer* layer );
    void writeEndFile();

    void startSection();
    void endSection();

    void writePoint( const QgsPoint& pt, const QString& layer, int color, const QgsFeature* f, const QgsSymbolLayerV2* symbolLayer, const QgsSymbolV2* symbol );
    void writeVertex( const QgsPoint& pt, const QString& layer );
    void writeDefaultLinestyles();
    void writeSymbolLayerLinestyle( const QgsSymbolLayerV2* symbolLayer );
    void writeLinestyle( const QString& styleName, const QVector<qreal>& pattern, QgsSymbolV2::OutputUnit u );

    //AC1018
    void writeHeaderAC1018( QTextStream& stream );
    void writeTablesAC1018( QTextStream& stream );
    void writeEntitiesAC1018( QTextStream& stream );
    void writeEntitiesSymbolLevelsAC1018( QTextStream& stream, QgsVectorLayer* layer );
    void writeSymbolLayerLinestyleAC1018( QTextStream& stream, const QgsSymbolLayerV2* symbolLayer );
    void writeLinestyleAC1018( QTextStream& stream, const QString& styleName, const QVector<qreal>& pattern, QgsSymbolV2::OutputUnit u );
    void writeVertexAC1018( QTextStream& stream, const QgsPoint& pt );
    void writePolylineAC1018( QTextStream& stream, const QgsPolyline& line, const QString& layer, const QString& lineStyleName, int color,
                              double width = -1, bool polygon = false );


    QgsRectangle dxfExtent() const;

    void addFeature( const QgsFeature& fet, const QString& layer, const QgsSymbolLayerV2* symbolLayer, const QgsSymbolV2* symbol );
    double scaleToMapUnits( double value, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits ) const;

    //returns dxf palette index from symbol layer color
    static int colorFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer );
    double widthFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer ) const;
    QString lineStyleFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer );

    //functions for dxf palette
    static int color_distance( QRgb p1, int index );
    static QRgb createRgbEntry( qreal r, qreal g, qreal b );

    //helper functions for symbology export
    QgsRenderContext renderContext() const;
    void startRender( QgsVectorLayer* vl ) const;
    void stopRender( QgsVectorLayer* vl ) const;
    static double mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );
    QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > > symbolLayers();
    static int nLineTypes( const QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2*> >& symbolLayers );
    static bool hasDataDefinedProperties( const QgsSymbolLayerV2* sl, const QgsSymbolV2* symbol );
    double dashSize() const;
    double dotSize() const;
    double dashSeparatorSize() const;
    double sizeToMapUnits( double s ) const;
    static QString lineNameFromPenStyle( Qt::PenStyle style );
    static QString dxfLayerName( const QString& name );
};

#endif // QGSDXFEXPORT_H
