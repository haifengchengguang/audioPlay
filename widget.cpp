#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->setAcceptDrops(true);

    this->setWindowTitle("多媒体播放器");
    thread = new playthread();

    connect(thread,SIGNAL(duration(int,int)),this,SLOT(onDuration(int,int)));

    connect(thread,SIGNAL(seekOk()),this,SLOT(onSeekOk()));


    void duration(long currentMs,long destMs);        //播放时长

    thread->start();

    sliderSeeking =false;
}



Widget::~Widget()
{
    delete ui;


    thread->stop();
}


void Widget::onSeekOk()
{
    sliderSeeking=false;
}

void Widget::onDuration(int currentMs,int destMs)      //时长
{
    static int currentMs1=-1,destMs1=-1;

    if(currentMs1==currentMs&&destMs1==destMs)
    {
        return;
    }

    currentMs1 = currentMs;
    destMs1   =  destMs;

    qDebug()<<"onDuration："<<currentMs<<destMs<<sliderSeeking;

    QString currentTime = QString("%1:%2:%3").arg(currentMs1/360000%60,2,10,QChar('0')).arg(currentMs1/6000%60/10,2,10,QChar('0')).arg(currentMs1/1000%60,2,10,QChar('0'));

    QString destTime = QString("%1:%2:%3").arg(destMs1/360000%60,2,10,QChar('0')).arg(destMs1/6000%60/10,2,10,QChar('0')).arg(destMs1/1000%60,2,10,QChar('0'));


    ui->label_duration->setText(currentTime+"/"+destTime);



    if(!sliderSeeking) //未滑动
    {
        ui->slider->setMaximum(destMs);
        ui->slider->setValue(currentMs);
    }

}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
      if(event->mimeData()->hasUrls())      //判断拖的类型
      {
            event->acceptProposedAction();
      }
      else
      {
            event->ignore();
      }
}

void Widget::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())        //判断放的类型
    {

        QList<QUrl> List = event->mimeData()->urls();

        if(List.length()!=0)
        {
          ui->line_audioPath->setText(List[0].toLocalFile());
        }

    }
    else
    {
          event->ignore();
    }
}


void Widget::on_btn_start_clicked()
{

    sliderSeeking=false;

    thread->play(ui->line_audioPath->text());

}


void Widget::on_btn_stop_clicked()
{
    thread->stop();
}

void Widget::on_btn_pause_clicked()
{
    thread->pause();
}

void Widget::on_btn_resume_clicked()
{
   thread->resume();
}


void Widget::on_slider_sliderPressed()
{
    sliderSeeking=true;
}

void Widget::on_slider_sliderReleased()
{

    thread->seek(ui->slider->value());

}

void Widget::on_pushButton_clicked()
{

    int position=thread->getseekMs();
    qDebug()<<"position"<<position;
    position=position+5000;
    ui->slider->setValue(position);
    //ui->label_duration
    thread->seek(position);
}
void Widget::on_pushButton_2_clicked()
{
    QFileDialog* f=new QFileDialog(this);
    f->setWindowTitle("选择音频文件*.mp3");
    f->setNameFilter("*.mp3");
    f->setViewMode(QFileDialog::Detail);

    QString filePath;
    if(f->exec()==QDialog::Accepted)
    filePath=f->selectedFiles()[0];
    ui->line_audioPath->setText(filePath);
}
void Widget::on_btn_back_clicked()
{
    int position=thread->getseekMs();
    qDebug()<<"position"<<position;
    position=position-5000;
    ui->slider->setValue(position);
    //ui->label_duration
    thread->seek(position);
}

