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
    connect(fileWidget, SIGNAL(pageClicked(QMouseEvent*,QImage)), this, SIGNAL(pageClicked(QMouseEvent*,QImage)));
    connect(fileWidget, SIGNAL(previewUpdate(Poppler::Page*)), this, SIGNAL(previewUpdate(Poppler::Page*)));

    fileWidgets.append(fileWidget);

    fileNames.append(fileName);

    containerLayout->insertWidget(containerLayout->count() - 1, fileWidget);
}
