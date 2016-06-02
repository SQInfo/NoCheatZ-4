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

#include "MRecipientFilter.h"

#include "Helpers.h" // edict, PEntityOfEntIndex
#include "Players/NczPlayerManager.h"

MRecipientFilter::MRecipientFilter(void)
{
	m_Recipients.EnsureCapacity(64);
	m_bInitMessage = false;
	m_bReliable = true;
}

MRecipientFilter::~MRecipientFilter(void)
{
}

MRecipientFilter::MRecipientFilter(MRecipientFilter const & other)
{
	m_bReliable = other.m_bReliable;
	m_bInitMessage = other.m_bInitMessage;
	m_Recipients = other.m_Recipients;
}

MRecipientFilter& MRecipientFilter::operator=(MRecipientFilter const & other)
{
	m_bReliable = other.m_bReliable;
	m_bInitMessage = other.m_bInitMessage;
	m_Recipients = other.m_Recipients;
	return *this;
}

void MRecipientFilter::SetReliable(bool reliable)
{
	m_bReliable = reliable;
}

void MRecipientFilter::SetInitMessage(bool init)
{
	m_bInitMessage = init;
}

int MRecipientFilter::GetRecipientCount() const
{
	return m_Recipients.Count();
}

int MRecipientFilter::GetRecipientIndex(int slot) const
{
    if ( slot < 0 || slot >= GetRecipientCount() )
        return -1;

    return m_Recipients[ slot ];
}

bool MRecipientFilter::IsInitMessage() const
{
    return m_bInitMessage;
}

bool MRecipientFilter::IsReliable() const
{
    return m_bReliable;
}

void MRecipientFilter::AddAllPlayers(int maxClients)
{
    m_Recipients.RemoveAll();
    for ( int i = 1; i <= maxClients; i++ )
    {
		SourceSdk::edict_t *pPlayer = Helpers::PEntityOfEntIndex(i);
        if ( !pPlayer || pPlayer->IsFree())
            continue;
        m_Recipients.AddToTail(i);
    }
}

void MRecipientFilter::RemoveAll()
{
	m_Recipients.RemoveAll();
}

void MRecipientFilter::AddTeam(int teamid)
{
	PLAYERS_LOOP_RUNTIME_UNROLL(x)
	{
		if (x_ph->status > INVALID)
		{
			void* playerinfo = x_ph->playerClass->GetPlayerInfo();
			if (playerinfo != nullptr)
			{
				int player_team;
				if (SourceSdk::InterfacesProxy::m_game == SourceSdk::CounterStrikeGlobalOffensive)
				{
					player_team = static_cast<SourceSdk::IPlayerInfo_csgo*>(playerinfo)->GetTeamIndex();
				}
				else
				{
					player_team = static_cast<SourceSdk::IPlayerInfo*>(playerinfo)->GetTeamIndex();
				}

				if (player_team == teamid)
				{
					AddRecipient(x_index);
				}
			}
		}
	}
	END_PLAYERS_LOOP_UNROLL(x)
}

void MRecipientFilter::AddAllPlayersExcludeTeam(int teamid)
{
	m_Recipients.RemoveAll();
	if (teamid != 0) AddTeam(0);
	if (teamid != 1) AddTeam(1);
	if (teamid != 2) AddTeam(2);
	if (teamid != 3) AddTeam(3);
}

void MRecipientFilter::AddRecipient( int iPlayer )
{
    if ( m_Recipients.Find( iPlayer ) != m_Recipients.InvalidIndex() )
        return;

    m_Recipients.AddToTail( iPlayer );
}

void MRecipientFilter::RemoveRecipient(int iPlayer)
{
	int index = m_Recipients.Find(iPlayer);
	if (index != m_Recipients.InvalidIndex())
		return;

	m_Recipients.AddToTail(iPlayer);
}
