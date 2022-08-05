/********************************************************************************\
 * @file    alog.hpp
 * @brief   Yet another header-only asynchronous logging library base on C++ 20
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
#include <cstddef>
#include <concepts>

/// @ref https://sourceforge.net/p/predef/wiki/OperatingSystems/

#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS_)

#include <windows.h>
#include <wincon.h>

#define ALOG_WINDOWS

#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__ANDROID__)

#include <sys/types.h>
#include <unistd.h>

#define ALOG_UNIX_LIKE

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <type_traits>

#elif defined(macintosh) || defined(Macintosh) || defined(__APPLE__)

#elif defined(sun) || defined(__sun)

#elif defined(__unix__) || defined(__unix)

#else

#endif


namespace alog {

	enum class output_direction_type : char {
		o_stdout = 1,
		o_stderr,
		o_logfile,
	};


	enum class output_ctrl : uint16_t {
#if defined(_WIN32) || defined(_WIN64)
		ft_black = 0,
		ft_red = FOREGROUND_RED,
		ft_green = FOREGROUND_GREEN,
		ft_yellow = FOREGROUND_RED | FOREGROUND_GREEN,
		ft_blue = FOREGROUND_BLUE,
		ft_purple = FOREGROUND_BLUE | FOREGROUND_RED,
		ft_cyan = FOREGROUND_BLUE | FOREGROUND_GREEN,
		ft_white = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
		bg_black = 0,
		bg_red = BACKGROUND_RED,
		bg_green = BACKGROUND_GREEN,
		bg_yellow = BACKGROUND_RED | BACKGROUND_GREEN,
		bg_blue = BACKGROUND_BLUE,
		bg_purple = BACKGROUND_BLUE | BACKGROUND_RED,
		bg_cyan = BACKGROUND_BLUE | BACKGROUND_GREEN,
		bg_white = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,

		ft_intensity = FOREGROUND_INTENSITY,
		bg_intensity = BACKGROUND_INTENSITY,

		reverse = COMMON_LVB_REVERSE_VIDEO,
		underscore = COMMON_LVB_UNDERSCORE,
		blink = 0,
		vanish = 0,
#else 
		ft_black = 0, // 30
		ft_red = 1,
		ft_green = 2,
		ft_yellow = 3,
		ft_blue = 4,
		ft_purple = 5,
		ft_cyan = 6,
		ft_white = 7,

		bg_black = 0x00, // 40
		bg_red = 0x10,
		bg_green = 0x20,
		bg_yellow = 0x30,
		bg_blue = 0x40,
		bg_purple = 0x50,
		bg_cyan = 0x60,
		bg_white = 0x70,

		ft_intensity = 0x0100,
		bg_intensity = 0,

		reverse = 0x0700,
		underscore = 0x0400,
		blink = 0x0500,
		vanish = 0x0800,
#endif
	};

	output_ctrl operator|(const output_ctrl & lhs, const output_ctrl & rhs) {
		return { static_cast<output_ctrl>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs)) };
	}

	output_ctrl & operator|=(output_ctrl& lhs, const output_ctrl& rhs) {
		return lhs = { static_cast<output_ctrl>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs)) };
	}


	enum class level {
		error,
		warn,
		info,
		debug,
	};

	

	void apply_global_args(int argc, char* argv[]);

	struct abstract_output_device {
		
	};

}

namespace alog::details {


#ifdef ALOG_WINDOWS
	template<output_direction_type DT = output_direction_type::o_stderr>
	requires (DT == output_direction_type::o_stdout || DT == output_direction_type::o_stderr)
	struct output_console : abstract_output_device {
		output_console() {
			HANDLE sh = GetStdHandle(
				DT == output_direction_type::o_stderr ?
				STD_ERROR_HANDLE : 
				STD_OUTPUT_HANDLE
			);
			_sh = std::move(sh);
			CONSOLE_SCREEN_BUFFER_INFO sci;
			GetConsoleScreenBufferInfo(_sh, &sci);
			_oc = static_cast<output_ctrl>(sci.wAttributes);
		}

		virtual ~output_console() {
			// no need to release STD_xxxx_HANDLE
		}

#if 0
		DWORD sync_send(const void* data, DWORD len) {
			DWORD len_written;
			if (WriteConsole(_sh, data, len, &len_written, nullptr))
				return len_written;
			return 0;
		}
#endif

		void async_send(const void* data, DWORD len) {
			async_send(data, len, _oc);
		}

		void async_send(const void* data, DWORD len, output_ctrl oc) {
			std::vector<char> buf(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + len);
			std::async([](HANDLE sh, std::vector<char> buf, output_ctrl oc, output_ctrl oc_old) -> DWORD {
				DWORD len_written;
				SetConsoleTextAttribute(sh, static_cast<WORD>(oc));
				auto ret = WriteConsole(sh, buf.data(), buf.size(), &len_written, nullptr);
				SetConsoleTextAttribute(sh, static_cast<WORD>(oc_old));
				return ret ? len_written : 0;
			}, _sh, std::move(std::ref(buf)), oc, _oc);
		}


	private:
		HANDLE _sh;
		output_ctrl _oc;
	};

	template<output_direction_type DT = output_direction_type::o_logfile>
	requires (DT == output_direction_type::o_logfile)
	struct output_logfile : abstract_output_device {

		output_logfile(const char *filename) {
			_file = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (_file == INVALID_HANDLE_VALUE) {
				throw std::string{ "Cannot open log file " } + filename;
			}
			SetFilePointer(_file, 0, 0, FILE_END);
		}

		output_logfile(const std::string & filename) : output_logfile(filename.c_str()) {}

		virtual ~output_logfile() {
			CloseHandle(_file);
		}

#if 0
		DWORD sync_send(const void* data, DWORD len) {
			DWORD len_written;
			if (WriteFile(_file, data, len, &len_written, nullptr))
				return len_written;
			return 0;
		}
#endif

		void async_send(const void* data, DWORD len) {
			std::vector<char> buf(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + len);
			std::async([](HANDLE file, std::vector<char> buf) -> DWORD {
				DWORD len_written;
				auto ret = WriteFile(file, buf.data(), buf.size(), &len_written, nullptr);
				return ret ? len_written : 0;
			}, _file, std::move(std::ref(buf)));
		}

	private:
		HANDLE _file;
	};

#elif defined(ALOG_UNIX_LIKE)
	template<output_direction_type DT>
	requires (DT == output_direction_type::o_stdout || DT == output_direction_type::o_stderr)
	struct output_console : abstract_output_device {
		output_console() {
			_oc = static_cast<output_ctrl>(0); // 0 means no specified style
		}

		virtual ~output_console() {

		}

#if 0
		ssize_t sync_send(const void* data, size_t len) {
			return write(static_cast<int>(DT), data, len);
		}
#endif

		void async_send(const void* data, unsigned int len) {
			async_send(data, len, _oc);
		}

#if 0
		void async_send(const void* data, unsigned int len, output_ctrl oc) {

			std::vector<char> buf(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + len);
			
			std::async([this](std::vector<char> print_buf, output_ctrl oc, output_ctrl oc_old) -> ssize_t {

				auto buf_beg = output_ctrl_to_echo_bytes(oc);
				auto buf_end = output_ctrl_to_echo_bytes(oc_old);
				std::vector<char> buf(print_buf.size() + buf_beg.size() + buf_end.size());

				auto 
				cur = std::copy(buf_beg.cbegin(), buf_beg.cend(), buf.begin());
				cur = std::copy(print_buf.cbegin(), print_buf.cend(), cur);
				cur = std::copy(buf_end.cbegin(), buf_end.cend(), cur);

				ssize_t ssiz = 0;
				ssiz += write(static_cast<int>(DT), buf.data(), buf.size());
				return ssiz;

			}, std::move(std::ref(buf)), oc, _oc);
		}
#endif

		void async_send(const void* data, unsigned int len, output_ctrl oc) {

			std::vector<char> buf(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + len);

			std::async([this](std::vector<char> print_buf, output_ctrl oc, output_ctrl oc_old) -> ssize_t {

				auto buf_beg = output_ctrl_to_echo_bytes(oc);
				auto buf_end = output_ctrl_to_echo_bytes(oc_old);
				std::vector<char> buf = buf_beg;

				std::for_each(print_buf.cbegin(), print_buf.cend(), [&buf, &buf_end, &buf_beg](const auto& e) {
					if (e == '\n') {
						buf.insert(buf.end(), buf_end.cbegin(), buf_end.cend());
						buf.push_back(e);
						buf.insert(buf.end(), buf_beg.cbegin(), buf_beg.cend());
					}
					else {
						buf.push_back(e);
					}
					});

				buf.insert(buf.end(), buf_end.cbegin(), buf_end.cend());

				ssize_t ssiz = 0;
				ssiz += write(static_cast<int>(DT), buf.data(), buf.size());
				return ssiz;

				}, std::move(std::ref(buf)), oc, _oc);
		}

	private:
		output_ctrl _oc;
		std::mutex _m;

	private:
		static std::vector<char> output_ctrl_to_echo_bytes(output_ctrl oc) {
			if (static_cast<uint16_t>(oc) == 0) {
				return { '\e', '[', '0', 'm' };
			} else {
				auto b = ((static_cast<uint16_t>(oc) >> 8) & 0x000F);
				if (b == 2 || b == 3 || b == 6 || b > 8) b = 0;
				return { '\e', '[', 
					'3', '0' + (static_cast<uint16_t>(oc) & 0x000F), ';',
					'4', '0' + ((static_cast<uint16_t>(oc) >> 4) & 0x000F), ';',
					'0' + (b ? b : 1),
					'm',
				};
			}
		}
	};

	template<output_direction_type DT = output_direction_type::o_logfile>
		requires (DT == output_direction_type::o_logfile)
	struct output_logfile : abstract_output_device {

		output_logfile(const char* filename) {
			_file = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
		}

		output_logfile(const std::string& filename) : output_logfile(filename.c_str()) {}

		virtual ~output_logfile() {
			close(_file);
		}

		void async_send(const void* data, DWORD len) {
			std::vector<char> buf(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + len);
			std::async([](int file, std::vector<char> buf) -> ssize_t {
				ssize_t ssiz = 0;
				ssiz += write(file, buf.data(), buf.size());
				return ssiz;
			}, _file, std::move(std::ref(buf)));
		}

	private:
		int _file;
	};
#else
	// not compiled

#endif



}


namespace alog::impl {

	template <level LV, typename Output>
	requires std::derived_from<Output, abstract_output_device>
	struct logger {
		Output _out;

		logger& operator<<(const std::string& rhs) {
			_out.async_send(rhs.c_str(), rhs.length());
			return *this;
		}

		logger& operator<<(const char * rhs) {
			_out.async_send(rhs, strlen(rhs));
			return *this;
		}

		logger& operator<<(char rhs) {
			_out.async_send(&rhs, 1);
			return *this;
		}
	};

}


#endif // __ALOG_HPP_444E89BD_26E4_41F9_A351_5AE6B7FB12E8__