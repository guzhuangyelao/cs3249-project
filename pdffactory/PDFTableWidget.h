#ifndef PDFTABLEWIDGET_H
#define PDFTABLEWIDGET_H

#include <poppler-qt4.h>

#include <QFrame>
#include <QHash>
#include "PDFPageWidget.h"

class QImage;
class QVBoxLayout;
class QString;
class QScrollArea;
class QMouseEvent;

class PDFFileWidget;

class PDFTableWidget: public QFrame
{
    Q_OBJECT
    public:
        PDFTableWidget(QWidget *parent = 0);
        void loadFile (QString fileName);
        void registerPage(PDFPageWidget *child);

    protected:
    private:
        QVBoxLayout     *outerLayout;
        QScrollArea     *scrollArea;
        QWidget         *containerWidget;
        QVBoxLayout     *containerLayout;

        QVector<Poppler::Document*> files;
        QVector<QString>            fileNames;
        QVector<PDFFileWidget *>    fileWidgets;

        QHash<QString, PDFPageWidget*> pageChilds;
    signals:
        void previewUpdate(Poppler::Page*);

private slots:
        void pageClicked(QMouseEvent*, QString path);
        void droppedPage(QString pathFrom, QString pathTo);
};
#endif
