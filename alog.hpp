/********************************************************************************\
 * @file    alog.hpp
 * @brief   Yet another header-only asynchronous logging library base on C++ 20
 *
 * @author  AloneCafe
 *
 *
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
#include <type_traits>
#include <concepts>
#include <sstream>
#include <cstring>
#include <ctime>
#include <algorithm>

 /// @ref https://sourceforge.net/p/predef/wiki/OperatingSystems/

#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS_)

#include <windows.h>
#include <wincon.h>

#define ALOG_WINDOWS

int clock_gettime(int, struct timespec *spec)      //C-file part
{
    constexpr __int64 n1 = 116444736000000000;
    constexpr __int64 n2 = 10000000;
    __int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime      -= n1;  //1jan1601 to 1jan1970
    spec->tv_sec  = wintime / n2;           //seconds
    spec->tv_nsec = wintime % n2 * 100;      //nano-seconds
    return 0;
}

#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__ANDROID__)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define ALOG_UNIX_LIKE

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define ALOG_UNIX_LIKE

#elif defined(macintosh) || defined(Macintosh) || defined(__APPLE__)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define ALOG_UNIX_LIKE

#elif defined(sun) || defined(__sun)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define ALOG_UNIX_LIKE

#elif defined(__unix__) || defined(__unix)

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define ALOG_UNIX_LIKE

#else

#error cannot compile on an unsupported operating system

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

	output_ctrl operator|(const output_ctrl& lhs, const output_ctrl& rhs) {
		return { static_cast<output_ctrl>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs)) };
	}

	output_ctrl& operator|=(output_ctrl& lhs, const output_ctrl& rhs) {
		return lhs = { static_cast<output_ctrl>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs)) };
	}

	enum log_level : uint16_t {
		fatal,
		error,
		warn,
		info,
		debug,

		// append here


		dummy,
	};
	bool __output_level_disabled[log_level::dummy - log_level::fatal] = { false };



	void apply_global_args(int argc, char* argv[]);

	struct __output_device {};
	struct rich_style_output_device : __output_device {};
	struct plain_style_output_device : __output_device {};


	output_ctrl log_body_output_ctrl_config[] = {
		output_ctrl::ft_red | output_ctrl::underscore,
		output_ctrl::ft_red | output_ctrl::ft_intensity,
		output_ctrl::ft_yellow ,
		output_ctrl::ft_white,
		output_ctrl::ft_green,
		
	};

	output_ctrl log_head_output_ctrl_config[] = {
		output_ctrl::reverse | output_ctrl::ft_red,
		output_ctrl::reverse | output_ctrl::ft_red | output_ctrl::ft_intensity,
		output_ctrl::reverse | output_ctrl::ft_yellow | output_ctrl::ft_intensity,
		output_ctrl::reverse | output_ctrl::ft_white | output_ctrl::ft_intensity,
		output_ctrl::reverse | output_ctrl::ft_green | output_ctrl::ft_intensity,
	};

	inline output_ctrl ALOG_LEVEL_BODY_OUTPUT_CTRL(log_level lv) {
		return log_body_output_ctrl_config[static_cast<uint16_t>(lv)];
	}

	inline output_ctrl ALOG_LEVEL_HEAD_OUTPUT_CTRL(log_level lv) {
		return log_head_output_ctrl_config[static_cast<uint16_t>(lv)];
	}
}

namespace alog::details {


#ifdef ALOG_WINDOWS
	template<output_direction_type DT = output_direction_type::o_stderr>
		requires (DT == output_direction_type::o_stdout || DT == output_direction_type::o_stderr)
	struct output_console : rich_style_output_device {
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
	struct output_logfile : plain_style_output_device {

		output_logfile(const char* filename) {
			_file = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (_file == INVALID_HANDLE_VALUE) {
				throw std::string{ "Cannot open log file " } + filename;
			}
			SetFilePointer(_file, 0, 0, FILE_END);
		}

		output_logfile(const std::string& filename) : output_logfile(filename.c_str()) {}

		virtual ~output_logfile() {
			CloseHandle(_file);
		}


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
	struct output_console : rich_style_output_device {
		output_console() {
			_oc = static_cast<output_ctrl>(0); // 0 means no specified style
		}

		virtual ~output_console() {

		}


		void async_send(const void* data, unsigned int len) {
			async_send(data, len, _oc);
		}


		void async_send(const void* data, unsigned int len, output_ctrl oc) {

			std::vector<char> buf(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + len);
			const static char blk[] = { ' ', ' ', ' ', ' ' };
			std::async([this](std::vector<char> print_buf, output_ctrl oc, output_ctrl oc_old) -> ssize_t {

				auto buf_beg = output_ctrl_to_echo_bytes(oc);
				auto buf_end = output_ctrl_to_echo_bytes(oc_old);
				std::vector<char> buf = buf_beg;

				std::for_each(print_buf.cbegin(), print_buf.cend(), [&buf, &buf_end, &buf_beg, &print_buf](const auto& e) {
					if (e == '\n') {
						buf.insert(buf.end(), buf_end.cbegin(), buf_end.cend());
						buf.push_back(e);
						buf.insert(buf.end(), buf_beg.cbegin(), buf_beg.cend());
					} else {
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
					'3', static_cast<char>('0' + (static_cast<uint16_t>(oc) & 0x000F)), ';',
					'4', static_cast<char>('0' + ((static_cast<uint16_t>(oc) >> 4) & 0x000F)), ';',
					static_cast<char>('0' + (b ? b : 1)),
					'm',
				};
			}
		}
	};

	template<output_direction_type DT = output_direction_type::o_logfile>
		requires (DT == output_direction_type::o_logfile)
	struct output_logfile : plain_style_output_device {

		output_logfile(const char* filename) {
			_file = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
		}

		output_logfile(const std::string& filename) : output_logfile(filename.c_str()) {}

		virtual ~output_logfile() {
			close(_file);
		}

		void async_send(const void* data, size_t len) {
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

	template <log_level LV, typename Output>
        requires std::derived_from<Output, __output_device>
	struct log_printer;

	template <log_level LV, typename Output>
		requires std::derived_from<Output, __output_device>
	struct log_printer_base {
		friend struct log_printer<LV, Output>;
	protected:
		Output _out;

		log_printer_base() : has_head_printed(false) {
		}

	public:
		virtual ~log_printer_base() {
			if (silenced()) { // has been silenced
				return;
			}

			// if not, output CR
			_out.async_send("\n", 1);
		}

		bool silenced() const {
			return __output_level_disabled[static_cast<uint16_t>(LV)];
		}

	public:
		virtual log_printer_base& operator<<(const std::string_view & rhs) {

			if (silenced()) { // has been silenced
				return *this;
			}

			auto tim = get_head_time_string(); 
			auto sign = get_head_sign_string();
			if (!has_head_printed) {

				if constexpr (std::derived_from<Output, rich_style_output_device>) {
					_out.async_send(tim.data(), strlen(tim.data()), ALOG_LEVEL_BODY_OUTPUT_CTRL(LV));
				} else if constexpr (std::derived_from<Output, plain_style_output_device>) {
					_out.async_send(tim.data(), strlen(tim.data()));
				} else {
					_out.async_send(tim.data(), strlen(tim.data()));
				}
				_out.async_send(" ", 1);

				if constexpr (std::derived_from<Output, rich_style_output_device>) {
					_out.async_send(sign.data(), sign.length(), ALOG_LEVEL_HEAD_OUTPUT_CTRL(LV));
				} else if constexpr (std::derived_from<Output, plain_style_output_device>) {
					_out.async_send(sign.data(), sign.length());
				} else {
					_out.async_send(sign.data(), sign.length());
				}
				_out.async_send(": ", 2);

				

				has_head_printed = true;
			}


			if constexpr (std::derived_from<Output, rich_style_output_device>) {
				_out.async_send(rhs.data(), rhs.length(), ALOG_LEVEL_BODY_OUTPUT_CTRL(LV));
			} else if constexpr (std::derived_from<Output, plain_style_output_device>) {
				_out.async_send(rhs.data(), rhs.length());
			} else {
				_out.async_send(rhs.data(), rhs.length());
			}

			return *this;
		}

		virtual log_printer_base& operator<<(const char* rhs) {
			return *this << std::string_view(rhs);
		}

		virtual log_printer_base& operator<<(char rhs) {
			return *this << std::string_view(&rhs, &rhs + 1);
		}

	private:
		bool has_head_printed;

		std::vector<char> get_head_time_string() {
			std::vector<char> buffer(80);
			timespec ts {  };
            clock_gettime(0, &ts);
			strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S", localtime(&ts.tv_sec));
			sprintf(buffer.data(), "%s.%09ld", buffer.data(), ts.tv_nsec);
			return buffer;
		}

		std::string_view get_head_sign_string() {
			std::string_view meta;
			
			if constexpr (LV == log_level::fatal) {
				meta = "[!]";
			} else if constexpr (LV == log_level::error) {
				meta = "[E]";
			} else if constexpr (LV == log_level::warn) {
				meta = "[W]";
			} else if constexpr (LV == log_level::info) {
				meta = "[I]";
			} else if constexpr (LV == log_level::debug) {
				meta = "[D]";
			} else {
				meta = "[*]";
			}

			return meta;
		}
	};

	struct log_printer_dummy {
		template <typename T>
		log_printer_dummy & operator<<(T) {
			return *this;
		}
	};

	template <log_level LV, typename Output>
		requires std::derived_from<Output, __output_device>
	struct log_printer {
		
		using base = log_printer_base<LV, Output>;

	public:
		base operator()() {
			return base{};
		}
	};

}

namespace alog {

	impl::log_printer<log_level::fatal, details::output_console<output_direction_type::o_stderr> > _log_FATAL;
	impl::log_printer<log_level::error, details::output_console<output_direction_type::o_stderr> > _log_ERROR;
	impl::log_printer<log_level::warn,  details::output_console<output_direction_type::o_stderr> > _log_WARN;
	impl::log_printer<log_level::info,  details::output_console<output_direction_type::o_stderr> > _log_INFO;
	impl::log_printer<log_level::debug, details::output_console<output_direction_type::o_stderr> > _log_DEBUG;

	void set_output_threshold(log_level disable_above) {
		uint16_t beg = static_cast<uint16_t>(disable_above);
		uint16_t end = static_cast<uint16_t>(log_level::dummy);
		for (auto i = beg + 1; i < end; ++i) {
			__output_level_disabled[i] = true;
		}
	}
}

#define LOG(X) alog::_log_##X()

#define LOGF   alog::_log_FATAL()
#define LOGE   alog::_log_ERROR()
#define LOGW   alog::_log_WARN()
#define LOGI   alog::_log_INFO()
#define LOGD   alog::_log_DEBUG()

#define LOG_FATAL  alog::_log_FATAL()
#define LOG_ERROR  alog::_log_ERROR()
#define LOG_ERR    alog::_log_ERROR()
#define LOG_WARN   alog::_log_WARN()
#define LOG_INFO   alog::_log_INFO()
#define LOG_DEBUG  alog::_log_DEBUG()
#define LOG_DBG    alog::_log_DEBUG()

#endif // __ALOG_HPP_444E89BD_26E4_41F9_A351_5AE6B7FB12E8__