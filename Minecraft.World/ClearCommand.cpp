#include "stdafx.h"
#include "net.minecraft.commands.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.network.packet.h"
#include "../Minecraft.Client/MinecraftServer.h"
#include "../Minecraft.Client/ServerPlayer.h"
#include "ClearCommand.h"

EGameCommand ClearCommand::getId()
{
    return eGameCommand_Clear;
}

int ClearCommand::getPermissionLevel()
{
    return LEVEL_GAMEMASTERS;
}

void ClearCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
    ByteArrayInputStream bais(commandData);
    DataInputStream dis(&bais);

    PlayerUID uid = dis.readPlayerUID();
    shared_ptr<ServerPlayer> player = getPlayer(uid);

    if (player == nullptr || player->inventory == nullptr)
    {
        source->sendMessage(L"Player not found.");
        return;
    }

    int cleared = player->inventory->clearInventory(-1, -1);
    player->inventory->setChanged();

    // Push the inventory changes to the client immediately instead of waiting
    // for the normal container update tick.
    if (player->containerMenu != nullptr)
    {
        player->containerMenu->broadcastChanges();
    }
    player->broadcastCarriedItem();

    source->sendMessage(L"Cleared " + to_wstring(cleared) + L" items from " + player->getName() + L"'s inventory.");
}

shared_ptr<GameCommandPacket> ClearCommand::preparePacket(shared_ptr<Player> player)
{
    if (player == nullptr)
        return nullptr;

    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);
    dos.writePlayerUID(player->getXuid());

    return std::make_shared<GameCommandPacket>(eGameCommand_Clear, baos.toByteArray());
}
