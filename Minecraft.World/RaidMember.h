#pragma once

// neoLegacy - anything that can be part of a raid (the illagers and the Ravager). A raid sends its members to the village centre
// when they have nothing to fight (see RaidGoals).
class RaidMember
{
public:
	virtual ~RaidMember() {}
	virtual bool isInRaid() = 0;
	virtual void getRaidCenter(int *x, int *y, int *z) = 0;
	virtual void joinRaid(int x, int y, int z) = 0;
	virtual void leaveRaid() = 0;		// the raid is over (won, lost or called off): stop heading for the village
};
