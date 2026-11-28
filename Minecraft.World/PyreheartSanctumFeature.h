#pragma once
#include "StructureFeature.h"
#include "StructureStart.h"
#include "StructurePiece.h"
class PyreheartSanctumFeature : public StructureFeature
{
private:
    bool isSpotSelected;
    ChunkPos *sanctumChunk;
public:
    PyreheartSanctumFeature();
    virtual ~PyreheartSanctumFeature();
    virtual wstring getFeatureName();
protected:
    virtual bool isFeatureChunk(int x, int z, bool bIsSuperflat = false);
    virtual StructureStart *createStructureStart(int x, int z);
public:
    class SanctumStart : public StructureStart
    {
    public:
        static StructureStart *Create() { return new SanctumStart(); }
        virtual EStructureStart GetType() { return eStructureStart_PyreheartSanctum; }
        SanctumStart();
        SanctumStart(Level *level, Random *random, int chunkX, int chunkZ);
    };
    class SanctumPiece : public StructurePiece
    {
    public:
        static StructurePiece *Create() { return new SanctumPiece(); }
        virtual EStructurePiece GetType() { return eStructurePiece_PyreheartSanctumPiece; }
        SanctumPiece();
        SanctumPiece(int chunkX, int chunkZ);
        virtual bool postProcess(Level *level, Random *random, BoundingBox *chunkBB);
    protected:
        virtual void addAdditonalSaveData(CompoundTag *tag) {}
        virtual void readAdditonalSaveData(CompoundTag *tag) {}
    };
};
