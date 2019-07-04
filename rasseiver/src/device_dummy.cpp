#include "device_dummy.hpp"

#include <chrono>
#include <numeric>

DummyDevice::DummyDevice(int count): count {count} {
}

DummyDevice::~DummyDevice() {
	stop();
}

void DummyDevice::receive(Process<int16_t> &process) {
	running = true;
	thd = std::thread {
		[&]() {
			std::array<int16_t,
				   DummyDevice::device_bufsize * 2> data;
			int count {0};

			while (running &&
			       (count++ < DummyDevice::count)) {

				std::iota(data.begin(), data.end(), 0);
				process.apply(data.data(), data.size());

				std::this_thread::sleep_for(
					std::chrono::milliseconds {26});
			}

			running = false;
		}
	};
}

void DummyDevice::stop() {
	if (running || thd.joinable()) {
		running = false;
		thd.join();
	}
}

size_t DummyDevice::buffer_size() {
	return DummyDevice::device_bufsize;
}

int DummyDevice::max_value() {
	return 4096;
}

bool DummyDevice::is_streaming() {
	return running;
}
