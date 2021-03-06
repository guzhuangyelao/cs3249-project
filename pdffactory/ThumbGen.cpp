#include <QtGlobal>
#include <QtGui>
#include "ThumbGen.h"


#include "PDFPageWidget.h"
ThumbGen::ThumbGen()
{

}

double ThumbGen::calcDpi(Poppler::Page *pdfPage, QSize targetSize){
    double dpi;
    QSizeF oriSize = pdfPage->pageSizeF();
    double oriDpi = 72;
    if (targetSize.width() > targetSize.height()){
        dpi = targetSize.height()/(float)oriSize.height()*72;
    }else {
        dpi= targetSize.width()/(float)oriSize.width()*72;
    }
    return dpi;

}

void ThumbGen::run(){
    while(pWidgets.size()!=0){
        mutex.lock();
        PDFPageWidget *pw = pWidgets.first();
        pWidgets.pop_front();
        Poppler::Page *pp = pPages.first();
        pPages.pop_front();
        mutex.unlock();

        double dpi=calcDpi(pp,pw->size());
        QImage pageImage = pp->renderToImage(dpi, dpi);
        emit updateThumbnail(pageImage,pw);
    }
}
void ThumbGen::render(PDFPageWidget* pw,Poppler::Page* pp){
    mutex.lock();
    pWidgets.append(pw);
    pPages.append(pp);
    mutex.unlock();

}

