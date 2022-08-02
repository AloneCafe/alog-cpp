#include "alog.hpp"


int main() {
	alog::details::output_console_win32<alog::output_std_type::ost_stderr> o;
	std::string s = "hello, world!\n", t = s, x = t, y = t;
	o.async_send(s.c_str(), s.size());
	for (int s = 10000; s > 0; s--);
	o.async_send(y.c_str(), s.size());
	for (int s = 10000; s > 0; s--);
	o.async_send(x.c_str(), s.size());

	return 0;
}
