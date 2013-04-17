#include <QtGui>

#include "PDFTableWidget.h"
#include "PDFPageWidget.h"
#include "PDFFileWidget.h"
#include <QtAlgorithms>
#include "PDFJam.h"

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
    scrollArea -> setFrameStyle(QFrame::Plain);

    containerLayout = new QVBoxLayout();
    containerLayout -> setSpacing(10);
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

void PDFTableWidget::loadFile(QString fileName){
    Poppler::Document *doc = Poppler::Document::load(fileName);
    files.append(doc);

    PDFFileWidget *fileWidget = new PDFFileWidget();
    fileWidget->setAncestor(this);
    fileWidget->setDocument(doc,fileName);
    pdfJam.loadFile(fileName,files.size()-1,doc);

    fileWidgets.append(fileWidget);
    fileNames.append(fileName);

    containerLayout->insertWidget(containerLayout->count() - 1, fileWidget);
    connect(fileWidget, SIGNAL(fileRemoveButtonClicked(PDFFileWidget*)), this, SLOT(fileRemoveButtonClicked(PDFFileWidget*)));
}

QVector<PDFFileWidget*> PDFTableWidget::getSelectedFiles() {
    return selectedFiles;
}

QVector<int> PDFTableWidget::getSelectedIndices() {
    QVector<int> retVector;
    for (int i = 0; i < selectedFiles.size(); i++) {
        retVector.append(fileWidgets.indexOf(selectedFiles.at(i)));
    }
    return retVector;
}

QVector<QString> PDFTableWidget::getSelectedFileNames() {
    QVector<QString> retVector;
    QVector<int> indices = getSelectedIndices();
    for (int i = 0; i < indices.size(); i++) {
        retVector.append(fileNames.at(indices.at(i)));
    }
    return retVector;
}

void PDFTableWidget::registerPage(PDFPageWidget* child){
    //come up with a new name here
    QString name = QString("/home/pdfpage").append(QString::number(pageChilds.size()));
    qDebug()<<"Registering name:";
    qDebug()<<name;
    pageChilds[name] = child;
    child->registerName(name);
}

void PDFTableWidget::copySelected(){
    for (int i = 0;i<selectedPages.size();i++)
        copyPage(selectedPages.at(i));
}
void PDFTableWidget::cutSelected(){
    for (int i = 0;i<selectedPages.size();i++){
        cutPage(selectedPages.at(i));
        i=-1;
    }
}
void PDFTableWidget::deleteSelected(){
    for (int i = 0;i<selectedPages.size();i++)
        deletePage(selectedPages.at(i));
}

void PDFTableWidget::fileClicked(PDFFileWidget* sender, QMouseEvent* event) {
    if (event->button() == Qt::LeftButton){
        if (event->modifiers() != Qt::ControlModifier) {
            for (int i = 0; i < selectedFiles.size(); i++) {
                selectedFiles.at(i)->setSelected(false);
            }

            selectedFiles.clear();

            if (!sender->isSelected()) {
                sender->setSelected(true);
                selectedFiles.append(sender);
            }
        } else {
            if (!sender->isSelected()) {
                sender->setSelected(true);
                selectedFiles.append(sender);
            } else {
                sender->setSelected(false);
                selectedFiles.remove(selectedFiles.indexOf(sender));
            }
        }
    }
}

void PDFTableWidget::fileRemoveButtonClicked(PDFFileWidget* sender) {
    selectedFiles.remove(selectedFiles.indexOf(sender));

    QVector<int> pagesToRemove;
    for (int i = 0; i < selectedPages.size(); i++) {
        if (selectedPages.at(i)->getFather() == sender) {
            pagesToRemove.append(i);
        }
    }

    for (int i = 0; i < pagesToRemove.size(); i++) {
        selectedPages.at(pagesToRemove.at(i))->setSelected(false);
        selectedPages.remove(pagesToRemove.at(i));
    }

    // Handle remove file
    sender->hide();
}

void PDFTableWidget::pageClicked(PDFPageWidget *sender, QMouseEvent* event, QString path){
    if (event->button() == Qt::LeftButton){
        // Handle selection
        if (selectedPages.size() > 0 && event->modifiers() != Qt::ControlModifier) {

            if (sender->isSelected() ){
                QDrag *drag = new QDrag(this);
                QMimeData *mimeData = new QMimeData;

                mimeData->setText(path);
                drag->setMimeData(mimeData);
                drag->setPixmap(QPixmap(":/images/copy.png"));

                drag->exec();
            }else {
                for (int i = 0; i < selectedPages.size(); i++) {
                    selectedPages.at(i)->setSelected(false);
                }
                selectedPages.clear();
                sender->setSelected(true);

                selectedPages.insert(0,sender);
                emit previewUpdate(sender->getPage(), sender->getRotation());
            }

        } else {
            if (!sender->isSelected()) {
                sender->setSelected(true);
                emit previewUpdate(sender->getPage(), sender->getRotation());

                PDFFileWidget* senderF = (PDFFileWidget*)sender->getFather();
                int senderPID = senderF->indexChild(sender);
                int senderFID = fileWidgets.indexOf(senderF);

                int i = 0;
                for (i = 0;i<selectedPages.size();i++){
                    PDFPageWidget* target = selectedPages.at(i);
                    PDFFileWidget* targetF = (PDFFileWidget*)target->getFather();
                    int targetPID = targetF->indexChild(target);
                    int targetFID = fileWidgets.indexOf(targetF);

                    if (targetFID == senderFID && targetPID > senderPID)
                        break;

                    if (targetFID > senderFID)
                        break;
                }

                selectedPages.insert(i,sender);
            } else {
                sender->setSelected(false);
                selectedPages.remove(selectedPages.indexOf(sender));
            }
        }
    }

    if (event->button() == Qt::RightButton){
        for (int i = 0; i < selectedPages.size(); i++) {
                selectedPages.at(i)->setSelected(false);
            }
        selectedPages.clear();
    }
}

void PDFTableWidget::pageDropped(PDFPageWidget *sender, QDropEvent *event, QString pathFrom, QString pathTo){
    moveSelectedPages(pathFrom, pathTo);
}

void PDFTableWidget::moveSelectedPages(QString pathFrom, PDFPageWidget* page){
    moveSelectedPages(pathFrom, page->getName());
}

void PDFTableWidget::deletePage(PDFPageWidget* pageWidget){
    PDFFileWidget *daddy = (PDFFileWidget*)pageWidget->getFather();
    int daddyID = fileWidgets.indexOf(daddy);
    int pageID = daddy->indexChild(pageWidget);

    pdfJam.removePage(daddyID, daddy->pagesContainerWidget->pageWidgets.size(),pageID);
    daddy->removeChild(pageWidget);

    int spos = selectedPages.indexOf(pageWidget);
    if (spos!=-1){
        selectedPages.remove(spos);
    }
    spos = copiedPages.indexOf(pageWidget);
    if (spos!=-1){
        copiedPages.remove(spos);
    }
    pageChilds.remove(pageWidget->getName());

    // PLS ACTIVATE THIS LINE ONCE EVERYTHING HAS BEEN FIXED
    // :D :D :D :D :D :D
    //delete page;

}

void PDFTableWidget::cutPage(PDFPageWidget* pageWidget){
    deletePage(pageWidget);
    int id = copiedPages.size();
    copiedPages.push_back(pageWidget);

    PDFFileWidget *daddy = (PDFFileWidget*)pageWidget->getFather();
    int daddyID = fileWidgets.indexOf(daddy);
    int pageID = daddy->indexChild(pageWidget);

    pdfJam.cutPage(daddyID, daddy->pagesContainerWidget->pageWidgets.size(),pageID, id);

}

void PDFTableWidget::clearClipboard(){
    copiedPages.clear();
}

void PDFTableWidget::copyPage(PDFPageWidget* pageWidget){
    int id = copiedPages.size();
    copiedPages.push_back(pageWidget);

    PDFFileWidget *daddy = (PDFFileWidget*)pageWidget->getFather();
    int daddyID = fileWidgets.indexOf(daddy);
    int pageID = daddy->indexChild(pageWidget);

    pdfJam.copyPage(daddyID, pageID, id);
}

void PDFTableWidget::pastePage(PDFFileWidget* fileWidget, int pageID){
    if (copiedPages.size() > 0){
        for (int i = 0;i<copiedPages.size(); i++){
            PDFPageWidget* page =copiedPages.at(i);

            PDFPageWidget* pageWidget = new PDFPageWidget();
            pageWidget->setAncestor(this);
            pageWidget->setFather(page->getFather());
            pageWidget->setPopplerPage(page->getPage());

            int fileID = fileWidgets.indexOf(fileWidget);
            pdfJam.pastePage(fileID, fileWidget->pagesContainerWidget->pageWidgets.size(), pageID, i);

            fileWidget->insertChildAt(pageWidget,pageID+i);
        }
    }
}

void PDFTableWidget::rotatePage(PDFPageWidget* pageWidget) {
    pageWidget->rotate90();

    int fileIndex = fileWidgets.indexOf((PDFFileWidget *)pageWidget->getFather());
    int pageIndex = fileWidgets.at(fileIndex)->indexChild(pageWidget);
    pdfJam.rotatePage(fileIndex, pageIndex, 90);

    emit checkPreviewUpdate(pageWidget->getPage(), pageWidget->getRotation());
}

void PDFTableWidget::rotateSelectedPages() {
    for (int i = 0; i < selectedPages.size(); i++) {
        rotatePage(selectedPages.at(i));
    }
}

void PDFTableWidget::moveSelectedPages(QString pathFrom, QString pathTo){
    if (selectedPages.size() == 0)
        return;

    bool accept = false;
    for (int i = 0;i<selectedPages.size();i++)
        if (selectedPages.at(i)->getName() == pathFrom)
            accept=true;
    if (!accept)
        return;
/*
    for (int i = 0;i<selectedPages.size();i++)
        if (selectedPages.at(i)->getName() == pathTo)
            return;*/

    PDFPageWidget* childTo  = pageChilds[pathTo];
    PDFFileWidget* fileTo   = (PDFFileWidget*) childTo->getFather();

    int posTo=  -1;
    PDFFileWidget* file = NULL;
    for (int i = selectedPages.size() - 1; i>=0 ;i--){
        PDFPageWidget* childFrom    = selectedPages[i];
        PDFFileWidget* fileFrom     = (PDFFileWidget*) childFrom->getFather();
        if (file == fileFrom){
            posTo-=1;
        }

        if (pathTo==childFrom->getName() ){
            file = fileFrom;
            posTo = fileFrom->indexChild(childFrom);
        }


        pdfJam.cutPage(
                fileWidgets.indexOf(fileFrom),
                fileFrom->pagesContainerWidget->pageWidgets.size(),
                fileFrom->indexChild(childFrom),
                i
                );

        fileFrom->removeChild(childFrom);

    }
    if (posTo ==-1)
        posTo               = fileTo->indexChild(childTo);
    for (int i = selectedPages.size() - 1; i>=0 ;i--){
        pdfJam.pastePage(
                fileWidgets.indexOf(fileTo),
                fileTo->pagesContainerWidget->pageWidgets.size(),
                posTo,
                i
                );

        PDFPageWidget* childFrom    = selectedPages[i];
        PDFFileWidget* fileFrom     = (PDFFileWidget*) childFrom->getFather();
        fileTo->insertChildAt(childFrom, posTo);

                //fileFrom->insertChildAt(childTo, posFrom);
    }
}
