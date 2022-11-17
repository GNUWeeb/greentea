
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cstdlib>
#include <signal.h>

#include "Bot.h"

namespace tgbot {

volatile bool g_stop = false;

static void signal_handler(int sig)
{
	printf("\nGot signal: %d\n", sig);
	g_stop = true;
}

static void run_tgbot(void)
{
	const char *data_dir;
	const char *api_hash;
	const char *tmp;
	uint32_t api_id;
	Bot *bot;

	tmp = getenv("TGBOT_API_ID");
	if (!tmp) {
		puts("Missing TGBOT_API_ID!");
		return;
	}
	api_id = static_cast<uint32_t>(atoi(tmp));

	data_dir = getenv("TGBOT_DATA_DIR");
	if (!data_dir) {
		puts("Missing TGBOT_DATA_DIR!");
		return;
	}

	api_hash = getenv("TGBOT_API_HASH");
	if (!api_hash) {
		puts("Missing TGBOT_API_HASH!");
		return;
	}

	bot = new Bot(api_id, data_dir, api_hash);
	bot->Run();
	delete bot;
}

static int setup_signal_handler(void)
{
	struct sigaction act;
	int ret;

	memset(&act, 0, sizeof(act));
	act.sa_handler = signal_handler;

	if (sigaction(SIGINT, &act, NULL))
		goto err;
	if (sigaction(SIGTERM, &act, NULL))
		goto err;
	if (sigaction(SIGHUP, &act, NULL))
		goto err;

	return 0;
err:
	ret = errno;
	printf("sigaction() error: %s\n", strerror(ret));
	return -ret;
}

void tgbot_main(void)
{
	int ret;

	ret = setup_signal_handler();
	if (ret < 0)
		return;

	run_tgbot();
}

} /* namespace tgbot */
