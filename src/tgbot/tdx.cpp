
#include "tdx.h"

namespace tgbot {

Tdx::Tdx(void)
{
	td::ClientManager::execute(td_api::make_object<td_api::setLogVerbosityLevel>(1));
	client_manager_ = std::make_unique<td::ClientManager>();
	client_id_ = client_manager_->create_client_id();
	current_query_id_.store(0, std::memory_order_release);
}

Tdx::~Tdx(void)
{
}

void Tdx::SendQuery(td_api::object_ptr<td_api::Function> f,
		    std::function<void(Tdx *, Object)> handler)
{
	uint64_t query_id;

	query_id = next_query_id();
	if (handler) {
		handlers_lock_.lock();
		handlers_.emplace(query_id, std::move(handler));
		handlers_lock_.unlock();
	}

	client_manager_->send(client_id_, query_id, std::move(f));
}

Object Tdx::SendQuerySync(td_api::object_ptr<td_api::Function> f)
{
	volatile bool done = false;
	Object ret = nullptr;
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);
	std::condition_variable cond;

	std::function<void(Tdx *, Object)> callback =
		[&mutex, &ret, &cond, &done](Tdx *, Object obj)
		{
			mutex.lock();
			ret = std::move(obj);
			done = true;
			cond.notify_one();
			mutex.unlock();
		};

	SendQuery(std::move(f), std::move(callback));
	while (!done)
		cond.wait(lock);

	return ret;
}

void Tdx::ProcessResponse(td::ClientManager::Response res)
{
	if (!res.object)
		return;

	if (res.request_id == 0) {
		ProcessUpdate(std::move(res.object));
		return;
	}

	handlers_lock_.lock();
	auto it = handlers_.find(res.request_id);
	if (it != handlers_.end()) {
		handlers_lock_.unlock();
		it->second(this, std::move(res.object));
		handlers_lock_.lock();
		handlers_.erase(it);
	}
	handlers_lock_.unlock();
}

inline auto Tdx::__ProcessUpdate(void)
{
	using td_api::updateAuthorizationState;
	using td_api::updateNewChat;
	using td_api::updateChatTitle;

	return overloaded(
		[this](updateAuthorizationState &up)
		{
			OnAuthStateUpdate(std::move(up.authorization_state_));
		},
		[this](updateNewChat &up)
		{
			InvokeThis(callback_.update_new_chat_, up);
		},
		[this](updateChatTitle &up)
		{
			InvokeThis(callback_.update_chat_title_, up);
		},
		[this](td_api::updateUser &up)
		{
			InvokeThis(callback_.update_user_, up);
		},
		[](auto &up){}
	);
}

inline
void Tdx::OnAuthStateUpdate(td_api::object_ptr<td_api::AuthorizationState> s)
{
	auth_state_ = std::move(s);
}

void Tdx::ProcessUpdate(td_api::object_ptr<td_api::Object> update)
{
	td_api::downcast_call(*update, __ProcessUpdate());
}

} /* namespace tgbot */
