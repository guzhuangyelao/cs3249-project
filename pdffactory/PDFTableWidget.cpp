#include <QtGui>

#include "PDFTableWidget.h"
#include "PDFPageWidget.h"
#include "PDFFileWidget.h"

// Constructor
PDFTableWidget::PDFTableWidget(QWidget* parent) : QFrame(parent)
{
    // Frame (Expanding with VBox) - Scroll Area (Expanding) - Container (Expanding with VBox) - Children

    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    outerLayout = new QVBoxLayout();
    outerLayout->setContentsMargins(0,0,0,0);

    scrollArea = new QScrollArea();
    scrollArea -> setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea -> setWidgetResizable(true);

    containerLayout = new QVBoxLayout();
    containerWidget = new QWidget();
    containerWidget -> setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QWidget *spacer = new QWidget();
    spacer -> setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    containerLayout -> addWidget(spacer);
    containerWidget -> setLayout(containerLayout);

    scrollArea -> setWidget(containerWidget);

    outerLayout -> addWidget(scrollArea);

    setLayout(outerLayout);
}

void PDFTableWidget::loadFile (QString fileName){
    Poppler::Document *doc = Poppler::Document::load(fileName);
    files.append(doc);

    PDFFileWidget *fileWidget = new PDFFileWidget();
    fileWidget->setAncestor(this);
    fileWidget->setDocument(doc,fileName);
    pdfJam.loadFile(fileName,files.size()-1,doc->numPages());

    fileWidgets.append(fileWidget);

    fileNames.append(fileName);

    containerLayout->insertWidget(containerLayout->count() - 1, fileWidget);
}

void PDFTableWidget::registerPage(PDFPageWidget* child){
    //come up with a new name here
    QString name = QString("/home/pdfpage").append(QString::number(pageChilds.size()));
    qDebug()<<"Registering name:";
    qDebug()<<name;
    pageChilds[name] = child;
    child->registerName(name);
}

void PDFTableWidget::pageClicked(QMouseEvent* event, QString path){
    if (event->button() == Qt::LeftButton){
        //left click -> start dragging

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(path);
        drag->setMimeData(mimeData);
        drag->setPixmap(QPixmap(":/images/copy.png"));

        Qt::DropAction dropAction = drag->exec();
    }
}
void PDFTableWidget::droppedPage(QString pathFrom, QString pathTo){
    // Page was dragged and dropped
    // Handle backend operations here

    // Handle frontend operations here
    //
    if (pathFrom == pathTo)
        return;
    qDebug() << pathFrom;
    qDebug() << pathTo;
    PDFPageWidget* childFrom    = pageChilds[pathFrom];
    PDFPageWidget* childTo      = pageChilds[pathTo];

    PDFFileWidget* fileFrom     = (PDFFileWidget*) childFrom->getFather();
    PDFFileWidget* fileTo       = (PDFFileWidget*) childTo->getFather();

    int posFrom = fileFrom->removeChild(childFrom);
    int posTo   = fileTo->removeChild(childTo);

    fileTo->insertChildAt(childFrom, posTo);
    fileFrom->insertChildAt(childTo, posFrom);
}

