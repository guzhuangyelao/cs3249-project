#include <QtGlobal>
#include <QtGui>

#include "PDFFileWidget.h"
#include "PDFPageWidget.h"

#define COLLAPSE_BUTTON_WIDTH   32
#define COLLAPSE_BUTTON_HEIGHT  32

#define CHILD_AREA_WIDTH     150
#define CHILD_AREA_HEIGHT    150
#define CHILD_AREA_SIDE_MARGIN 20

PagesContainerWidget::PagesContainerWidget(QWidget *parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAcceptDrops(true);
    mainLayout = new QHBoxLayout();

    setLayout(mainLayout);
}

int PagesContainerWidget::getPagesCount() const {
    return pageWidgets.size();
}

QSize PagesContainerWidget::sizeHint() const {
    return QSize((CHILD_AREA_SIDE_MARGIN + CHILD_AREA_WIDTH)*getPagesCount(), CHILD_AREA_HEIGHT + 20);
}

void PagesContainerWidget::addPageWidget(PDFPageWidget *pageWidget){
    pageWidgets.append(pageWidget);
    mainLayout->addWidget(pageWidget);
    adjustSize();
}

void PagesContainerWidget::dragEnterEvent(QDragEnterEvent *event) {
    event->acceptProposedAction();
}

void PagesContainerWidget::dropEvent(QDropEvent *event) {
   QPoint pos = event->pos();
   qDebug() << "FILE DROP";
   qDebug() << (pos.x() / (CHILD_AREA_SIDE_MARGIN + CHILD_AREA_WIDTH));
    event->acceptProposedAction();
}
// ======================================================================

PDFFileWidget::PDFFileWidget(QWidget *parent):QFrame(parent){
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    topLayout      = new QGridLayout();

    collapseButton = new QPushButton();
    collapseButton->resize(QSize(COLLAPSE_BUTTON_WIDTH,COLLAPSE_BUTTON_HEIGHT));
    collapseButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    collapseButton->setIcon(QIcon(":/images/collapse.png"));
    connect(collapseButton, SIGNAL(released()), this, SLOT(collapsedButtonClick()));
    topLayout->addWidget(collapseButton, 0, 0);

    fileNameLabel = new QLabel();
    topLayout->addWidget(fileNameLabel, 0, 1);

    pagesContainerWidget = new PagesContainerWidget();
    scrollArea = new QScrollArea();
    scrollArea->setWidget(pagesContainerWidget);
    topLayout->addWidget(scrollArea, 1, 0, 1, 5);

    setLayout(topLayout);

    setCollapsed(false);

    connect(&tgen, SIGNAL(updateThumbnail(QImage,PDFPageWidget*)), this, SLOT(updateThumbnail(QImage,PDFPageWidget*)));

}

void PDFFileWidget::updateThumbnail(QImage img,PDFPageWidget* pw){
    pw->setThumbnail(img);
}

void PDFFileWidget::setCollapsed(bool state){
    collapsed = state;
    if (state == true) {
        scrollArea->hide();
        collapseButton->setIcon(QIcon(":/images/expand.png"));
    } else {
        scrollArea->show();
        collapseButton->setIcon(QIcon(":/images/collapse.png"));
    }
}

void PDFFileWidget::collapsedButtonClick(){
    setCollapsed(!collapsed);
}

void PDFFileWidget::pageCLickedHandler(QMouseEvent*, QImage){

}

void PDFFileWidget::setDocument(Poppler::Document* document, QString fileName){
    document->setRenderHint(Poppler::Document::TextAntialiasing);
    Poppler::Document *doc = Poppler::Document::load(fileName);
    doc->setRenderHint(Poppler::Document::TextAntialiasing);

    int numPages = document -> numPages();
    for (int i = 0; i < numPages; i++){
        Poppler::Page *pdfPage = document->page(i);

        PDFPageWidget *pageWidget = new PDFPageWidget();

        pageWidget->setAncestor(ancestor);
        pageWidget->setFather(this);
        pageWidget->setPopplerPage(doc->page(i));
        tgen.render(pageWidget,pdfPage);



        pagesContainerWidget->addPageWidget(pageWidget);
    }
        tgen.start();

    fileNameLabel->setText(fileName);
}

int PDFFileWidget::removeChild(PDFPageWidget* child){
    int pos = pagesContainerWidget->pageWidgets.indexOf(child);
    pagesContainerWidget->pageWidgets.remove(pos);
    pagesContainerWidget->mainLayout->removeItem(pagesContainerWidget->mainLayout->itemAt(pos));

    return pos;
}
void PDFFileWidget::insertChildAt(PDFPageWidget* child, int pos){
    child->setFather(this);
    pagesContainerWidget->mainLayout->insertWidget(pos, child);
    pagesContainerWidget->pageWidgets.insert(pos,child);
}
