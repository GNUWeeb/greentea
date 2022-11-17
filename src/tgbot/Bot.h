
#ifndef GT_TGBOT__BOT_H
#define GT_TGBOT__BOT_H

#include <workqueue2/WorkQueue.h>

#include "tdx.h"

namespace tgbot {

class Bot {
public:
	Bot(uint32_t api_id, const char *data_dir, const char *api_hash);

	~Bot(void);

	int Run(void);

	inline Tdx *tdx(void)
	{
		return &tdx_;
	}

	inline Wq::WorkQueue *wq(void)
	{
		return &wq_;
	}

private:
	Tdx			tdx_;
	Wq::WorkQueue		wq_;

	int _Run(void);
	void HandleNewMessage(td_api::updateNewMessage up);
};

} /* namespace tgbot */

#endif /* #ifndef GT_TGBOT__BOT_H */
