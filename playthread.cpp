#include "playthread.h"

playthread::playthread()
{
    audio=NULL;

    type = control_none;
}

bool playthread::initAudio(int SampleRate)
{
    QAudioFormat format;

    if(audio!=NULL)
        return true;

    format.setSampleRate(SampleRate);		//设置采样率
    format.setChannelCount(2);		//设置通道数
    format.setSampleSize(16);		//样本数据16位
    format.setCodec("audio/pcm");		//播出格式为pcm格式
    format.setByteOrder(QAudioFormat::LittleEndian);  //默认小端模式
    format.setSampleType(QAudioFormat::UnSignedInt);	//无符号整形数

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());	//选择默认输出设备

//    foreach(int count,info.supportedChannelCounts())
//    {
//        qDebug()<<"输出设备支持的通道数:"<<count;
//    }

//    foreach(int count,info.supportedSampleRates())
//    {
//        qDebug()<<"输出设备支持的采样率:"<<count;
//    }

//    foreach(int count,info.supportedSampleSizes())
//    {
//        qDebug()<<"输出设备支持的样本数据位数:"<<count;
//    }

    if (!info.isFormatSupported(format))
    {
        qDebug()<<"输出设备不支持该格式，不能播放音频";
        return false;
    }

    audio = new QAudioOutput(format, this);

    audio->setBufferSize(100000);

    return true;
}
int playthread::getseekMs(){
    return seekMs;
}
void playthread::play(QString filePath)
{
    this->filePath = filePath;
    type = control_play;

    if(!this->isRunning())
    {
         this->start();
    }
}

void playthread::stop()
{

    if(this->isRunning())
    {
        type = control_stop;
    }

}
void playthread::pause()
{

    if(this->isRunning())
    {
        type = control_pause;
    }

}

void playthread::resume()
{
    if(this->isRunning())
    {
        type = control_resume;
    }
}


void playthread::seek(int value)
{

    if(this->isRunning())
    {
        seekMs = value;
        type = control_seek;
    }
}

void playthread::debugErr(QString prefix, int err)  //根据错误编号获取错误信息并打印
{
    char errbuf[512]={0};

    av_strerror(err,errbuf,sizeof(errbuf));

    qDebug()<<prefix<<":"<<errbuf;

    emit ERROR(prefix+":"+errbuf);
}



bool playthread::runIsBreak()      //处理控制,判断是否需要停止
{

    bool ret = false;
    //处理播放暂停
    if(type == control_pause)
    {
        while(type == control_pause)
        {
             audio->suspend();
             msleep(500);
        }

        if(type == control_resume)
        {
             audio->resume();
        }
    }

    if(type == control_play)    //重新播放
    {
        ret = true;
        if(audio->state()== QAudio::ActiveState)
            audio->stop();
    }

    if(type == control_stop)    //停止
    {
         ret = true;
         if(audio->state()== QAudio::ActiveState)
             audio->stop();
    }
    return ret;
}

void playthread::runPlay()
{
    int ret;

    int destMs,currentMs;

    if(audio==NULL)
    {
        emit ERROR("输出设备不支持该格式，不能播放音频");
        return ;
    }
    //初始化网络库 （可以打开rtsp rtmp http 协议的流媒体视频）
    avformat_network_init();
    AVFormatContext *pFmtCtx=NULL;
    ret = avformat_open_input(&pFmtCtx, this->filePath.toLocal8Bit().data(),NULL, NULL) ;  //打开音视频文件并创建AVFormatContext结构体以及初始化.
    if (ret!= 0)
    {
        debugErr("avformat_open_input",ret);
        return ;
    }
    ret = avformat_find_stream_info(pFmtCtx, NULL);   //初始化流信息
    if (ret!= 0)
    {
        debugErr("avformat_find_stream_info",ret);
        return ;
    }

    int audioindex=-1;

    audioindex = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    qDebug()<<"audioindex:"<<audioindex;

    AVCodec *acodec = avcodec_find_decoder(pFmtCtx->streams[audioindex]->codecpar->codec_id);//获取codec

    AVCodecContext *acodecCtx = avcodec_alloc_context3(acodec); //构造AVCodecContext ,并将vcodec填入AVCodecContext中
    avcodec_parameters_to_context(acodecCtx, pFmtCtx->streams[audioindex]->codecpar); //初始化AVCodecContext

    ret = avcodec_open2(acodecCtx, NULL,NULL);  //打开解码器,由于之前调用avcodec_alloc_context3(vcodec)初始化了vc,那么codec(第2个参数)可以填NULL
    if (ret!= 0)
    {
        debugErr("avcodec_open2",ret);
        return ;
    }
    SwrContext *swrctx =NULL;
    swrctx=swr_alloc_set_opts(swrctx, av_get_default_channel_layout(2),AV_SAMPLE_FMT_S16,44100,
                                acodecCtx->channel_layout, acodecCtx->sample_fmt,acodecCtx->sample_rate, NULL,NULL);
    swr_init(swrctx);

    destMs = av_q2d(pFmtCtx->streams[audioindex]->time_base)*1000*pFmtCtx->streams[audioindex]->duration;
    qDebug()<<"码率:"<<acodecCtx->bit_rate;
    qDebug()<<"格式:"<<acodecCtx->sample_fmt;
    qDebug()<<"通道:"<<acodecCtx->channels;
    qDebug()<<"采样率:"<<acodecCtx->sample_rate;
    qDebug()<<"时长:"<<destMs;
    qDebug()<<"解码器:"<<acodec->name;

    AVPacket * packet =av_packet_alloc();
    AVFrame *frame =av_frame_alloc();

    audio->stop();
    QIODevice*io = audio->start();

    while(1)
    {


        if(runIsBreak())
            break;

        if(type == control_seek)
        {
            av_seek_frame(pFmtCtx, audioindex, seekMs/(double)1000/av_q2d(pFmtCtx->streams[audioindex]->time_base),  AVSEEK_FLAG_BACKWARD);
            type = control_none;
            emit seekOk();
        }

        ret = av_read_frame(pFmtCtx, packet);
        if (ret!= 0)
        {
            debugErr("av_read_frame",ret);
            emit duration(destMs,destMs);
            break ;
        }



        if(packet->stream_index==audioindex)
        {
            //解码一帧数据
            ret = avcodec_send_packet(acodecCtx, packet);
            av_packet_unref(packet);

            if (ret != 0)
            {
                debugErr("avcodec_send_packet",ret);
                continue ;
            }

            while( avcodec_receive_frame(acodecCtx, frame) == 0)
            {

                if(runIsBreak())
                    break;
                uint8_t *data[2] = { 0 };
                int byteCnt=frame->nb_samples * 2 * 2;

                unsigned char *pcm = new uint8_t[byteCnt];     //frame->nb_samples*2*2表示分配样本数据量*两通道*每通道2字节大小

                data[0] = pcm;  //输出格式为AV_SAMPLE_FMT_S16(packet类型),所以转换后的LR两通道都存在data[0]中

                ret = swr_convert(swrctx,
                                  data, frame->nb_samples,		//输出
                                 (const uint8_t**)frame->data,frame->nb_samples );	//输入


                //将重采样后的data数据发送到输出设备,进行播放
                while (audio->bytesFree() < byteCnt)
                {
                    if(runIsBreak())
                        break;
                    msleep(10);
                }

                if(!runIsBreak())
                 io->write((const char *)pcm,byteCnt);

                currentMs = av_q2d(pFmtCtx->streams[audioindex]->time_base)*1000*frame->pts;
                //qDebug()<<"时长:"<<destMs<<currentMs;
                emit duration(currentMs,destMs);

                delete[] pcm;
            }
        }


    }


    //释放内存
    av_frame_free(&frame);
    av_packet_free(&packet);
    swr_free(&swrctx);
    avcodec_free_context(&acodecCtx);
    avformat_close_input(&pFmtCtx);

}

void playthread::run()
{

    if(!initAudio(44100))
    {
        emit ERROR("输出设备不支持该格式，不能播放音频");
    }

    while(1)
    {

        switch(type)
        {
            case control_none: msleep(100);    break;
            case control_play : type=control_none;runPlay();  break;    //播放
            default: type=control_none;   break;
        }
    }

}


