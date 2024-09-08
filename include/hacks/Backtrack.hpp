/*

Backtrack.hpp

*/

#pragma once
#include "common.hpp"
#include "entityhitboxcache.hpp"

namespace hacks::backtrack
{
class CIncomingSequence
{
public:
    int inreliablestate;
    int sequencenr;
    float curtime;

    CIncomingSequence(int instate, int seqnr, float time)
    {
        inreliablestate = instate;
        sequencenr      = seqnr;
        curtime         = time;
    }
};

class BacktrackData
{
public:
    int tickcount{};
    int entidx{};

    bool in_range{};

    std::vector<matrix3x4_t> bones{};

    Vector m_vecOrigin{};
    Vector m_vecAngles{};
    float simtime;
    float animtime;
    float cycle;
    int sequence;
    std::array<hitbox_cache::CachedHitbox, HITBOXES_SIZE> hitboxes{};
};

extern settings::Float latency;
void adjustPing(INetChannel *ch);
bool backtrackEnabled();
bool isTickInRange(int tickcount);
void MoveToTick(BacktrackData data);
void RestoreEntity(int entidx);
bool hasData();
std::optional<BacktrackData> getData();
std::optional<std::vector<BacktrackData>> getGoodTicks(CachedEntity *ent);

extern std::vector<std::vector<BacktrackData>> bt_data;

} // namespace hacks::backtrack