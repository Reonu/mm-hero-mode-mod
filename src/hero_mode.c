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

typedef struct EnBigslime EnBigslime;
typedef struct EnMinislime EnMinislime;
typedef void (*EnBigslimeActionFunc)(struct EnBigslime*, PlayState*);
typedef void (*EnMinislimeActionFunc)(struct EnMinislime*, PlayState*);
typedef struct EnMinislime {
    /* 0x000 */ Actor actor;
    /* 0x144 */ EnMinislimeActionFunc actionFunc;
    /* 0x148 */ u8 frozenAlpha;
    /* 0x149 */ u8 id; // 0-14, each of the 15 minislimes gets a unique number
    /* 0x14A */ union {
                    s16 frozenTimer;
                    s16 bounceTimer;
                    s16 growShrinkTimer;
                    s16 idleTimer;
                    s16 knockbackTimer;
                    s16 meltTimer;
                    s16 throwTimer;
                };
    /* 0x14C */ s16 attackTimer;
    /* 0x150 */ f32 frozenScale;
    /* 0x154 */ Vec3f shakeRefPos;
    /* 0x160 */ ColliderCylinder collider;
} EnMinislime; // size = 0x1AC

typedef struct {
    /* 0x00 */ Vec3f pos;
    /* 0x0C */ Vec3f velocity;
    /* 0x18 */ s16 isEnabled;
    /* 0x1A */ Vec3s rot;
    /* 0x20 */ f32 scale;
} EnBigslimeIceShardEffect; // size = 0x24


typedef struct EnBigslime {
    /* 0x0000 */ Actor actor;
    /* 0x0144 */ SkelAnime skelAnime;
    /* 0x0188 */ EnBigslimeActionFunc actionFunc;
    /* 0x018C */ EnBigslimeActionFunc actionFuncStored;
    /* 0x0190 */ Vec3s jointTable[24];
    /* 0x0220 */ Vec3s morphTable[24];
    /* 0x02B0 */ u8 minislimeState;
    /* 0x02B1 */ u8 dynamicVtxState; // Toggles between two states of dynamic Vtx
    /* 0x02B2 */ u8 isAnimFinished;
    /* 0x02B3 */ union {
                    u8 formBigslimeTimer; // Bigslime will start forming when timer reaches 0
                    u8 minislimeCounter;
                    u8 numGekkoMeleeAttacks; // The Gekko will melee-attack link at 1-3 times at each position while engulfed in bigslime
                };
    /* 0x02B4 */ u8 shockwaveAlpha;
    /* 0x02B5 */ u8 gekkoDrawDmgEffType;
    /* 0x02B6 */ s16 gekkoYaw;
    /* 0x02B8 */ s16 csId;
    /* 0x02BA */ union { // multi-use timer
                    s16 idleTimer;
                    s16 noticeTimer;
                    s16 callTimer;
                    s16 jumpTimer;
                    s16 damageSpinTimer;
                    s16 scaleFactor; // Used between individual gekko melee-attacks while engulfed in bigslime
                    s16 windupPunchTimer;
                    s16 throwPlayerTimer;
                    s16 ceilingMoveTimer;
                    s16 squishFlatTimer;
                    s16 riseCounter;
                    s16 grabPlayerTimer;
                    s16 formBigslimeCutsceneTimer;
                    s16 defeatTimer;
                    s16 despawnTimer;
                    s16 spawnFrogTimer;
                    s16 isDespawned;
                    s16 isInitJump;
                };
    /* 0x02BC */ s16 wavySurfaceTimer; // decrements from 24, used in Math_SinF for currData2
    /* 0x02BE */ s16 stunTimer;
    /* 0x02C0 */ union {
                    s16 freezeTimer;
                    s16 meltCounter;
                };
    /* 0x02C2 */ s16 ceilingDropTimer; // Bigslime is still during this timer and shakes before dropping
    /* 0x02C4 */ s16 numGekkoPosGrabPlayer; // The Gekko will melee-attack link at 6 positions while engulfed in bigslime
    /* 0x02C6 */ s16 subCamId;
    /* 0x02C8 */ s16 subCamYawGrabPlayer;
    /* 0x02CA */ s16 rotation; // is always 0, used in Matrix_RotateYS
    /* 0x02CC */ s16 itemDropTimer; // items only drop when zero, Set to 40 then decrements, prevents itemDrop spam
    /* 0x02CE */ Vec3s gekkoRot;
    /* 0x02D4 */ Vec3f gekkoPosOffset; // Used when Bigslime grabs link
    /* 0x02E0 */ Vec3f gekkoProjectedPos;
    /* 0x02EC */ union {
                    Vec3f frozenPos;
                    Vec3f subCamDistToFrog; // Used to zoom into frogs as Gekko despawns/Frog spawns
                };
    /* 0x02F8 */ Vec3f gekkoBodyPartsPos[12];
    /* 0x0388 */ f32 gekkoDrawDmgEffAlpha;
    /* 0x038C */ f32 gekkoDrawDmgEffScale;
    /* 0x0390 */ f32 gekkoDrawDmgEffFrozenSteamScale;
    /* 0x0394 */ f32 gekkoScale;
    /* 0x0398 */ f32 vtxSurfacePerturbation[162];
    /* 0x0620 */ f32 vtxScaleX;
    /* 0x0624 */ f32 vtxScaleZ;
    /* 0x0628 */ f32 shockwaveScale;
    /* 0x062C */ ColliderCylinder gekkoCollider;
    /* 0x0678 */ ColliderCylinder bigslimeCollider[12];
    /* 0x0A14 */ EnMinislime* minislime[15];
    /* 0x0A44 */ EnMinislime* minislimeToThrow;
    /* 0x0A48 */ AnimatedMaterial* bigslimeFrozenTexAnim;
    /* 0x0A4C */ AnimatedMaterial* minislimeFrozenTexAnim;
    /* 0x0A50 */ AnimatedMaterial* iceShardTexAnim;
    /* 0x0A54 */ EnBigslimeIceShardEffect iceShardEffect[(162 + 10*15)]; // 312 = 162 (bigslime) + 10 * 15 (minislime)
} EnBigslime; // size = 0x3634

PlayState* gGlobalPlay;
RECOMP_HOOK("EnBigslime_AttackPlayerInBigslime") void on_EnBigslime_AttackPlayerInBigslime(EnBigslime* this, PlayState* play) {
    gGlobalPlay = play;
}

// Fix softlock when dying to the slime grab attack.
RECOMP_HOOK_RETURN("EnBigslime_AttackPlayerInBigslime") void return_EnBigslime_AttackPlayerInBigslime(void) {
    Player* player = GET_PLAYER(gGlobalPlay);
    if (gSaveContext.save.saveInfo.playerData.health <= 0) {
            player->actor.parent = NULL;
            player->av2.actionVar2 = 100;
    }
}