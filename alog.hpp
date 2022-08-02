/********************************************************************************\
 * @file    alog.hpp
 * @brief   Yet another header-only C++ asynchronous logging library
 * 
 * @author  AloneCafe
 * @license MIT
 \*******************************************************************************/



/********************************************************************************\
 MIT License

 Copyright (c) 2022 Alone Cafe

 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
\********************************************************************************/

#ifndef __ALOG_HPP_444E89BD_26E4_41F9_A351_5AE6B7FB12E8__
#define __ALOG_HPP_444E89BD_26E4_41F9_A351_5AE6B7FB12E8__

#include <iostream>
#include <vector>
#include <future>

namespace alog {

	enum class output_std_type {
		ost_stderr,
		ost_stdout,
	};

	enum class level {
		error,
		warn,
		info,
		debug,
	};

	//template <output_std_type st, typename OutputDevice = alog::details::output_device<st>>
	//struct out {
	//	OutputDevice& _odev;

	//	out(OutputDevice& odev) : _odev(odev) {}

	//};

	namespace impl {

	}

	void apply_global_args(int argc, char* argv[]);

}

namespace alog::details {

#if defined(_WIN32) || defined(_WIN64) 
	#include <windows.h>
#else

#endif


	template<output_std_type st>
	struct output_console_win32 {
		output_console_win32() {
			HANDLE sh = GetStdHandle(
				st == output_std_type::ost_stderr ? 
				STD_ERROR_HANDLE : 
				STD_OUTPUT_HANDLE
			);
			_sh = std::move(sh);
		}

		virtual ~output_console_win32() {
			// no need to release STD_xxxx_HANDLE
		}

		DWORD sync_send(const void *data, DWORD len) {
			DWORD len_written;
			if (WriteConsole(_sh, data, len, &len_written, nullptr))
				return len_written;
			return 0;
		}

		void async_send(const void* data, DWORD len) {
			std::vector<char> buf(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + len);
			std::async([](HANDLE sh, std::vector<char> buf) -> DWORD {
				DWORD len_written;
				for (int s = 10000000; s > 0; s--);
				if (WriteConsole(sh, buf.data(), buf.size(), &len_written, nullptr))
					return len_written;
				return 0;
			}, _sh, std::move(std::ref(buf)));
		}

	private:
		HANDLE _sh;
		std::string buf;
	};

	template<output_std_type st>
	struct output_console_unix {
	
	};

	template<output_std_type st>
	using output_device =
#if defined(_WIN32) || defined(_WIN64) 
		output_console_win32<st>;
#else
		output_console_unix<st>;
#endif
}

#endif // __ALOG_HPP_444E89BD_26E4_41F9_A351_5AE6B7FB12E8__