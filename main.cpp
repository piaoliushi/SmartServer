#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QPalette>
#include <QStyleFactory>
#include <QWSServer>
#include <QtPlugin>
#include <QMessageBox>
#include "LocalConfig.h"
#include "./net/config.h"
#include "./database/ConnectionPool.h"
#include "snmp_pp.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QWSServer::setCursorVisible(false);
    //Q_IMPORT_PLUGIN(qsqlpsql)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
   //QApplication::setStyle(QStyleFactory::create("plastique"));
    QPalette pal = QApplication::palette();
    pal.setBrush(QPalette::Window,QColor(15,15,45));
    pal.setBrush(QPalette::WindowText,Qt::cyan);
    pal.setBrush(QPalette::Disabled,QPalette::WindowText,QColor(159,159,159));
    pal.setBrush(QPalette::Base,QColor(40,40,80));
    pal.setBrush(QPalette::Text,Qt::cyan);
    pal.setBrush(QPalette::AlternateBase,QColor(120,120,120));
    pal.setBrush(QPalette::ToolTipBase,QColor(255, 255, 225));
    pal.setBrush(QPalette::ToolTipText,Qt::black);
    pal.setBrush(QPalette::Button, QColor(100,100,100));
    pal.setBrush(QPalette::ButtonText,Qt::white);
    pal.setBrush(QPalette::Disabled,QPalette::ButtonText,QColor(159,159,159));
    pal.setBrush(QPalette::BrightText,Qt::white);
    pal.setBrush(QPalette::Light,QColor(120,120,120,150));
    pal.setBrush(QPalette::Midlight,QColor(90,90,90,150));
    pal.setBrush(QPalette::Mid,QColor(60,60,60));
    pal.setBrush(QPalette::Dark,QColor(30,30,30));
    pal.setBrush(QPalette::Highlight, QColor(40,40,70,150));
    //pal.setBrush(QPalette::HighlightedText, Qt::white);
    pal.setBrush(QPalette::Link, QColor(85,170,255));
    //pal.setBrush(QPalette::LinkVisited, QColor(170,100,240));


    QApplication::setPalette(pal);

    QFont font  = a.font();
    font.setPointSize(16);
    a.setFont(font);
    QString AppDir = QCoreApplication::applicationDirPath();
    AppDir.append("/ServerLocalConfig.xml");
    if(!GetInst(LocalConfig).load_local_config(AppDir.toLatin1().constData())){
        QMessageBox::information(NULL,QObject::tr("错误"),QObject::tr("加载本地配置文件失败！"));
        return -1;
    }

    DefaultLog::log()->set_filter(DEBUG_LOG, 0);
    DefaultLog::log()->set_filter(INFO_LOG, 0);
    MainWindow w;
    w.showMaximized();
    int nyet = a.exec();
    ConnectionPool::release();
    return nyet;
}
