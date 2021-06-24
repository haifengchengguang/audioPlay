#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QFile>
#include <QLineEdit>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>
#include <QTime>
#include <QBuffer>
#include <QIODevice>


extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libavutil/error.h>
}

class playthread : public QThread
{
    Q_OBJECT

    enum controlType{

        control_none ,

        control_stop ,    //停止

        control_pause,    //暂停

        control_resume,   //恢复

        control_play,     //播放

        control_seek     //滑动至其它位置播放

    };

    controlType type;

    QString filePath;
    int seekMs;

    QAudioOutput* audio; // class member.

    void run();

    void runPlay();

    bool initAudio(int SampleRate);

    void debugErr(QString prefix, int err);

    bool runIsBreak();      //处理控制

public:
    playthread();

    int getseekMs();
    void play(QString filePath);

    void stop();

    void pause();

    void resume();

    void seek(int value);

signals:
    void ERROR(QString err);

    void duration(int currentMs,int destMs);        //播放时长

    void seekOk();
};

#endif // PLAYTHREAD_H
