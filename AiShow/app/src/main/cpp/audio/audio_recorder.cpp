/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include <cstdlib>
#include "audio_recorder.h"
/*
 * bqRecorderCallback(): called for every buffer is full;
 *                       pass directly to handler
 */
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *rec) {
  (static_cast<AudioRecorder *>(rec))->ProcessSLCallback(bq);
}

/*void AudioRecorder::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
#ifdef ENABLE_LOG
  recLog_->logTime();
#endif
  assert(bq == recBufQueueItf_);
  sample_buf *dataBuf = NULL;
  devShadowQueue_->front(&dataBuf);
  devShadowQueue_->pop();
  dataBuf->size_ = dataBuf->cap_;  // device only calls us when it is really
                                   // full

  //callback_(ctx_, ENGINE_SERVICE_MSG_RECORDED_AUDIO_AVAILABLE, dataBuf);

  callback_(callbackCtx_,dataBuf );
  recQueue_->push(dataBuf);

  sample_buf *freeBuf;
  while (freeQueue_->front(&freeBuf) && devShadowQueue_->push(freeBuf)) {
    freeQueue_->pop();
    SLresult result = (*bq)->Enqueue(bq, freeBuf->buf_, freeBuf->cap_);
    SLASSERT(result);
  }

  ++audioBufCount;

  // should leave the device to sleep to save power if no buffers
  if (devShadowQueue_->size() == 0) {
    (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
  }
}*/

void AudioRecorder::ProcessSLCallback(SLAndroidSimpleBufferQueueItf bq) {
#ifdef ENABLE_LOG
    recLog_->logTime();
#endif
    assert(bq == recBufQueueItf_);
    callback_(callbackCtx_, &m_bufs_[0]);
    SLresult result = (*bq)->Enqueue(bq, m_bufs_[0].buf_, m_bufs_[0].cap_);
    //LOGI("audio buf  %d  %d",m_bufs_[0].cap_,m_bufs_[0].size_);
    SLASSERT(result);
}

AudioRecorder::AudioRecorder(SampleFormat *sampleFormat, SLEngineItf slEngine)
    : freeQueue_(nullptr),
      recQueue_(nullptr),
      devShadowQueue_(nullptr),
      callback_(nullptr) {
  SLresult result;
  sampleInfo_ = *sampleFormat;
  SLAndroidDataFormat_PCM_EX format_pcm;
  ConvertToSLSampleFormat(&format_pcm, &sampleInfo_);

  LOGI("audioRecorder  %d  %d  %d   %d   %d   %d ",
       format_pcm.bitsPerSample,
       format_pcm.formatType,
       format_pcm.containerSize,
       format_pcm.sampleRate,
       format_pcm.channelMask,
       format_pcm.numChannels
  );
  // configure audio source
  SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE,
                                    SL_IODEVICE_AUDIOINPUT,
                                    SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};
  SLDataSource audioSrc = {&loc_dev, NULL};

  // configure audio sink
  SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
          SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, DEVICE_SHADOW_BUFFER_QUEUE_LEN};

  SLDataSink audioSnk = {&loc_bq, &format_pcm};

  // create audio recorder
  // (requires the RECORD_AUDIO permission)
  const SLInterfaceID id[2] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                               SL_IID_ANDROIDCONFIGURATION};
  const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
  result = (*slEngine)->CreateAudioRecorder(
          slEngine, &recObjectItf_, &audioSrc, &audioSnk,
          sizeof(id) / sizeof(id[0]), id, req);
  //LOGI("audiorecoder1 %d ",result);
  SLASSERT(result);

  // Configure the voice recognition preset which has no
  // signal processing for lower latency.
  SLAndroidConfigurationItf inputConfig;
  result = (*recObjectItf_)
          ->GetInterface(recObjectItf_, SL_IID_ANDROIDCONFIGURATION,
                         &inputConfig);
  if (SL_RESULT_SUCCESS == result) {
    SLuint32 presetValue = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
    (*inputConfig)
            ->SetConfiguration(inputConfig, SL_ANDROID_KEY_RECORDING_PRESET,
                               &presetValue, sizeof(SLuint32));
  }
  result = (*recObjectItf_)->Realize(recObjectItf_, SL_BOOLEAN_FALSE);
  //LOGI("audiorecoder2 %d ",result);
  SLASSERT(result);
  result =
          (*recObjectItf_)->GetInterface(recObjectItf_, SL_IID_RECORD, &recItf_);
  //LOGI("audiorecoder3 %d ",result);
  SLASSERT(result);

  result = (*recObjectItf_)
          ->GetInterface(recObjectItf_, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                         &recBufQueueItf_);
  //LOGI("audiorecoder4 %d ",result);
  SLASSERT(result);

  result = (*recBufQueueItf_)
          ->RegisterCallback(recBufQueueItf_, bqRecorderCallback, this);
  //LOGI("audiorecoder5 %d ",result);
  SLASSERT(result);

  uint32_t bufSize =
          sampleFormat->framesPerBuf_ * sampleFormat->channels_ * SL_PCMSAMPLEFORMAT_FIXED_16;
  bufSize = (bufSize + 7) >> 3;  // bits --> byte
  m_bufs_ = allocateSampleBufs(1, bufSize);

  LOGI("audioRecored buf  %d   cap = %d",bufSize, m_bufs_[0].cap_);


  //devShadowQueue_ = new AudioQueue(DEVICE_SHADOW_BUFFER_QUEUE_LEN);
  //assert(devShadowQueue_);
#ifdef ENABLE_LOG
  std::string name = "rec";
  recLog_ = new AndroidLog(name);
#endif
}


SLboolean AudioRecorder::Start(void) {

  SLresult result;
  // in case already recording, stop recording and clear buffer queue
  result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
  SLASSERT(result);
  result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
  SLASSERT(result);

  result = (*recBufQueueItf_)->Enqueue(recBufQueueItf_, m_bufs_[0].buf_, m_bufs_[0].cap_);
  SLASSERT(result);
  result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_RECORDING);
  SLASSERT(result);
  return (result == SL_RESULT_SUCCESS ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE);
}

/*SLboolean AudioRecorder::Start(void) {
  if (!freeQueue_ || !recQueue_ || !devShadowQueue_) {
    LOGE("====NULL poiter to Start(%p, %p, %p)", freeQueue_, recQueue_,
         devShadowQueue_);
    return SL_BOOLEAN_FALSE;
  }
  audioBufCount = 0;

  SLresult result;
  // in case already recording, stop recording and clear buffer queue
  result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
  SLASSERT(result);
  result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
  SLASSERT(result);

  for (int i = 0; i < RECORD_DEVICE_KICKSTART_BUF_COUNT; i++) {
    sample_buf *buf = NULL;
    if (!freeQueue_->front(&buf)) {
      LOGE("=====OutOfFreeBuffers @ startingRecording @ (%d)", i);
      break;
    }
    freeQueue_->pop();
    assert(buf->buf_ && buf->cap_ && !buf->size_);

    result = (*recBufQueueItf_)->Enqueue(recBufQueueItf_, buf->buf_, buf->cap_);
    SLASSERT(result);
    devShadowQueue_->push(buf);
  }

  result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_RECORDING);
  SLASSERT(result);

  return (result == SL_RESULT_SUCCESS ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE);
}*/

SLboolean AudioRecorder::Stop(void) {
  // in case already recording, stop recording and clear buffer queue
  SLuint32 curState;

  SLresult result = (*recItf_)->GetRecordState(recItf_, &curState);
  SLASSERT(result);
  if (curState == SL_RECORDSTATE_STOPPED) {
    return SL_BOOLEAN_TRUE;
  }
  result = (*recItf_)->SetRecordState(recItf_, SL_RECORDSTATE_STOPPED);
  SLASSERT(result);
  result = (*recBufQueueItf_)->Clear(recBufQueueItf_);
  SLASSERT(result);

#ifdef ENABLE_LOG
  recLog_->flush();
#endif

  return SL_BOOLEAN_TRUE;
}

AudioRecorder::~AudioRecorder() {
  // destroy audio recorder object, and invalidate all associated interfaces
  if (recObjectItf_ != NULL) {
    (*recObjectItf_)->Destroy(recObjectItf_);
  }

  if (devShadowQueue_) {
    sample_buf *buf = NULL;
    while (devShadowQueue_->front(&buf)) {
      devShadowQueue_->pop();
      freeQueue_->push(buf);
    }
    delete (devShadowQueue_);
  }

  uint32_t count = 1;
  if(m_bufs_)    releaseSampleBufs(m_bufs_, count);

#ifdef ENABLE_LOG
  if (recLog_) {
    delete recLog_;
  }
#endif
}

void AudioRecorder::SetBufQueues(AudioQueue *freeQ, AudioQueue *recQ) {
  assert(freeQ && recQ);
  freeQueue_ = freeQ;
  recQueue_ = recQ;
}

/*void AudioRecorder::RegisterCallback(ENGINE_CALLBACK cb, void *ctx) {
  callback_ = cb;
  ctx_ = ctx;
}*/

void AudioRecorder::RegisterCallback(void* ctx,std::function<void(void *ctx, sample_buf *data)> func) {
    callbackCtx_ = ctx;
    callback_ = func;
}

int32_t AudioRecorder::dbgGetDevBufCount(void) {
  return devShadowQueue_->size();
}
