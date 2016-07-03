#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#ifdef Q_OS_WIN
#include <QSystemTrayIcon>
#endif
#include "qnotifyhandler.h"
using namespace std;
namespace Ui {
class MainWindow;
}

class  QSvcStatePage;
class  QDevStatePage;
class  QClientStatePage;
class  QSystemInfoPage;

#ifdef Q_OS_WIN
class  QSystemTrayIcon;
#endif
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:

    void testSetTime();
    void ShowSvcStatePage();
    void ShowDevStatePage();
    void ShowClientStatePage();
    void ShowSystemInfoPage();
    void OnUpdateDevList(bool bLoad);
#ifdef Q_OS_WIN
    void startSvc();
    void OnUpdateMenu();
    void showMessage(QString &sHeader,QString &sContent);
    void timeUpdate();
    void signalAppSlot(const QString& activeMsg );
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void quitApp();
protected:
    void closeEvent(QCloseEvent *event);
private:
    void createTrayIcon();
    void createActions();
#endif
private:
    Ui::MainWindow *ui;
    QNotifyHandler         d_Notify;
    QSvcStatePage      *d_pSvcStatePage;
    QDevStatePage      *d_pDevStatePage;
    QClientStatePage   *d_pClientStatePage;
    QSystemInfoPage   *d_pSystemInfoPage;
#ifdef Q_OS_WIN
    QAction *startupAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QDateTime tSvcStartTime;
#endif
};

#endif // MAINWINDOW_H
