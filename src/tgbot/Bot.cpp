
#include <iostream>

#include "Bot.h"

namespace tgbot {

extern volatile bool g_stop;
constexpr static const uint32_t max_work = 2048;
constexpr static const uint32_t max_thread = 1024;
constexpr static const uint32_t min_idle_thread = 5;

Bot::Bot(uint32_t api_id, const char *data_dir, const char *api_hash):
	tdx_(api_id, data_dir, api_hash),
	wq_(max_work, max_thread, min_idle_thread)
{
}

Bot::~Bot(void)
{
}

int Bot::Run(void)
{
	int ret;

	ret = wq_.Init();
	if (ret)
		return ret;

	tdx_.callback_.update_new_message_ = [this](Tdx *tdx, auto up){
		HandleNewMessage(std::move(up));
	};
	return _Run();
}

inline int Bot::_Run(void)
{
	while (!g_stop)
		tdx_.Loop(5);

	return 0;
}

inline void Bot::HandleNewMessage(td_api::updateNewMessage up)
{
	std::cout << to_string(up) << std::endl;
}

} /* namespace tgbot */
