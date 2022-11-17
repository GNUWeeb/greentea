
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
#include <unordered_map>
#include <condition_variable>

namespace td_api = td::td_api;
using Object = td_api::object_ptr<td_api::Object>;

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
		overload<F>(f),
		overload<Fs...>(fs...)
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
	Func<td_api::updateNewMessage> update_new_message_ = nullptr;
};

template<typename T, typename U>
class TdxMap {
public:
	inline void Lock(void)
	{
		lock_.lock();
	}

	inline void Unlock(void)
	{
		lock_.unlock();
	}

	inline std::mutex *GetLock(void)
	{
		return &lock_;
	}

	inline std::unordered_map<T, U> *GetMap(void)
	{
		return &map_;
	}

	inline void set(const T &key, U &val)
	{
		Lock();
		map_[key] = std::move(val);
		Unlock();
	}

	inline U *get(const T &key)
	{
		U *ret = nullptr;

		Lock();
		auto it = map_.find(key);
		if (it != map_.end())
			ret = &it->second;
		Unlock();

		return ret;
	}

private:
	std::unordered_map<T, U>	map_;
	std::mutex			lock_;
};

class Tdx {
public:
	using HFunc = std::function<void(Tdx *, Object)>;
	using Object = td_api::object_ptr<td_api::Object>;
	template<typename T>
	using ObjPtr = td_api::object_ptr<T>;

	Tdx(uint32_t api_id, const char *data_dir, const char *api_hash);

	~Tdx(void);

	void SendQuery(td_api::object_ptr<td_api::Function> f,
		       std::function<void(Tdx *, Object)> handler = nullptr);

	Object SendQuerySync(td_api::object_ptr<td_api::Function> f);

	void Loop(int timeout = 1);

	TdxCallback				callback_;
private:
	std::atomic<uint64_t>			current_query_id_;
	std::unique_ptr<td::ClientManager>	client_manager_;
	std::mutex				handlers_lock_;
	std::map<uint64_t, HFunc>		handlers_;
	ObjPtr<td_api::AuthorizationState>	auth_state_;
	int32_t					client_id_;
	uint32_t				api_id_;
	const char				*data_dir_;
	const char				*api_hash_;
	bool					is_authorized_ = false;
	bool					need_restart_ = false;

	TdxMap<int64_t, td_api::chat>		c_chat_;
	TdxMap<int64_t, td_api::user>		c_users_;

	inline uint64_t query_id(void)
	{
		return current_query_id_.load(std::memory_order_acquire);
	}

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
	auto __OnAuthStateUpdate(void);
	auto CreateAuthHandler(void);
	void CheckAuthError(Object &object);
};

} /* namespace tgbot */

#endif /* #ifndef GT_TGBOT__TD_H */
