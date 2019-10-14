#pragma once

#ifndef __IMPPROC_H__
#define __IMPPROC_H__

#include <jni.h>
#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <functional>
#include <thread>
#include <string>
#include "../camera/camera_manager.h"
#include "../imagereader/image_reader.h"
#include "../utils/notifier.h"
#include <unordered_map>
#include "../opencv2/opencv.hpp"
#include "../mediacodec/rtmp.h"
#include <SLES/OpenSLES.h>
#include "../audio/audio_common.h"
#include "../audio/audio_recorder.h"

class ImpProc {
public:
	explicit ImpProc(JNIEnv *env, jobject instance);	
	~ImpProc();

	void  init_camera(int height, int width, bool  isback);
	void  init_facerec(std::string path);
	void  image_proc(ImageReader *reader);
	void  add_faceSet(int*  image, int width, int height, std::string name);
	std::string  find_face(std::vector<float> vec, float &cos);
	void draw_Image(cv::Mat img);
	
	void set_Window(jobject surface);

	void startPreview(bool start) {
		if(camera_)		camera_->StartPreview(start);
	}

	const ImageFormat& GetCompatibleCameraRes() const {
		return view_;
	}

    void init_textNotifier();
    void add_text(std::string  str);
    std::string  get_text();
    int  wait(int millisecond);

	void init_audio(int sampleRate, int framesPerBuf);
	void audio_proc(sample_buf *data);

	void start_audio(bool start);
private:
	JNIEnv * env_;
	jobject javaInstance_;
	jobject surface_ =nullptr;

	std::unique_ptr<NDKCamera>  camera_;
	std::unique_ptr<ImageReader> jpgReader_;
	std::unordered_map<std::string, std::vector<float>> faceSet;

    std::unique_ptr<utils_imp::Notifier> textNotifier;
    std::unique_ptr<std::list<std::string>> textList;

	ANativeWindow *showWindow_=nullptr;
	ImageFormat  view_;

    Rtmp  *m_rtmp = nullptr;

	SLObjectItf m_slEngineObj_;
	SLEngineItf m_slEngineItf_;
	AudioRecorder *m_recorder_;

    std::mutex lock_;
    void push_video(cv::Mat dst);
    void showImage(ANativeWindow_Buffer *buf, uint32_t* out_data, int h, int w);
	void merge_Image(cv::Mat &img, cv::Mat face);
};

#endif  
