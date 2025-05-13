#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

extern bool Player_InBlockingCsMode(PlayState* play, Player* player);
extern s32 func_808339D4(PlayState* play, Player* this, s32 damage);
RECOMP_PATCH s32 Health_ChangeBy(PlayState* play, s16 healthChange) {
    s16 damageMultiplier = (s16)recomp_get_config_u32("damage_multiplier");
    u8 oneHitKo = recomp_get_config_u32("one_hit_ko");
    if (healthChange > 0) {
        Audio_PlaySfx(NA_SE_SY_HP_RECOVER);
    } else if (gSaveContext.save.saveInfo.playerData.doubleDefense && (healthChange < 0)) {
        healthChange >>= 1;
    }

    if (healthChange < 0) {
        healthChange = healthChange * damageMultiplier;
    }

    gSaveContext.save.saveInfo.playerData.health += healthChange;

    if (((void)0, gSaveContext.save.saveInfo.playerData.health) >
        ((void)0, gSaveContext.save.saveInfo.playerData.healthCapacity)) {
        gSaveContext.save.saveInfo.playerData.health = gSaveContext.save.saveInfo.playerData.healthCapacity;
    }

    if (oneHitKo == 0) {
        gSaveContext.save.saveInfo.playerData.health = 0;
        return true;
    }

    if (gSaveContext.save.saveInfo.playerData.health <= 0) {
        gSaveContext.save.saveInfo.playerData.health = 0;
        return false;
    } else {
        return true;
    }
}

u8 gItemIsHeart;
RECOMP_HOOK("EnItem00_Init") void on_EnItem00_Init(Actor* thisx, PlayState* play) {
    if (thisx->params == ITEM00_RECOVERY_HEART) {
        gItemIsHeart = true;
    }
}

RECOMP_PATCH s32 Flags_GetCollectible(PlayState* play, s32 flag) {
    u8 gHeartsRemovedConfig = recomp_get_config_u32("disable_recovery_hearts");
    if (gItemIsHeart && gHeartsRemovedConfig == 0) {
        gItemIsHeart = false;
        return true;
    } else {
        if ((flag > 0) && (flag < 0x80)) {
            return play->actorCtx.sceneFlags.collectible[(flag & ~0x1F) >> 5] & (1 << (flag & 0x1F));
        }
        gItemIsHeart = false;
        return 0;    
    }
}