#include <QContextMenuEvent>
#include <QMenu>
#include <QtWebKit>
#include <QApplication>
#include <QClipboard>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

#include "basewebview.h"
#include "resource.h"

BaseWebView::BaseWebView(QWidget *parent) :
    QWebView(parent)
{
}

void BaseWebView::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
    QMenu *menu = new QMenu(this);
    QAction *copyAction = new QAction(QIcon(Resource::CopyIcon), tr("Copy"), menu);
    if(page()->hasSelection() && !page()->selectedText().isEmpty()){
        menu->addAction(copyAction);
        menu->addSeparator();
    }
    QAction *printAction = new QAction(QIcon(Resource::PrintLargeIcon), tr("Print..."), menu);
    menu->addAction(printAction);
    QAction *printPreviewAction = new QAction(tr("Print Preview..."), menu);
    menu->addAction(printPreviewAction);
    QAction *action = menu->exec(e->globalPos());
    if(action==copyAction){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(page()->selectedText());
    } else if(action==printAction){
        QPrinter webViewPrinter(QPrinter::HighResolution);
        QPrintDialog printDialog(&webViewPrinter, this);
        printDialog.setWindowTitle(tr("Print..."));
        if(printDialog.exec()==QDialog::Accepted)
            print(&webViewPrinter);
    } else if(action==printPreviewAction){
        QPrinter printer(QPrinter::HighResolution);
        QPrintPreviewDialog ppd(&printer, this, Qt::WindowMaximizeButtonHint);
        ppd.setWindowTitle(tr("Print Preview..."));
        connect(&ppd, SIGNAL(paintRequested(QPrinter*)), this, SLOT(print(QPrinter*)));
        ppd.showMaximized();
        ppd.exec();
    }
    menu->deleteLater();
}
