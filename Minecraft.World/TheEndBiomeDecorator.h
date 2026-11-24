#include "BiomeDecorator.h"

class TheEndBiomeDecorator : public BiomeDecorator
{
public:
	typedef struct  
	{
		int iChunkX;
		int iChunkZ;
		int x;
		int z;
		int radius;
	}
	SPIKE;	
	
	TheEndBiomeDecorator(Biome *biome);
protected:
	Feature *spikeFeature;
	Feature *endPodiumFeature;
	virtual void decorate();


public:
	static SPIKE SpikeValA[8];
	static const int SPIKE_BASE_Y = 62;
};