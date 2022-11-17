
#include <iostream>

#include "tdx.h"

namespace tgbot {

Tdx::Tdx(uint32_t api_id, const char *data_dir, const char *api_hash):
	api_id_(api_id),
	data_dir_(data_dir),
	api_hash_(api_hash)
{
	td::ClientManager::execute(td_api::make_object<td_api::setLogVerbosityLevel>(1));
	client_manager_ = std::make_unique<td::ClientManager>();
	client_id_ = client_manager_->create_client_id();
	current_query_id_.store(1, std::memory_order_relaxed);
	SendQuery(td_api::make_object<td_api::getOption>("version"));
}

Tdx::~Tdx(void)
{
	SendQuery(td_api::make_object<td_api::close>());
	Loop(1);
}

void Tdx::Loop(int timeout)
{
	ProcessResponse(client_manager_->receive(timeout));
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
	return overloaded(
		[this](td_api::updateAuthorizationState &up)
		{
			OnAuthStateUpdate(std::move(up.authorization_state_));
		},
		[this](td_api::updateNewChat &up)
		{
			c_chat_.set(up.chat_->id_, *up.chat_);
			InvokeThis(callback_.update_new_chat_, up);
		},
		[this](td_api::updateChatTitle &up)
		{
			InvokeThis(callback_.update_chat_title_, up);
		},
		[this](td_api::updateUser &up)
		{
			c_users_.set(up.user_->id_, *up.user_);
			InvokeThis(callback_.update_user_, up);
		},
		[this](td_api::updateNewMessage &up)
		{
			InvokeThis(callback_.update_new_message_, up);
		},
		[](auto &up){}
	);
}

inline auto Tdx::CreateAuthHandler(void)
{
	return [this](Tdx *tdx, Object obj){
		CheckAuthError(obj);
	};
}

inline void Tdx::CheckAuthError(Object &obj)
{
	if (obj->get_id() == td_api::error::ID) {
		auto err = td::move_tl_object_as<td_api::error>(obj);
		std::cout << "Error: " << to_string(err) << std::flush;
	}
}

inline auto Tdx::__OnAuthStateUpdate(void)
{
	/*
	 * TODO(ammarfaizi2):
	 * We should not be doing these here. Split them into smaller
	 * manageable pieces.
	 */
	return overloaded(
		[this](td_api::authorizationStateReady &)
		{
			is_authorized_ = true;
			std::cout << "Got authorization" << std::endl;
		},
		[this](td_api::authorizationStateLoggingOut &)
		{
			is_authorized_ = false;
			std::cout << "Logging out" << std::endl;
		},
		[](td_api::authorizationStateClosing &)
		{
			std::cout << "Closing" << std::endl;
		},
		[this](td_api::authorizationStateClosed &)
		{
			is_authorized_ = false;
			need_restart_ = true;
			std::cout << "Terminated" << std::endl;
		},
		[this](td_api::authorizationStateWaitPhoneNumber &) {
			std::cout << "Enter phone number: " << std::flush;
			std::string phone_number;
			std::cin >> phone_number;
			SendQuery(td_api::make_object<td_api::setAuthenticationPhoneNumber>(phone_number, nullptr),
				  CreateAuthHandler());
		},
		[this](td_api::authorizationStateWaitEmailAddress &) {
			std::cout << "Enter email address: " << std::flush;
			std::string email_address;
			std::cin >> email_address;
			SendQuery(td_api::make_object<td_api::setAuthenticationEmailAddress>(email_address),
				  CreateAuthHandler());
		},
		[this](td_api::authorizationStateWaitEmailCode &) {
			std::cout << "Enter email authentication code: " << std::flush;
			std::string code;
			std::cin >> code;
			SendQuery(td_api::make_object<td_api::checkAuthenticationEmailCode>(
				  td_api::make_object<td_api::emailAddressAuthenticationCode>(code)),
				  CreateAuthHandler());
		},
		[this](td_api::authorizationStateWaitCode &) {
			std::cout << "Enter authentication code: " << std::flush;
			std::string code;
			std::cin >> code;
			SendQuery(td_api::make_object<td_api::checkAuthenticationCode>(code),
				  CreateAuthHandler());
		},
		[this](td_api::authorizationStateWaitRegistration &) {
			std::string first_name;
			std::string last_name;
			std::cout << "Enter your first name: " << std::flush;
			std::cin >> first_name;
			std::cout << "Enter your last name: " << std::flush;
			std::cin >> last_name;
			SendQuery(td_api::make_object<td_api::registerUser>(first_name, last_name),
				  CreateAuthHandler());
		},
		[this](td_api::authorizationStateWaitPassword &uo) {
			std::cout << to_string(uo) << std::endl;
			std::cout << "Enter authentication password: " << std::flush;
			std::string password;
			std::getline(std::cin, password);
			SendQuery(td_api::make_object<td_api::checkAuthenticationPassword>(password),
				  CreateAuthHandler());
		},
		[](td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
			std::cout << "Confirm this login link on another device: " << state.link_ << std::endl;
		},
		[this](td_api::authorizationStateWaitTdlibParameters &up)
		{
			auto r = td_api::make_object<td_api::setTdlibParameters>();
			r->database_directory_ = data_dir_;
			r->use_message_database_ = true;
			r->use_secret_chats_ = true;
			r->api_id_ = api_id_;
			r->api_hash_ = api_hash_;
			r->system_language_code_ = "en";
			r->device_model_ = "Desktop";
			r->application_version_ = "1.0";
			r->enable_storage_optimizer_ = true;
			SendQuery(std::move(r), CreateAuthHandler());
		}
	);
}

inline
void Tdx::OnAuthStateUpdate(td_api::object_ptr<td_api::AuthorizationState> s)
{
	auth_state_ = std::move(s);
	td_api::downcast_call(*auth_state_, __OnAuthStateUpdate());
}

void Tdx::ProcessUpdate(td_api::object_ptr<td_api::Object> update)
{
	td_api::downcast_call(*update, __ProcessUpdate());
}

} /* namespace tgbot */
