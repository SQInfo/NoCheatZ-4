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

#include <cstdlib> // rand
#include <ctime>

#include "PlayerRunCommandHookListener.h"

#include "Interfaces/InterfacesProxy.h"
#include "Interfaces/iserverunknown.h"
#include "Console/convar.h"

#include "plugin.h"
#include "Systems/ConfigManager.h"

/////////////////////////////////////////////////////////////////////////
// PlayerRunCommandHookListener
/////////////////////////////////////////////////////////////////////////

PlayerRunCommand_t PlayerRunCommandHookListener::gpOldPlayerRunCommand = nullptr;
DWORD* PlayerRunCommandHookListener::pdwInterface = nullptr;
ListenersListT PlayerRunCommandHookListener::m_listeners;
SourceSdk::CUserCmd_csgo PlayerRunCommandHookListener::m_lastCUserCmd[MAX_PLAYERS];

PlayerRunCommandHookListener::PlayerRunCommandHookListener()
{
	//for(int x = 1; x < MAX_PLAYERS; ++x) m_lastCUserCmd[x] = SourceSdk::CUserCmd();
	std::srand((unsigned int)(std::time(0)));
}

PlayerRunCommandHookListener::~PlayerRunCommandHookListener()
{
}

void* PlayerRunCommandHookListener::GetLastUserCmd(NczPlayer* player)
{
	return &(m_lastCUserCmd[player->GetIndex()]);
}

void PlayerRunCommandHookListener::HookPlayerRunCommand(NczPlayer* player)
{
	Assert(Helpers::isValidEdict(player->GetEdict()));
	void* unk = player->GetEdict()->m_pUnk;
	DWORD* vtptr = IFACE_PTR(unk);

	if(pdwInterface != vtptr)
	{
		pdwInterface = vtptr;

		DWORD OldFunc = VirtualTableHook(pdwInterface, ConfigManager::GetInstance()->GetVirtualFunctionId("playerruncommand"), (DWORD)nPlayerRunCommand );
		*(DWORD*)&(gpOldPlayerRunCommand) = OldFunc;
	}
}

void PlayerRunCommandHookListener::UnhookPlayerRunCommand()
{
	if(pdwInterface && gpOldPlayerRunCommand)
	{
		VirtualTableHook(pdwInterface, ConfigManager::GetInstance()->GetVirtualFunctionId("playerruncommand"), (DWORD)gpOldPlayerRunCommand, (DWORD)nPlayerRunCommand);
		pdwInterface = nullptr;
		gpOldPlayerRunCommand = nullptr;
	}
}

#ifdef GNUC
void PlayerRunCommandHookListener::nPlayerRunCommand(void* This, SourceSdk::CUserCmd* pCmd, IMoveHelper* pMoveHelper)
#else
void HOOKFN_INT PlayerRunCommandHookListener::nPlayerRunCommand(void* This, void*, void* pCmd, IMoveHelper* pMoveHelper)
#endif
{
	PlayerHandler* ph = NczPlayerManager::GetInstance()->GetPlayerHandlerByBasePlayer(This);
	PlayerRunCommandRet ret = CONTINUE;
	
	if(ph->status > PLAYER_CONNECTING)
	{
		SourceSdk::CUserCmd_csgo& old_cmd = m_lastCUserCmd[ph->playerClass->GetIndex()];


		ListenersListT::elem_t* it = m_listeners.GetFirst();
		while (it != nullptr)
		{
			if (ph->status >= it->m_value.filter)
			{
				ret = it->m_value.listener->PlayerRunCommandCallback(ph->playerClass, pCmd, &old_cmd);
				if (ret > CONTINUE) break;
			}
			it = it->m_next;
		}
		
		memcpy(&old_cmd, pCmd, sizeof(SourceSdk::CUserCmd_csgo));
	}

	if(ret < BLOCK)
	{
		if(ret == INERT) InertCmd(pCmd);

		if (SourceSdk::InterfacesProxy::m_game == SourceSdk::CounterStrikeGlobalOffensive)
		{
			static_cast<SourceSdk::CUserCmd_csgo*>(pCmd)->random_seed = std::rand() & std::numeric_limits<int>::max();
		}
		else
		{
			static_cast<SourceSdk::CUserCmd*>(pCmd)->random_seed = std::rand() & std::numeric_limits<int>::max();
		}

		gpOldPlayerRunCommand(This, pCmd, pMoveHelper);
	}
}

void PlayerRunCommandHookListener::RegisterPlayerRunCommandHookListener(PlayerRunCommandHookListener* listener, size_t priority, SlotStatus filter)
{
	m_listeners.Add(listener, priority, filter);
}

void PlayerRunCommandHookListener::RemovePlayerRunCommandHookListener(PlayerRunCommandHookListener* listener)
{
	m_listeners.Remove(listener);
}

inline void InertCmd(void* pcmd)
{
	if (SourceSdk::InterfacesProxy::m_game == SourceSdk::CounterStrikeGlobalOffensive)
	{
		SourceSdk::CUserCmd_csgo* z = static_cast<SourceSdk::CUserCmd_csgo*>(pcmd);
		VectorCopy(SourceSdk::QAngle(0.0f, 0.0f, 0.0f), z->viewangles);
		z->buttons = 0;
		//z->tick_count = z->tick_count - 50;
	}
	else
	{
		SourceSdk::CUserCmd* z = static_cast<SourceSdk::CUserCmd*>(pcmd);
		VectorCopy(SourceSdk::QAngle(0.0f, 0.0f, 0.0f), z->viewangles);
		z->buttons = 0;
		//z->tick_count = z->tick_count - 50;
	}
}