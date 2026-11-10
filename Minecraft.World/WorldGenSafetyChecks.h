#pragma once

class Level;

// Small shared helpers used by the new atmospheric worldgen features
// (LuckyBlockSpawnFeature, AbandonedTentFeature, and the mini-structures)
// to keep placement safe and avoid dropping content underground,
// underwater, or on top of something already generated there.
//
// HONEST LIMITATION: this engine doesn't expose a "is this position
// already inside a generated structure" query in the parts of the
// codebase available here (no getStructureReferences-equivalent was
// found). These checks are a best-effort proxy instead: they require
// the ground and a small surrounding footprint to still look like
// undisturbed natural terrain (grass/dirt/stone/sand, nothing
// artificial already sitting there). Because BiomeDecorator runs after
// structures are carved into the chunk during generation, a spot that
// already has village/mineshaft blocks in it will normally fail this
// check and get skipped - but this is not a guaranteed structural
// exclusion the way a real bounding-box query would be.
class WorldGenSafetyChecks
{
public:
	// True if (x, groundY, z) is open to the sky, not underwater, and
	// sitting on solid natural ground - i.e. a normal outdoor surface
	// spot, not a cave, not underground, not a lake bed.
	static bool isSafeOutdoorSurface(Level *level, int x, int z, int &outGroundY);

	// True if every block in the given flat footprint (inclusive range,
	// same Y) is still natural, undisturbed terrain - used as a cheap
	// proxy for "nothing else already generated here".
	static bool isFootprintNatural(Level *level, int x0, int z0, int x1, int z1, int y);

	static bool isNaturalGroundTile(int tileId);
};
