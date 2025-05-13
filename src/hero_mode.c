#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"

enum config_toggle_options {
    CONFIG_ON,
    CONFIG_OFF,
};

typedef struct EnElf EnElf;
typedef void (*EnElfActionFunc)(struct EnElf*, PlayState*);
typedef void (*EnElfUnkFunc)(struct EnElf*, PlayState*);

typedef struct EnElf {
    /* 0x000 */ Actor actor;
    /* 0x144 */ SkelAnime skelAnime;
    /* 0x188 */ Vec3s jointTable[7];
    /* 0x1B2 */ Vec3s morphTable[7];
    /* 0x1DC */ Color_RGBAf innerColor;
    /* 0x1EC */ Color_RGBAf outerColor;
    /* 0x1FC */ LightInfo lightInfoGlow;
    /* 0x20C */ LightNode* lightNodeGlow;
    /* 0x210 */ LightInfo lightInfoNoGlow;
    /* 0x220 */ LightNode* lightNodeNoGlow;
    /* 0x224 */ Vec3f unk_224;
    /* 0x230 */ Actor* elfMsg;
    /* 0x234 */ Actor* unk_234;
    /* 0x238 */ f32 unk_238;
    /* 0x23C */ f32 unk_23C;
    /* 0x240 */ f32 unk_240;
    /* 0x244 */ s16 unk_244;
    /* 0x246 */ s16 unk_246;
    /* 0x248 */ s16 unk_248;
    /* 0x24A */ s16 unk_24A;
    /* 0x24C */ s16 unk_24C;
    /* 0x250 */ f32 unk_250;
    /* 0x254 */ f32 unk_254;
    /* 0x258 */ s16 unk_258;
    /* 0x25A */ u16 timer;
    /* 0x25C */ s16 unk_25C;
    /* 0x25E */ s16 disappearTimer;
    /* 0x260 */ s16 collectibleFlag;
    /* 0x262 */ u16 fairyFlags;
    /* 0x264 */ u16 unk_264;
    /* 0x266 */ u16 unk_266;
    /* 0x268 */ u8 unk_268;
    /* 0x269 */ u8 unk_269;
    /* 0x26C */ EnElfUnkFunc unk_26C;
    /* 0x270 */ EnElfActionFunc actionFunc;
} EnElf; // size = 0x274

extern bool Player_InBlockingCsMode(PlayState* play, Player* player);
extern s32 func_808339D4(PlayState* play, Player* this, s32 damage);
RECOMP_PATCH s32 Health_ChangeBy(PlayState* play, s16 healthChange) {
    s16 damageMultiplier = (s16)recomp_get_config_u32("damage_multiplier");
    u8 oneHitKo = recomp_get_config_u32("one_hit_ko");
    u8 loseRupees = recomp_get_config_u32("lose_rupees_on_death");
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

    if (healthChange < 0 && oneHitKo == CONFIG_ON) {
        gSaveContext.save.saveInfo.playerData.health = 0;
        if (loseRupees == CONFIG_ON) {
            gSaveContext.save.saveInfo.playerData.rupees = 0;
        }
        return false;
    }

    if (gSaveContext.save.saveInfo.playerData.health <= 0) {
        gSaveContext.save.saveInfo.playerData.health = 0;
        if (loseRupees == CONFIG_ON) {
            gSaveContext.save.saveInfo.playerData.rupees = 0;
        }
        return false;
    } else {
        return true;
    }

}

u8 gItemIsHeart;
RECOMP_HOOK("EnItem00_Init") void on_EnItem00_Init(Actor* thisx, PlayState* play) {
    s16 maskedParam = thisx->params & 0xFF;
    if (maskedParam == ITEM00_RECOVERY_HEART || maskedParam == ITEM00_3_HEARTS) {
        gItemIsHeart = true;
    }
}

u8 gFairySpawned;
RECOMP_HOOK("func_8088CC48") void on_func_8088CC48(EnElf* this, PlayState* play) {
    gFairySpawned = true;
}

RECOMP_PATCH s32 Flags_GetCollectible(PlayState* play, s32 flag) {
    u8 gHeartsRemovedConfig = recomp_get_config_u32("disable_recovery_hearts");
    u8 gFairiesRemovedConfig = recomp_get_config_u32("disable_fairies");
    if (gItemIsHeart && gHeartsRemovedConfig == CONFIG_ON) {
        gItemIsHeart = false;
        return true;
    } else if (gFairySpawned && gFairiesRemovedConfig == CONFIG_ON) {
        gFairySpawned = false;
        return true;
    } else {
        if ((flag > 0) && (flag < 0x80)) {
            return play->actorCtx.sceneFlags.collectible[(flag & ~0x1F) >> 5] & (1 << (flag & 0x1F));
        }
        gItemIsHeart = false;
        gFairySpawned = false;
        return 0;    
    }
}