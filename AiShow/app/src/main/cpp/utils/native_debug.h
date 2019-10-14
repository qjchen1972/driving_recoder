#pragma once

#ifndef __NATIVE_DEBUG_H__
#define __NATIVE_DEBUG_H__

#include <android/log.h>
#include <sstream>


namespace std {
    template<class T>
    std::string to_string(T value) {
        std::ostringstream os;
        os << value;
        return os.str();
    }
}

#define LOG_TAG "aiShow"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ASSERT(cond, fmt, ...)                                \
  if (!(cond)) {                                              \
    __android_log_assert(#cond, LOG_TAG, fmt, ##__VA_ARGS__); \
  }

#endif



