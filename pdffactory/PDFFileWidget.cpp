#include <QtGlobal>
#include <QtGui>

#include "PDFFileWidget.h"
#include "PDFPageWidget.h"

#define COLLAPSE_BUTTON_WIDTH   60
#define COLLAPSE_BUTTON_HEIGHT  40

#define CHILD_AREA_WIDTH     150
#define CHILD_AREA_HEIGHT    150

FilesContainerWidget::FilesContainerWidget(QWidget *parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAcceptDrops(true);
    mainLayout = new QHBoxLayout();

    setLayout(mainLayout);
}

int FilesContainerWidget::getPagesCount() const {
    return pageWidgets.size();
}

QSize FilesContainerWidget::sizeHint() const {
    return QSize(CHILD_AREA_WIDTH*getPagesCount(), CHILD_AREA_HEIGHT + 20);
}

void FilesContainerWidget::addPageWidget(PDFPageWidget *pageWidget){
    pageWidgets.append(pageWidget);
    mainLayout->addWidget(pageWidget);
    adjustSize();
}

void FilesContainerWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

void FilesContainerWidget::dropEvent(QDropEvent *event) {
    int from    = event->mimeData()->text().toInt();
    int to      = findPageWidgetInLayout(pageWidgets[findPageContainingClickEvent(event->pos())]);

    mainLayout->removeWidget(pageWidgets[from]);
    mainLayout->insertWidget(to, pageWidgets[from]);

    event->acceptProposedAction();
}

void FilesContainerWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton){
        int draggedChild = (findPageContainingClickEvent(event->pos()));

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;

        mimeData->setText(QString::number(draggedChild));
        drag->setMimeData(mimeData);
        drag->setPixmap(QPixmap(":/images/copy.png"));

        Qt::DropAction dropAction = drag->exec();
    }
}

int FilesContainerWidget::findPageContainingClickEvent(QPoint pos){
    for (int i=0; i < getPagesCount(); i++)
        if (pageWidgets[i]->geometry().contains(pos))
            return i;
    return getPagesCount()-1;
}

int FilesContainerWidget::findPageWidgetInLayout(PDFPageWidget *pageWidget){
    for (int i= 0; i<getPagesCount(); i++)
        if (mainLayout->itemAt(i)->widget() == pageWidget)
            return i;
    return getPagesCount()-1;
}

// ======================================================================

PDFFileWidget::PDFFileWidget(QWidget *parent){
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    topLayout      = new QGridLayout();

    collapseButton = new QPushButton(tr("X"));
    collapseButton->setMinimumSize(QSize(COLLAPSE_BUTTON_WIDTH,COLLAPSE_BUTTON_HEIGHT));
    collapseButton->setMaximumSize(QSize(COLLAPSE_BUTTON_WIDTH,COLLAPSE_BUTTON_HEIGHT));
    connect(collapseButton, SIGNAL(released()), this, SLOT(collapsedButtonClick()));
    topLayout->addWidget(collapseButton, 0, 0);

    fileNameLabel  = new QLabel();
    fileNameLabel->setText(tr("File 1"));
    topLayout->addWidget(fileNameLabel, 0, 1);

    filesContainerWidget = new FilesContainerWidget();
    scrollArea = new QScrollArea();
    scrollArea->setWidget(filesContainerWidget);
    topLayout->addWidget(scrollArea ,1, 0, 1, 5);

    setLayout(topLayout);

    setCollapsed(false);
}

void PDFFileWidget::setCollapsed(bool state){
    collapsed = state;
    if (state == true) {
        scrollArea->hide();
    } else {
        scrollArea->show();
    }
}

void PDFFileWidget::collapsedButtonClick(){
    setCollapsed(!collapsed);
}

void PDFFileWidget::setDocument(Poppler::Document* document, QString fileName){
    int numPages = document -> numPages();
    for (int i = 0; i < numPages; i++){

        Poppler::Page *pdfPage = document->page(i);

        QImage *pageImage = new QImage();
        *pageImage = pdfPage->renderToImage(144, 144);

        PDFPageWidget *pageWidget = new PDFPageWidget();
        pageWidget->setThumbnail(pageImage);

        filesContainerWidget->addPageWidget(pageWidget);
    }
}
