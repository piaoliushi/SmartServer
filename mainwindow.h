#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qnotifyhandler.h"
using namespace std;
namespace Ui {
class MainWindow;
}

class  QSvcStatePage;
class  QDevStatePage;
class  QClientStatePage;
class  QSystemInfoPage;
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
private:
    Ui::MainWindow *ui;
    QNotifyHandler         d_Notify;
    QSvcStatePage      *d_pSvcStatePage;
    QDevStatePage      *d_pDevStatePage;
    QClientStatePage   *d_pClientStatePage;
    QSystemInfoPage   *d_pSystemInfoPage;
};

#endif // MAINWINDOW_H
