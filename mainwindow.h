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
class  QProcess;
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

    //void  timeUpdate();
    //void readStandardOutput();
protected:
    //void OpenBacklight();
    //bool event(QEvent *event);
    //bool eventFilter(QObject *object, QEvent *event);
private:
    Ui::MainWindow *ui;
    QNotifyHandler         d_Notify;
    QSvcStatePage      *d_pSvcStatePage;
    QDevStatePage      *d_pDevStatePage;
    QClientStatePage   *d_pClientStatePage;
    QSystemInfoPage   *d_pSystemInfoPage;

    int    d_close_backlight_count;
    int    d_cur_backlight;
     //QProcess *d_process_ptr;
};

#endif // MAINWINDOW_H
