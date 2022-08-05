#include "alog.hpp"

using namespace alog;
int main() {
	//alog::details::output_console<alog::output_direction_type::o_stderr> o;
	//std::string s = "hello, world!\n ", t = s, x = t, y = t;
	////o.async_send(s.c_str(), s.size(), alog::output_ctrl::bg_red | alog::output_ctrl::ft_intensity | alog::output_ctrl::ft_white);
	//for (int s = 10000; s > 0; s--);
	//o.async_send(y.c_str(), s.size(), alog::output_ctrl::bg_green);
	//for (int s = 10000; s > 0; s--);
	//o.async_send(x.c_str(), s.size(), alog::output_ctrl::bg_red);
	//for (int s = 10000; s > 0; s--);
	//o.async_send(x.c_str(), s.size());
	//alog::impl::logger<level::error, details::output_console<output_direction_type::o_stderr> > log;
	alog::impl::logger<level::error, details::output_logfile<output_direction_type::o_logfile> > log("G:\\a.log");
	log << std::string{ "hello, world!" } << '\n';
	return 0;
}
