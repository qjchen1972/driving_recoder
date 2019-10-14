#include <cstdio>
#include <cstring>
#define USE_OPENCV
#include "impproc.h"
#include "../camera/camera_manager.h"
#include "../utils/native_debug.h"
#include "../utils/notifier.h"
#include "../mtcnn/mtcnn_imp.h"
#include "../face_lib/faceimp.h"
#include "../imagereader/image_reader.h"



ImpProc::ImpProc(JNIEnv* env, jobject instance): env_(env),javaInstance_(instance), surface_(nullptr){
	init_textNotifier();
	memset(&view_, 0, sizeof(view_));
}

ImpProc::~ImpProc() {
	if (surface_) {
		env_->DeleteGlobalRef(surface_);
		surface_ = nullptr;
	}
    if(showWindow_) {
	    ANativeWindow_release(showWindow_);
        showWindow_ = nullptr;
	}
   if(m_rtmp) {
        delete m_rtmp;
        m_rtmp = nullptr;
    }
    camera_.reset();
    jpgReader_.reset();
    textNotifier.reset();
    textList.reset();
}

void ImpProc::init_camera(int height, int width, bool  isback) {
	if( isback)
		camera_.reset(new NDKCamera(ACAMERA_LENS_FACING_BACK));
	else
		camera_.reset(new NDKCamera(ACAMERA_LENS_FACING_FRONT));

	camera_->MatchCaptureSizeRequest(width, height, &view_);

    jpgReader_.reset(new ImageReader(view_.width, view_.height,AIMAGE_FORMAT_JPEG));
	jpgReader_->SetPresentRotation(0);
	jpgReader_->RegisterCallback(this, [this](void *ctx, ImageReader* reader) -> void {
		reinterpret_cast<ImpProc* >(ctx)->image_proc(reader);
	});
	camera_->CreateSession(jpgReader_->GetNativeWindow(), nullptr, false, 0);

    m_rtmp = new Rtmp();
    int fps = 30, bitrate = 300000;
    std::string h264profile = "high444";
    std::string outputServer = "rtmp://52.83.197.38/live/aiShow";
     int ret = m_rtmp->init_ffmpeg(view_.width,view_.height,fps,bitrate,h264profile,48000,64000,outputServer);
     if(ret < 0 ){
         LOGI("init_ffmpeg Error!");
         delete m_rtmp;
         m_rtmp = nullptr;
     }
 }


void ImpProc::init_audio(int sampleRate, int framesPerBuf){
    SLresult result;
    result = slCreateEngine(&m_slEngineObj_, 0, NULL, 0, NULL, NULL);
    SLASSERT(result);

    result =(*m_slEngineObj_)->Realize(m_slEngineObj_, SL_BOOLEAN_FALSE);
    SLASSERT(result);
    result = (*m_slEngineObj_)->GetInterface(m_slEngineObj_, SL_IID_ENGINE, &m_slEngineItf_);
 	 SLASSERT(result);

	 SampleFormat sampleFormat;
	 memset(&sampleFormat, 0, sizeof(sampleFormat));
	 sampleFormat.pcmFormat_ = static_cast<uint16_t>(SL_PCMSAMPLEFORMAT_FIXED_16);

	 // SampleFormat.representation_ = SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT;
	 sampleFormat.channels_ = AUDIO_SAMPLE_CHANNELS;
	 sampleFormat.sampleRate_ = static_cast<SLmilliHertz>(sampleRate) * 1000;
	 sampleFormat.framesPerBuf_ = static_cast<uint32_t>(framesPerBuf);

     m_recorder_ = new AudioRecorder(&sampleFormat, m_slEngineItf_);
	 if (!m_recorder_)  return;
	 m_recorder_->RegisterCallback(this, [this](void *ctx, sample_buf *data) -> void {
		 reinterpret_cast<ImpProc* >(ctx)->audio_proc(data);
	 });
}

void  ImpProc::audio_proc(sample_buf *data) {
    std::lock_guard<std::mutex> lock(lock_);
    m_rtmp->push_audio_stream(data->buf_, data->cap_);
}

void ImpProc::start_audio(bool start) {
	if(start)
		m_recorder_->Start();
	else
		m_recorder_->Stop();
}

void ImpProc::set_Window(jobject surface) {
	if(showWindow_) ANativeWindow_release(showWindow_);
	if (surface_) 	env_->DeleteGlobalRef(surface_);
	surface_ = env_->NewGlobalRef(surface);
	showWindow_ = ANativeWindow_fromSurface(env_, surface);
	ANativeWindow_setBuffersGeometry(showWindow_, view_.height, view_.width, WINDOW_FORMAT_RGBA_8888);
}

void ImpProc::init_facerec(std::string path) {
	mtcnn_imp::init(path);
	mtcnn_imp::set_MinFace(20);

	std::string initnet = path + "init_net.pb";
    std::string precnet = path + "predict_net.pb";
    face_init(initnet.c_str(), precnet.c_str());

    initnet = path + "modify_init_net";
    precnet = path + "modify_pred_net";
	modify_init(initnet.c_str(), precnet.c_str());

	initnet = path + "ssd_init_net";
	precnet = path + "ssd_pred_net";
	ssd_init(initnet.c_str(), precnet.c_str());
}
void ImpProc::push_video(cv::Mat dst){
	std::lock_guard<std::mutex> lock(lock_);
	if(m_rtmp) m_rtmp->push_video_stream(dst);
}


void    ImpProc::image_proc(ImageReader* reader) {

	AImage *image = jpgReader_->GetLatestImage();//GetNextImage();
	if (!image) return;
	uint8_t *data = nullptr;
	int32_t len = 0;
	AImage_getPlaneData(image, 0, &data, &len);
	AImage_delete(image);

	cv::Mat src_data = cv::Mat(1, len, CV_8UC1, data);
	src_data = cv::imdecode(src_data, CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat timg;
	cv::transpose(src_data, timg);
	cv::flip(timg, src_data, 1);

	cv::Mat dst, face;

	src_data.copyTo(dst);



	std::vector<float> ssd_result;
	if(!ssd_get_vec(src_data, ssd_result, &dst) || ssd_result.size() == 0) {
		draw_Image(dst);
		push_video(dst);
		return;
	}

    int cls = 0;
   	cls = int(ssd_result[4]);
		char ch[256];
		sprintf(ch, "%s %.2f", ssd_get_class(cls).c_str(), ssd_result[5]);
		add_text(ch);

	//it is not human
    if(cls != 15){
        draw_Image(dst);
        push_video(dst);
        return;
    }


    float man_height;
    float man_width;
    man_height =(ssd_result[3] - ssd_result[1])* dst.rows;
    man_width = (ssd_result[2] - ssd_result[0])* dst.cols;


	std::vector<mtcnn_imp::SMtcnnFace> finalBbox;
	mtcnn_imp::detectMaxFace(src_data.data, src_data.rows, src_data.cols, mtcnn_imp::eBGR888,
							 finalBbox);
	const int num_box = finalBbox.size();

	if (num_box == 0) {
        draw_Image(dst);
        push_video(dst);
		return;
	}

	std::vector<cv::Rect> bbox;
	bbox.resize(num_box);

	float face_height;
	float face_width;

	for (int i = 0; i < num_box; i++) {
		bbox[i] = cv::Rect(finalBbox[i].boundingBox[0], finalBbox[i].boundingBox[1],
						   finalBbox[i].boundingBox[2] - finalBbox[i].boundingBox[0] + 1,
						   finalBbox[i].boundingBox[3] - finalBbox[i].boundingBox[1] + 1);

		for (int j = 0; j < 5; j = j + 1) {
			cv::circle(dst, cvPoint(finalBbox[i].landmark[j], finalBbox[i].landmark[j + 5]), 2,
					   CV_RGB(0, 255, 0), CV_FILLED);
		}
		face_height = bbox[i].height;
        face_width = bbox[i].width;
	}

	for (std::vector<cv::Rect>::iterator it = bbox.begin(); it != bbox.end(); it++) {
		cv::rectangle(dst, (*it), cv::Scalar(0, 0, 255), 2, 8, 0);
	}

	char buf[1024];
	//sprintf(buf,"找到脸(%d,%d,%d,%d)",bbox[0].x,bbox[0].y,bbox[0].height,bbox[0].width);
	//add_text(buf);


	std::vector<float> vec;
	std::vector<cv::Point2f> five;
	for (int num = 0; num < 5; num++){
		five.push_back(cv::Point2f((float)finalBbox[0].landmark[num], (float)finalBbox[0].landmark[num+5]));
	}
	if (!face_get_vec(src_data,bbox[0], five,vec,&face)){
		draw_Image(dst);
        push_video(dst);
		return;
	}

	cv::Mat newface;
    modify_face(face, newface);
     merge_Image(dst,newface);


	float cos;
	std::string name = find_face(vec, cos);
	if (name.empty()) {
  		draw_Image(dst);
        push_video(dst);
		return;
	}

	sprintf(buf, "这人是 %s. 相似度是 %f,身高是 %f cm", name.c_str(), cos, 20*man_height / face_height);
	add_text(buf);

	cv::Point origin;
	origin.x = bbox[0].x;
	origin.y = bbox[0].y;

	int font_face = cv::FONT_HERSHEY_COMPLEX;
	double font_scale = 1;
	int thickness = 1;
	cv::putText(dst, name.c_str(), origin, font_face, font_scale, cv::Scalar(0, 255, 255),
				thickness, 8, 0);
	draw_Image(dst);
    push_video(dst);
}



void ImpProc::add_faceSet(int*  image, int height, int width, std::string name) {

	char buf[1024];
	cv::Mat imgData(height, width, CV_8UC4, (unsigned char*)image);
	cv::cvtColor(imgData, imgData, CV_BGRA2BGR);
	std::vector<mtcnn_imp::SMtcnnFace> finalBbox;
	mtcnn_imp::detectMaxFace(imgData.data, height, width, mtcnn_imp::eBGR888, finalBbox);

	const int num_box = finalBbox.size();
	if (num_box == 0){
		add_text(name + " add face error");
		return;
	}

	cv::Rect  imgrect = { finalBbox[0].boundingBox[0],finalBbox[0].boundingBox[1],
		finalBbox[0].boundingBox[2] - finalBbox[0].boundingBox[0] + 1,
		finalBbox[0].boundingBox[3] - finalBbox[0].boundingBox[1] + 1 };


	std::vector<cv::Point2f> five;
	for (int num = 0; num < 5; num++){
		five.push_back(cv::Point2f((float)finalBbox[0].landmark[num], (float)finalBbox[0].landmark[num+5]));
	}
	std::vector<float>  vec;
	if (!face_get_vec(imgData,imgrect, five,vec)){
		add_text(name + " add face error");
		return;
	}
	faceSet[name] = vec;
    add_text(name + " add face ok!");
}

std::string  ImpProc::find_face(std::vector<float> vec, float &cos) {

	auto iter = faceSet.begin();
	float maxcos = -1.0;
	std::string temp;
	std::string  maxname;

	while (iter != faceSet.end()){
		float cos = face_cal_cos(vec, iter->second);
        if (cos > maxcos) {
			maxname = iter->first;
			maxcos = cos;
		}
        ++iter;
	}
	if (maxcos > 0.5) temp = maxname;
	cos = maxcos;
	return temp;
}
void ImpProc::merge_Image(cv::Mat &img, cv::Mat face){
    if(face.channels() == 1)  cvtColor(face, face, cv::COLOR_GRAY2RGB);
    face.copyTo(img(cv::Rect(0,0,face.cols,face.rows)));
}

void ImpProc::draw_Image(cv::Mat img){

	cv::Mat rgb_channel[3];
	split(img, rgb_channel);

	int height = img.rows;
	int width = img.cols;
	uint32_t *out_data = new uint32_t[width * height];

	unsigned int nR;
	unsigned int nG;
	unsigned int nB;
	unsigned int c;

	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++) {
            nR = rgb_channel[0].at<uchar>(h, w);
            nG = rgb_channel[1].at<uchar>(h, w);
            nB = rgb_channel[2].at<uchar>(h, w);
            c = 0xff000000 | (nR << 16) | (nG << 8) | nB;
            out_data[h * width + w] = c;
        }


    ANativeWindow_acquire(showWindow_);
	ANativeWindow_Buffer buf;
	if (ANativeWindow_lock(showWindow_, &buf, nullptr) < 0)  return;
    showImage(&buf, out_data, height, width);
	delete[] out_data;
	ANativeWindow_unlockAndPost(showWindow_);
	ANativeWindow_release(showWindow_);
}


void ImpProc::showImage(ANativeWindow_Buffer *buf, uint32_t* out_data, int h, int w) {

	uint32_t *out = static_cast<uint32_t *>(buf->bits);
	int32_t height = MIN(buf->width, h);
	int32_t width = MIN(buf->height, w);

    for (int32_t y = 0; y < height; y++) {
		for (int32_t x = 0; x < width; x++) {
			out[x] = out_data[y*width + x];
		}
		out += buf->stride;
	}
}

void ImpProc::init_textNotifier() {
	textNotifier.reset(new utils_imp::Notifier());
	textList.reset(new std::list<std::string>());
}

void ImpProc::add_text(std::string  str) {
	if (!textNotifier) return;
	textList->push_back(str);
    textNotifier->notify(0);
}

std::string  ImpProc::get_text() {
	std::string ret = "";
	if (textList->size() <= 0) return ret;
	ret = textList->front();
	textList->pop_front();
	return ret;
}

int  ImpProc::wait(int millisecond) {
	if(!textNotifier) return 0;
	return textNotifier->wait_for(millisecond);
}



