//
// Created by 陈黔江 on 2019/1/23.
//
#include "../utils/native_debug.h"
#include "rtmp.h"

Rtmp::Rtmp(){

}
Rtmp::~Rtmp(){
    exit_ffmpeg();
}

int  Rtmp::init_ffmpeg(int width, int height, int fps, int bitrate, std::string codec_profile,
                       int sample_rate, int bit_rate,std::string server){

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif
    int ret = avformat_network_init();
    if(ret<0) {
        LOGI("avformat_network_init Error!");
        return ret;
    }

    const char *output = server.c_str();
    m_height = width;//height;
    m_width = height;//width;
    ret = initialize_avformat_context(m_ofmt_ctx, "flv");
    if(ret<0) {
        LOGI("initialize_avformat_context Error!");
        return ret;
    }

    initialize_io_context(m_ofmt_ctx, output);
    if(ret<0) {
        LOGI("initialize_io_context Error!");
        return ret;
    }

    AVCodec* video_out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!video_out_codec) {
        LOGI("avcodec_find_encoder video Error!");
        return -1;
    }

    AVCodec* audio_out_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if(!video_out_codec) {
        LOGI("avcodec_find_encoder video Error!");
        return -1;
    }

    m_video_out_stream = avformat_new_stream(m_ofmt_ctx, nullptr);// out_codec);
    if(!m_video_out_stream) {
        LOGI("avformat_new_stream Error!");
        return -1;
    }
    m_video_out_stream->id =m_ofmt_ctx->nb_streams-1;

    m_video_out_codec_ctx = avcodec_alloc_context3(video_out_codec);
    if(!m_video_out_codec_ctx) {
        LOGI("avcodec_alloc_context3 Error!");
        return -1;
    }

    m_audio_out_stream = avformat_new_stream(m_ofmt_ctx, nullptr);// out_codec);
    if(!m_audio_out_stream) {
        LOGI("avformat_new_stream Error!");
        return -1;
    }

    m_audio_out_stream->id =m_ofmt_ctx->nb_streams-1;

    m_audio_out_codec_ctx = avcodec_alloc_context3(audio_out_codec);
    if(!m_audio_out_codec_ctx) {
        LOGI("avcodec_alloc_context3 Error!");
        return -1;
    }

    set_video_codec_params(m_ofmt_ctx, m_video_out_codec_ctx, m_width, m_height, fps, bitrate);
    set_audio_codec_params(m_ofmt_ctx, m_audio_out_codec_ctx, sample_rate, bit_rate);

    ret = initialize_video_codec_stream(m_video_out_stream, m_video_out_codec_ctx, video_out_codec, codec_profile);
    if(ret<0) {
        LOGI("initialize_codec_stream Error!");
        return ret;
    }

    ret = initialize_audio_codec_stream(m_audio_out_stream, m_audio_out_codec_ctx, audio_out_codec);
    if(ret<0) {
        LOGI("initialize_codec_stream Error!");
        return ret;
    }
    m_video_out_stream->codecpar->extradata = m_video_out_codec_ctx->extradata;
    m_video_out_stream->codecpar->extradata_size = m_video_out_codec_ctx->extradata_size;

    m_audio_out_stream->codecpar->extradata = m_audio_out_codec_ctx->extradata;
    m_audio_out_stream->codecpar->extradata_size = m_audio_out_codec_ctx->extradata_size;

    av_dump_format(m_ofmt_ctx, 0, output, 1);

    m_swsctx = initialize_sample_scaler(m_video_out_codec_ctx);
    m_video_frame = allocate_video_frame_buffer(m_video_out_codec_ctx);

    m_audio_frame = allocate_audio_frame_buffer(m_audio_out_codec_ctx);
    m_swrctx = initialize_sample_resample(m_audio_out_codec_ctx);

    ret = avformat_write_header(m_ofmt_ctx, nullptr);
    if (ret < 0)	LOGI("Could not write header");

    return ret;
}


void  Rtmp::exit_ffmpeg(){

    av_write_trailer(m_ofmt_ctx);

    avcodec_free_context(&m_video_out_codec_ctx);
    av_frame_free(&m_video_frame);
    sws_freeContext(m_swsctx);

    avcodec_free_context(&m_audio_out_codec_ctx);
    av_frame_free(&m_audio_frame);
    swr_free(&m_swrctx);

    avio_close(m_ofmt_ctx->pb);
    avformat_free_context(m_ofmt_ctx);
    avformat_network_deinit();
}


int   Rtmp::push_video_stream(cv::Mat img){

    cv::Mat image;
    img.copyTo(image);

    const int stride[] = { static_cast<int>(image.step[0]) };
    sws_scale(m_swsctx, &image.data, stride, 0, image.rows, m_video_frame->data, m_video_frame->linesize);
    m_video_frame->pts = m_video_next_pts++;//= av_rescale_q(1, m_video_out_codec_ctx->time_base, m_video_out_stream->time_base);
    int ret = write_frame(m_video_out_codec_ctx, m_ofmt_ctx, m_video_out_stream,m_video_frame);
    return ret;
}

int  Rtmp::push_audio_stream(uint8_t* buf, int size){
    int dst_nb_samples = 1024;
    /* convert to destination format */
    int ret = swr_convert(m_swrctx,m_audio_frame->data,dst_nb_samples,(const uint8_t **)&buf, size/2);
    m_audio_frame->pts = m_audio_next_pts;//av_rescale_q(m_audio_samples_count,
                                      //(AVRational){1, m_audio_out_codec_ctx->sample_rate},
                                       //m_audio_out_codec_ctx->time_base);
    m_audio_next_pts += dst_nb_samples;
    ret = write_frame(m_audio_out_codec_ctx, m_ofmt_ctx, m_audio_out_stream,m_audio_frame);
    return ret;
 }

 bool Rtmp::test_pts(){
     return av_compare_ts(m_video_next_pts, m_video_out_codec_ctx->time_base,
                          m_audio_next_pts, m_audio_out_codec_ctx->time_base) <= 0;
}

int   Rtmp::initialize_avformat_context(AVFormatContext* &fctx, const char *format_name){

    int ret = avformat_alloc_output_context2(&fctx, nullptr, format_name, nullptr);
    if (ret < 0) LOGI("Could not allocate output format context!");
    return  ret;
}

int   Rtmp::initialize_io_context(AVFormatContext* &fctx, const char *output){
    int ret = -1;
    if (!(fctx->oformat->flags & AVFMT_NOFILE)){
        ret = avio_open2(&fctx->pb, output, AVIO_FLAG_WRITE, nullptr, nullptr);
        if (ret < 0) LOGI("Could not open output IO context!");
    }
    return ret;
}

void  Rtmp::set_video_codec_params(AVFormatContext *&fctx, AVCodecContext *&codec_ctx, int width, int height, int fps, int bitrate){

    const AVRational dst_fps = { fps, 1 };
    codec_ctx->codec_tag = 0;
    codec_ctx->codec_id = AV_CODEC_ID_H264;
    codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    codec_ctx->width = width;
    codec_ctx->height = height;
    codec_ctx->gop_size = 12;
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    codec_ctx->framerate = dst_fps;
    codec_ctx->time_base = av_inv_q(dst_fps);
    codec_ctx->bit_rate = bitrate;
    if (fctx->oformat->flags & AVFMT_GLOBALHEADER){
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    //m_video_out_stream->time_base = codec_ctx->time_base;
}

void  Rtmp::set_audio_codec_params(AVFormatContext *&fctx, AVCodecContext *&codec_ctx, int sample_rate, int bit_rate) {
    codec_ctx->codec_tag = 0;
    codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
    codec_ctx->sample_fmt =  AV_SAMPLE_FMT_FLTP;
    codec_ctx->sample_rate = 44100;//sample_rate;
    codec_ctx->bit_rate = 64000;//bit_rate;
    codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;//AV_CH_LAYOUT_MONO;//AV_CH_LAYOUT_STEREO;
    codec_ctx->channels = av_get_channel_layout_nb_channels(codec_ctx->channel_layout);
    //ost->st->time_base = (AVRational){ 1, c->sample_rate };
    if (fctx->oformat->flags & AVFMT_GLOBALHEADER){
        codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    //m_audio_out_stream->time_base = (AVRational){ 1, codec_ctx->sample_rate };
    //codec_ctx->time_base = (AVRational){ 1, codec_ctx->sample_rate };
}

int   Rtmp::initialize_video_codec_stream(AVStream *&stream, AVCodecContext *&codec_ctx, AVCodec *&codec, std::string codec_profile){
    int ret = avcodec_parameters_from_context(stream->codecpar, codec_ctx);
    if (ret < 0) {
        LOGI("Could not initialize stream codec parameters!");
        return ret;
    }
    AVDictionary *codec_options = nullptr;
    av_dict_set(&codec_options, "profile", codec_profile.c_str(), 0);
    av_dict_set(&codec_options, "preset", "superfast", 0);
    av_dict_set(&codec_options, "tune", "zerolatency", 0);


    // open video encoder
    ret = avcodec_open2(codec_ctx, codec, &codec_options);
    av_dict_free(&codec_options);
    if (ret < 0){
        LOGI("Could not open video encoder!");
    }
    return ret;
}

int   Rtmp::initialize_audio_codec_stream(AVStream *&stream, AVCodecContext *&codec_ctx, AVCodec *&codec){

    int ret = avcodec_parameters_from_context(stream->codecpar, codec_ctx);
    if (ret < 0) {
        LOGI("Could not initialize stream codec parameters!");
        return ret;
    }
    // open video encoder
    ret = avcodec_open2(codec_ctx, codec,  NULL);
    if (ret < 0){
        LOGI("Could not open video encoder!");
    }
    return ret;
}

SwsContext*  Rtmp::initialize_sample_scaler(AVCodecContext *codec_ctx){
    SwsContext *swsctx = sws_getContext(codec_ctx->width, codec_ctx->height, AV_PIX_FMT_BGR24, codec_ctx->width, codec_ctx->height,
                                        codec_ctx->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (!swsctx)		LOGI("Could not initialize sample scaler!");
    return swsctx;
}

AVFrame*  Rtmp::allocate_video_frame_buffer(AVCodecContext *codec_ctx){

    AVFrame *frame = av_frame_alloc();
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;
    frame->format = static_cast<int>(codec_ctx->pix_fmt);
    int ret = av_frame_get_buffer(frame, 32);
    if(ret < 0) {
        LOGI("Could not alloc frame");
        av_frame_free(&frame);
        frame = nullptr;
    }
    return frame;
}

AVFrame*  Rtmp::allocate_audio_frame_buffer(AVCodecContext *codec_ctx){

    AVFrame *frame = av_frame_alloc();

    int nb_samples;
    if (codec_ctx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = codec_ctx->frame_size;

    frame->format = codec_ctx->sample_fmt;
    frame->channel_layout = codec_ctx->channel_layout;
    frame->sample_rate = codec_ctx->sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        int ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            LOGI("Error allocating an audio buffer");
            return nullptr;
        }
    }

    return frame;
}

SwrContext*  Rtmp::initialize_sample_resample(AVCodecContext *codec_ctx){

    SwrContext *swrctx = swr_alloc_set_opts(NULL,  // we're allocating a new context
                       codec_ctx->channel_layout,  // out_ch_layout
                       codec_ctx->sample_fmt,    // out_sample_fmt
                       codec_ctx->sample_rate, // out_sample_rat
                       AV_CH_LAYOUT_MONO,//codec_ctx->channel_layout, // in_ch_layout
                       AV_SAMPLE_FMT_S16,   // in_sample_fmt
                       48000, // in_sample_rate
                       0,                    // log_offset
                       NULL);                // log_ctx

    if (!swrctx) {
        LOGI("Could not initialize sample resample!");
        return swrctx;
    }
    if (swr_init(swrctx) < 0) {
        LOGI("Failed to initialize the resampling context\n");
        swr_free(&swrctx);
        return swrctx;
    }
    return swrctx;
}

int  Rtmp::write_frame(AVCodecContext *codec_ctx, AVFormatContext *fmt_ctx, AVStream *stream, AVFrame* frame){

    AVPacket pkt = { 0 };
    av_init_packet(&pkt);
    int ret = avcodec_send_frame(codec_ctx, frame);
    if (ret < 0){
        LOGI("Error sending frame to codec context!");
        return ret;
    }
    ret = avcodec_receive_packet(codec_ctx, &pkt);
    if (ret < 0){
        LOGI("Error receiving packet from codec context!");
        return ret;
    }

    ret = put_frame(fmt_ctx, &codec_ctx->time_base, stream, &pkt);
    //av_interleaved_write_frame(fmt_ctx, &pkt);
    av_packet_unref(&pkt);
    return ret;
}

 void Rtmp::log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    LOGI("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

int Rtmp::put_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream* st, AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;
    /* Write the compressed frame to the media file. */
    //log_packet(fmt_ctx, pkt);
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

