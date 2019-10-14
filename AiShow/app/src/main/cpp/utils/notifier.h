#pragma once

#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__

#include <condition_variable>
#include<list>

namespace utils_imp {
	class Notifier {
		std::condition_variable cv;
		std::mutex cv_m;
		std::mutex list_m;
		std::list<int> _list;

		int _get_front() {
			std::lock_guard<std::mutex> guard(list_m);
			if (_list.size() > 0) {
				int ret = _list.front();
				_list.pop_front();
				return ret;
			}
			return -1;
		}
	public:
		int wait_for(int timeout) {
			int ret = _get_front();
			if (ret >= 0) return ret;
			std::unique_lock<std::mutex> lk(cv_m);
			if (cv.wait_for(lk, std::chrono::milliseconds(timeout)) == std::cv_status::no_timeout) {
				return _get_front();
			}
			else return -1;
		}
		void notify(int index) {
			list_m.lock();
			_list.push_back(index);
			list_m.unlock();
			cv.notify_all();
		}
	};
}

#endif