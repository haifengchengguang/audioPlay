#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFile>
#include <QLineEdit>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QTime>
#include "playthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

    playthread *thread;


    bool sliderSeeking;      //滑动标志位,false:表示未seek,true:正在seek

public:
    Widget(QWidget *parent = nullptr);

    ~Widget();

private slots:
    void on_btn_start_clicked();

    void on_btn_stop_clicked();

    void on_btn_pause_clicked();

    void on_btn_resume_clicked();

    void onDuration(int currentMs, int destMs);

    void onSeekOk();


    void on_slider_sliderPressed();

    void on_slider_sliderReleased();

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_btn_back_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
