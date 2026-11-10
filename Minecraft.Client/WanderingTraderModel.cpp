#include "stdafx.h"
#include "WanderingTraderModel.h"
#include "ModelPart.h"

WanderingTraderModel::WanderingTraderModel() : VillagerModel(0.0f, 0.0f, 64, 64)
{
	hood = (new ModelPart(this))->setTexSize(64, 64);
	hood->setPos(0, 0, 0);
	hood->texOffs(32, 0)->addBox(-4, -10, -4, 8, 10, 8, 0.51f);
	head->addChild(hood);
	hood->compile(1.0f/16.0f);
}
