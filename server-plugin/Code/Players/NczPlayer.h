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

#ifndef NCZPCLASS
#define NCZPCLASS

#include "Misc/temp_basicstring.h"

#ifdef GNUC
#	include <mm_malloc.h>
#endif

#include "SdkPreprocessors.h"
#undef GetClassName
#include "Interfaces/edict.h"
#include "Interfaces/IPlayerInfoManager/IPlayerInfoManager.h"
#include "Interfaces/inetchannelinfo.h"

#include "Preprocessors.h"

/* Permet de conna�tre l'�tat d'un slot du serveur rapidement */
enum SlotStatus
{
	INVALID = 0, // Slot not used
	KICK, // In process of being kicked or banned
	BOT, // A bot ...
	PLAYER_CONNECTING, // Not a bot, not connected
	PLAYER_CONNECTED, // Connected as spectator or dead
	PLAYER_IN_TESTS // Playing the round and shooting people everywhere like a mad nerd :)
};

enum SlotFilterBehavior
{
	STATUS_EQUAL_OR_BETTER = 0,
	STATUS_BETTER,
	STATUS_STRICT
};

class SlotFilter
{
private:
	SlotStatus const m_status;
	SlotStatus const m_load_status;
	SlotFilterBehavior const m_behavior;

	SlotFilter& operator=(SlotFilter const & other)
	{
	}

protected:
	SlotFilter(SlotFilter const & other) : m_status(other.m_status), m_load_status(other.m_load_status), m_behavior(other.m_behavior)
	{
	}


	SlotFilter(SlotStatus status, SlotStatus load_filter, SlotFilterBehavior behavior) : m_status(status), m_load_status(load_filter), m_behavior(behavior)
	{
	}

	virtual ~SlotFilter() {};

public:
	virtual inline bool CanProcessThisSlot(SlotStatus const player_slot_status) const
	{
		if (player_slot_status < m_status) return false;
		switch (m_behavior)
		{
		case STATUS_BETTER:
			if (player_slot_status == m_status) return false;
		case STATUS_STRICT:
			if (player_slot_status > m_status) return false;
		case STATUS_EQUAL_OR_BETTER:
			return true;
		default:
			return false;
		};
	}

	virtual SlotStatus GetTargetSlotStatus() const
	{
		return m_status;
	}

	virtual SlotStatus GetLoadFilter() const
	{
		return m_load_status;
	}

	virtual SlotFilterBehavior GetFilterBehavior() const
	{
		return m_behavior;
	}
};

enum WpnShotType
{
	HAND,
	AUTO,
	PISTOL
};

class ALIGN16 NczPlayer
{
private:
	int cIndex;
	int m_userid;
	SourceSdk::edict_t * m_edict;
	SourceSdk::INetChannelInfo* m_channelinfo;
	float m_time_connected;

public:
	NczPlayer(int index);
	~NczPlayer(){};

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}

	void operator delete(void* p)
	{
		_mm_free(p);
	}

	int GetIndex() const { return cIndex; }; 
	int GetUserid() const;
	SourceSdk::edict_t * GetEdict() const;
	void * GetPlayerInfo() const;
	SourceSdk::INetChannelInfo* GetChannelInfo() const;
	const char * GetName() const;
	const char * GetSteamID() const;
	const char * GetIPAddress() const;
	WpnShotType GetWpnShotType() const;
	int aimingAt(); // Retourne index de la cible pr�sente sur le viseur

	basic_string GetReadableIdentity();

	float GetTimeConnected() const;

	bool isValidEdict();

	void OnConnect();

	void Kick(const char * msg = "Kicked by NoCheatZ 4");
	void Ban(const char * msg = "Banned by NoCheatZ 4", int minutes = 0);
} ALIGN16_POST;

#endif