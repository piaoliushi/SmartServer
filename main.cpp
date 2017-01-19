#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QPalette>
#include <QStyleFactory>
#include <QWSServer>
#include <QtPlugin>
#include <QMessageBox>
#include <QTranslator>
#include "LocalConfig.h"
#include "./net/config.h"
#include "./database/ConnectionPool.h"
#include "./snmp_pp/snmp_pp.h"
#ifdef Q_OS_WIN
#include <QSystemTrayIcon>
#include <QDebug>

#endif
#include "./signalApp/qtsingleapplication.h"
#ifdef Q_OS_WIN
//#include "vld.h"
#endif


void utf8ToGb2312(std::string& strUtf8)
{
    QTextCodec* utf8Codec= QTextCodec::codecForName("utf-8");
    QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");

    QString strUnicode= utf8Codec->toUnicode(strUtf8.c_str());
    QByteArray ByteGb2312= gb2312Codec->fromUnicode(strUnicode);

    strUtf8= ByteGb2312.data();
}
void gb2312ToUtf8(std::string& strGb2312)
{

    QTextCodec* utf8Codec= QTextCodec::codecForName("utf-8");
    QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");

    QString strUnicode= gb2312Codec->toUnicode(strGb2312.c_str());
    QByteArray ByteUtf8= utf8Codec->fromUnicode(strUnicode);

    strGb2312= ByteUtf8.data();
}


int main(int argc, char *argv[])
{
//#ifdef Q_OS_WIN
    QtSingleApplication a(argc, argv);
    if(a.sendMessage(QString("show app")))
        return 0;
//#else
//    QApplication a(argc, argv);
//#endif


#ifndef Q_OS_WIN
#ifdef ARM_LINUX_DEF
    QWSServer::setCursorVisible(false);
    Q_IMPORT_PLUGIN(qsqlpsql)
#endif
#endif


//#ifdef Q_OS_WIN
    /*QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());*/
//#else
//#endif
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));



    QApplication::setStyle(QStyleFactory::create("plastique"));
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
    pal.setBrush(QPalette::HighlightedText, Qt::white);
    pal.setBrush(QPalette::Link, QColor(85,170,255));
    //pal.setBrush(QPalette::LinkVisited, QColor(170,100,240));


    QString AppDir = QCoreApplication::applicationDirPath();
    std::string sAppDir = AppDir.toStdString();
    utf8ToGb2312(sAppDir);

    QTranslator qtTranslator;
    QString sTranspath = AppDir + "/SmartServer_CN.qm";
    if(qtTranslator.load(sTranspath)==true)
        a.installTranslator(&qtTranslator);

    QTranslator sys_translator;
    sys_translator.load("qt_zh_CN.qm");
    a.installTranslator(&sys_translator);

    QApplication::setPalette(pal);
    QFont font  = a.font();
#ifdef Q_OS_WIN
    font.setPointSize(12);
#else
    font.setPointSize(16);
#endif
    a.setFont(font);
    //AppDir.append("/ServerLocalConfig.xml");
    std::string sConfigPath = sAppDir+"/ServerLocalConfig.xml";
    if(!GetInst(LocalConfig).load_local_config(sConfigPath.c_str())){//{AppDir.toLocal8Bit().constData())
        QMessageBox::information(NULL,QObject::tr("error"),QObject::tr("Load local config file error!"));
        return -1;
    }

    DefaultLog::log()->set_filter(DEBUG_LOG, 0);
    DefaultLog::log()->set_filter(INFO_LOG, 0);
#ifdef Q_OS_WIN
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Systray"),
            QObject::tr("I couldn't detect any system tray "
            "on this system."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);
#endif

    MainWindow w;

#ifdef Q_OS_WIN
    w.show();
    a.setActivationWindow(&w);
    QObject::connect(&a,SIGNAL(messageReceived(const QString&)),&w,SLOT(signalAppSlot(const QString &)));
#else
    w.showMaximized();
#endif
    int nyet = a.exec();
    ConnectionPool::release();
    return nyet;
}
