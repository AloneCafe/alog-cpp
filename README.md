# alog-cpp
Yet **A**nother header-only asynchronous **LOG**ging library implemented by **C++ 20**

## Notice
*This repo is incomplete and in need of improvement.*

My goal is going to implement colorful (differentiate by logging level) outputing and the binding of multiple output direction,
the dynamic logging level control to filter the log text you want to record & display, 
and also the handing of command line arguments which could control the behavior of logger.

## Usage
  1. Copy the 'alog.hpp' as you like in your C++ project.
  2. Here is a simple example:
  ```cpp
#include "alog.hpp"

using namespace alog;
int main() {
	set_output_threshold(log_level::debug);
	LOGD << "hello, world!";
	LOGI << "hello, world!";
	LOGW << "hello, world!";
	LOGE << "hello, world!";
	LOGF << "hello, world!";
	return 0;
}
  ```
## Result
![image](https://user-images.githubusercontent.com/20834047/186965817-43cc4e8e-cd98-4092-9d10-166b471aafbc.png)
