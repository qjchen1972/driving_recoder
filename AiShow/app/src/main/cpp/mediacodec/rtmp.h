//
// Created by 陈黔江 on 2019/1/23.
//

#ifndef AISHOW_RTMP_H
#define AISHOW_RTMP_H

#include <string>
#include "../opencv2/opencv.hpp"

extern "C" {
#include "../ffmpeg/libavformat/avformat.h"
#include "../ffmpeg/libavcodec/avcodec.h"
#include "../ffmpeg/libavutil/imgutils.h"
#include "../ffmpeg/libswscale/swscale.h"
#include "../ffmpeg/libswresample/swresample.h"
#include "../ffmpeg/libavutil/timestamp.h"
}

class Rtmp{
public:
    Rtmp();
    ~Rtmp();

    int  init_ffmpeg(int  width, int height, int fps, int bitrate, std::string codec_profile,
                     int sample_rate, int bit_rate,std::string server);
    void exit_ffmpeg();
    int  push_video_stream(cv::Mat image);
    int  push_audio_stream(uint8_t* buf, int size);
    bool test_pts();

private:

    int  initialize_avformat_context(AVFormatContext* &fctx, const char *format_name);
    int  initialize_io_context(AVFormatContext* &fctx, const char *output);
    void set_video_codec_params(AVFormatContext *&fctx, AVCodecContext *&codec_ctx, int width, int height, int fps, int bitrate);
    void set_audio_codec_params(AVFormatContext *&fctx, AVCodecContext *&codec_ctx, int sample_rate, int bit_rate);

    int  initialize_video_codec_stream(AVStream *&stream, AVCodecContext *&codec_ctx, AVCodec *&codec, std::string codec_profile);
    int  initialize_audio_codec_stream(AVStream *&stream, AVCodecContext *&codec_ctx, AVCodec *&codec);

    SwsContext*  initialize_sample_scaler(AVCodecContext *codec_ctx);
    AVFrame*  allocate_video_frame_buffer(AVCodecContext *codec_ctx);

    SwrContext*  initialize_sample_resample(AVCodecContext *codec_ctx);
    AVFrame*  allocate_audio_frame_buffer(AVCodecContext *codec_ctx);

    int  write_frame(AVCodecContext *codec_ctx, AVFormatContext *fmt_ctx, AVStream *stream,AVFrame *frame);
    void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt);
    int put_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt);


    AVFormatContext *m_ofmt_ctx = nullptr;

    AVStream *m_video_out_stream = nullptr;
    AVStream *m_audio_out_stream = nullptr;

    AVCodecContext *m_video_out_codec_ctx = nullptr;
    AVCodecContext *m_audio_out_codec_ctx = nullptr;

    SwsContext *m_swsctx = nullptr;
    AVFrame *m_video_frame =nullptr;

    SwrContext *m_swrctx= nullptr;
    AVFrame *m_audio_frame =nullptr;

    int m_height= 0;
    int m_width = 0;
    int m_video_next_pts = 0;
    int m_audio_next_pts = 0;
};

#endif //AISHOW_RTMP_H
