/*
Copyright 2012 - Le Padellec Sylvain

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "UserMessageHookListener.h"

UserMessageHookListener::ListenersList_t UserMessageHookListener::m_listeners;
bool UserMessageHookListener::bypass(false);
SourceSdk::bf_write* UserMessageHookListener::m_buffer(nullptr);

UserMessageHookListener::UserMessageHookListener()
{

}

UserMessageHookListener::~UserMessageHookListener()
{

}

void UserMessageHookListener::HookUserMessage()
{
	if (SourceSdk::InterfacesProxy::m_game == SourceSdk::CounterStrikeGlobalOffensive)
	{
		HookInfo sendusermessage_info(SourceSdk::InterfacesProxy::m_engineserver, 45, (DWORD)nSendUserMessage);
		HookGuard::GetInstance()->VirtualTableHook(sendusermessage_info);
	}
	else
	{
		HookInfo usermessagebegin_info(SourceSdk::InterfacesProxy::m_engineserver, 43, (DWORD)nUserMessageBegin);
		HookGuard::GetInstance()->VirtualTableHook(usermessagebegin_info);
	}

	HookInfo messageend_info(SourceSdk::InterfacesProxy::m_engineserver, 44, (DWORD)nMessageEnd);
	HookGuard::GetInstance()->VirtualTableHook(messageend_info);
}

void UserMessageHookListener::RegisterUserMessageHookListener(UserMessageHookListener* listener)
{
	m_listeners.Add(listener);
}

void UserMessageHookListener::RemoveUserMessageHookListener(UserMessageHookListener* listener)
{
	m_listeners.Remove(listener);
}

#ifdef GNUC
void HOOKFN_INT UserMessageHookListener::nSendUserMessage(SourceSdk::IVEngineServer023csgo* thisptr, SourceSdk::IRecipientFilter &, int, google::protobuf::Message const &)
#else
void HOOKFN_INT UserMessageHookListener::nSendUserMessage(SourceSdk::IVEngineServer023csgo* thisptr, void*, SourceSdk::IRecipientFilter & filter, int message_id, google::protobuf::Message const & buffer)
#endif
{
	Assert(bypass == false);
	bypass = false;

	ListenersList_t::elem_t* it = m_listeners.GetFirst();
	while (it != nullptr)
	{
		bypass |= it->m_value.listener->SendUserMessageCallback(filter, message_id, buffer);

		it = it->m_next;
	}

	if (!bypass)
	{
		SourceSdk::InterfacesProxy::Call_SendUserMessage(&filter, message_id, buffer);
	}
}

#ifdef GNUC
SourceSdk::bf_write* HOOKFN_INT UserMessageHookListener::nUserMessageBegin(SourceSdk::IVEngineServer023* thisptr, SourceSdk::IRecipientFilter*, int)
#else
SourceSdk::bf_write* HOOKFN_INT UserMessageHookListener::nUserMessageBegin(SourceSdk::IVEngineServer023* thisptr, void*, SourceSdk::IRecipientFilter* filter, int message_id)
#endif
{
	Assert(bypass == false);
	bypass = false;

	ListenersList_t::elem_t* it = m_listeners.GetFirst();
	while (it != nullptr)
	{
		bypass |= it->m_value.listener->UserMessageBeginCallback(filter, message_id);

		it = it->m_next;
	}

	if (bypass)
	{
		m_buffer = new SourceSdk::bf_write;
		m_buffer->m_pData = new unsigned long[128];
		m_buffer->m_nDataBytes = m_buffer->m_nDataBits = m_buffer->m_iCurBit = m_buffer->m_bOverflow = 0;
		m_buffer->m_bAssertOnOverflow = true;
		m_buffer->m_pDebugName = "Intercepted Buffer";
		return m_buffer;
	}
	else
	{
		return SourceSdk::InterfacesProxy::Call_UserMessageBegin(filter, message_id);
	}
}

#ifdef GNUC
void HOOKFN_INT UserMessageHookListener::nMessageEnd(void* thisptr)
#else
void HOOKFN_INT UserMessageHookListener::nMessageEnd(void* thisptr, void*)
#endif
{
	if (bypass)
	{
		if (m_buffer->m_pData) delete m_buffer->m_pData;
		if (m_buffer) delete m_buffer;
		m_buffer = nullptr;
		bypass = false;
	}
	else
	{
		SourceSdk::InterfacesProxy::Call_MessageEnd();
	}
}