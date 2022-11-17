
#ifndef GT_TGBOT__TD_H
#define GT_TGBOT__TD_H

#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#include <map>
#include <mutex>
#include <atomic>
#include <cstdint>
#include <functional>
#include <condition_variable>

namespace td_api = td::td_api;

namespace tgbot {

namespace detail {

template <class... Fs>
struct overload;

template <class F>
struct overload<F>: public F {
	explicit overload(F f):
		F(f)
	{
	}
};

template <class F, class... Fs>
struct overload<F, Fs...>: public overload<F>, public overload<Fs...> {
	overload(F f, Fs... fs):
		overload<F>(f), overload<Fs...>(fs...)
	{
	}

	using overload<F>::operator();
	using overload<Fs...>::operator();
};

} /* namespace detail */

template <class... F>
auto overloaded(F... f)
{
	return detail::overload<F...>(f...);
}

class Tdx;

class TdxCallback {
public:
	template<typename T>
	using Func = std::function<void(Tdx *, T)>;

	Func<td_api::updateNewChat> update_new_chat_ = nullptr;
	Func<td_api::updateChatTitle> update_chat_title_ = nullptr;
	Func<td_api::updateUser> update_user_ = nullptr;
};

class Tdx {
public:
	using Object = td_api::object_ptr<td_api::Object>;

	Tdx(void);
	~Tdx(void);
	void SendQuery(td_api::object_ptr<td_api::Function> f,
		       std::function<void(Tdx *, Object)> handler);

private:
	std::unique_ptr<td::ClientManager> client_manager_;
	std::atomic<uint64_t> current_query_id_;
	int32_t client_id_;

	std::mutex handlers_lock_;
	std::map<uint64_t, std::function<void(Tdx *, Object)>> handlers_;
	td_api::object_ptr<td_api::AuthorizationState> auth_state_;

	TdxCallback callback_;

	inline uint64_t next_query_id(void)
	{
		return current_query_id_.fetch_add(1, std::memory_order_acq_rel);
	}

	template<typename T, typename U>
	__attribute__((__always_inline__))
	inline void InvokeThis(T &callback, U &up)
	{
		if (callback)
			callback(this, std::move(up));
	}

	void ProcessResponse(td::ClientManager::Response response);
	void ProcessUpdate(td_api::object_ptr<td_api::Object> update);
	auto __ProcessUpdate(void);
	void OnAuthStateUpdate(td_api::object_ptr<td_api::AuthorizationState> s);
};

} /* namespace tgbot */

#endif /* #ifndef GT_TGBOT__TD_H */
