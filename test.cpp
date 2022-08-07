#include "alog.hpp"

using namespace alog;
int main() {
	set_output_threshold(log_level::warn);
	LOGD << "hello, world!";
	LOGI << "hello, world!";
	LOGW << "hello, world!";
	LOGE << "hello, world!";
	LOGF << "hello, world!";
	return 0;
}
