#pragma once

#include <stdint.h>

#include <string>
#include <map>
#include <set>
#include <memory>
#include <unordered_map>
#include <phosg/Encoding.hh>
#include <phosg/JSON.hh>

#include "../Text.hh"

namespace Episode3 {



// The comment in Server.hh does not apply to this file (and DataIndex.cc).
// Except for the Location structure, these structures and functions are not
// based on Sega's original implementation.

class DataIndex;



enum BehaviorFlag {
  SKIP_DECK_VERIFY       = 0x00000001,
  IGNORE_CARD_COUNTS     = 0x00000002,
  SKIP_D1_D2_REPLACE     = 0x00000004,
  DISABLE_TIME_LIMITS    = 0x00000008,
  ENABLE_STATUS_MESSAGES = 0x00000010,
  LOAD_CARD_TEXT         = 0x00000020,
  ENABLE_RECORDING       = 0x00000040,
  DISABLE_MASKING        = 0x00000080,
  DISABLE_INTERFERENCE   = 0x00000100,
};



enum class StatSwapType : uint8_t {
  NONE = 0,
  A_T_SWAP = 1,
  A_H_SWAP = 2,
};

enum class ActionType : uint8_t {
  INVALID_00 = 0,
  DEFENSE = 1,
  ATTACK = 2,
};

enum class AttackMedium : uint8_t {
  UNKNOWN = 0,
  PHYSICAL = 1,
  TECH = 2,
  UNKNOWN_03 = 3, // Probably Resta
  INVALID_FF = 0xFF,
};

const char* name_for_attack_medium(AttackMedium medium);

enum class CriterionCode : uint8_t {
  NONE = 0x00,
  HU_CLASS_SC = 0x01,
  RA_CLASS_SC = 0x02,
  FO_CLASS_SC = 0x03,
  SAME_TEAM = 0x04,
  SAME_PLAYER = 0x05,
  SAME_TEAM_NOT_SAME_PLAYER = 0x06, // Allies only
  UNKNOWN_07 = 0x07,
  NOT_SC = 0x08,
  SC = 0x09,
  HU_OR_RA_CLASS_SC = 0x0A,
  HUNTER_HUMAN_SC = 0x0B,
  HUNTER_HU_CLASS_MALE_SC = 0x0C,
  HUNTER_FEMALE_SC = 0x0D,
  HUNTER_HU_OR_FO_CLASS_HUMAN_SC = 0x0E,
  HUNTER_HU_CLASS_ANDROID_SC = 0x0F,
  UNKNOWN_10 = 0x10,
  UNKNOWN_11 = 0x11,
  HUNTER_HUNEWEARL_CLASS_SC = 0x12,
  HUNTER_RA_CLASS_MALE_SC = 0x13,
  HUNTER_RA_CLASS_FEMALE_SC = 0x14,
  HUNTER_RA_OR_FO_CLASS_FEMALE_SC = 0x15,
  HUNTER_HU_OR_RA_CLASS_HUMAN_SC = 0x16,
  HUNTER_RA_CLASS_ANDROID_SC = 0x17,
  HUNTER_FO_CLASS_FEMALE_SC = 0x18,
  HUNTER_FEMALE_HUMAN_SC = 0x19,
  HUNTER_ANDROID_SC = 0x1A,
  HU_OR_FO_CLASS_SC = 0x1B,
  RA_OR_FO_CLASS_SC = 0x1C,
  PHYSICAL_OR_UNKNOWN_ATTACK_MEDIUM = 0x1D,
  TECH_OR_UNKNOWN_ATTACK_MEDIUM = 0x1E,
  PHYSICAL_OR_TECH_OR_UNKNOWN_ATTACK_MEDIUM = 0x1F,
  UNKNOWN_20 = 0x20,
  UNKNOWN_21 = 0x21,
  UNKNOWN_22 = 0x22,
};

enum class CardRarity : uint8_t {
  N1    = 0x01,
  R1    = 0x02,
  S     = 0x03,
  E     = 0x04,
  N2    = 0x05,
  N3    = 0x06,
  N4    = 0x07,
  R2    = 0x08,
  R3    = 0x09,
  R4    = 0x0A,
  SS    = 0x0B,
  D1    = 0x0C,
  D2    = 0x0D,
  INVIS = 0x0E,
};

enum class CardType : uint8_t {
  HUNTERS_SC = 0x00,
  ARKZ_SC    = 0x01,
  ITEM       = 0x02,
  CREATURE   = 0x03,
  ACTION     = 0x04,
  ASSIST     = 0x05,
  INVALID_FF = 0xFF,
  END_CARD_LIST = 0xFF,
};

enum class CardClass : uint16_t {
  HU_SC                      = 0x0000,
  RA_SC                      = 0x0001,
  FO_SC                      = 0x0002,
  NATIVE_CREATURE            = 0x000A,
  A_BEAST_CREATURE           = 0x000B,
  MACHINE_CREATURE           = 0x000C,
  DARK_CREATURE              = 0x000D,
  GUARD_ITEM                 = 0x0015,
  MAG_ITEM                   = 0x0017,
  SWORD_ITEM                 = 0x0018,
  GUN_ITEM                   = 0x0019,
  CANE_ITEM                  = 0x001A,
  ATTACK_ACTION              = 0x001E,
  DEFENSE_ACTION             = 0x001F,
  TECH                       = 0x0020,
  PHOTON_BLAST               = 0x0021,
  CONNECT_ONLY_ATTACK_ACTION = 0x0022,
  BOSS_ATTACK_ACTION         = 0x0023,
  BOSS_TECH                  = 0x0024,
  ASSIST                     = 0x0028,
};

bool card_class_is_tech_like(CardClass cc);

enum class TargetMode : uint8_t {
  NONE = 0x00, // Used for defense cards, mags, shields, etc.
  SINGLE_RANGE = 0x01,
  MULTI_RANGE = 0x02,
  SELF = 0x03,
  TEAM = 0x04,
  EVERYONE = 0x05,
  MULTI_RANGE_ALLIES = 0x06, // e.g. Shifta
  ALL_ALLIES = 0x07, // e.g. Anti, Resta, Leilla
  ALL = 0x08, // e.g. Last Judgment, Earthquake
  OWN_FCS = 0x09, // e.g. Traitor
};

enum class ConditionType : uint8_t {
  NONE                 = 0x00,
  AP_BOOST             = 0x01, // Temporarily increase AP by N
  RAMPAGE              = 0x02,
  MULTI_STRIKE         = 0x03, // Duplicate attack N times
  DAMAGE_MOD_1         = 0x04, // Set attack damage / AP to N after action cards applied (step 1)
  IMMOBILE             = 0x05, // Give Immobile condition
  HOLD                 = 0x06, // Give Hold condition
  UNKNOWN_07           = 0x07,
  TP_BOOST             = 0x08, // Add N TP temporarily during attack
  GIVE_DAMAGE          = 0x09, // Cause direct N HP loss
  GUOM                 = 0x0A, // Give Guom condition
  PARALYZE             = 0x0B, // Give Paralysis condition
  UNKNOWN_0C           = 0x0C, // Swap AP and TP temporarily (presumably)
  A_H_SWAP             = 0x0D, // Swap AP and HP temporarily
  PIERCE               = 0x0E, // Attack SC directly even if they have items equipped
  UNKNOWN_0F           = 0x0F,
  HEAL                 = 0x10, // Increase HP by N
  RETURN_TO_HAND       = 0x11, // Return card to hand
  UNKNOWN_12           = 0x12,
  UNKNOWN_13           = 0x13,
  ACID                 = 0x14, // Give Acid condition
  UNKNOWN_15           = 0x15,
  MIGHTY_KNUCKLE       = 0x16, // Temporarily increase AP by N, and set ATK dice to zero
  UNIT_BLOW            = 0x17, // Temporarily increase AP by N * number of this card set within phase
  CURSE                = 0x18, // Give Curse condition
  COMBO_AP             = 0x19, // Temporarily increase AP by number of this card set within phase
  PIERCE_RAMPAGE_BLOCK = 0x1A, // Block attack if Pierce/Rampage
  ABILITY_TRAP         = 0x1B, // Temporarily disable opponent abilities
  FREEZE               = 0x1C, // Give Freeze condition
  ANTI_ABNORMALITY_1   = 0x1D, // Cure all abnormal conditions
  UNKNOWN_1E           = 0x1E,
  EXPLOSION            = 0x1F, // Damage all SCs and FCs by number of this same card set * 2
  UNKNOWN_20           = 0x20,
  UNKNOWN_21           = 0x21,
  UNKNOWN_22           = 0x22,
  RETURN_TO_DECK       = 0x23, // Cancel discard and move to bottom of deck instead
  AERIAL               = 0x24, // Give Aerial status
  AP_LOSS              = 0x25, // Make attacker temporarily lose N AP during defense
  BONUS_FROM_LEADER    = 0x26, // Gain AP equal to the number of cards of type N on the field
  FREE_MANEUVER        = 0x27, // Enable movement over occupied tiles
  HASTE                = 0x28, // Multiply all move action costs by expr (which may be zero)
  CLONE                = 0x29, // Make setting this card free if at least one card of type N is already on the field
  DEF_DISABLE_BY_COST  = 0x2A, // Disable use of any defense cards costing between (N / 10) and (N % 10) points, inclusive
  FILIAL               = 0x2B, // Increase controlling SC's HP by N when this card is destroyed
  SNATCH               = 0x2C, // Steal N EXP during attack
  HAND_DISRUPTER       = 0x2D, // Discard N cards from hand immediately
  DROP                 = 0x2E, // Give Drop condition
  ACTION_DISRUPTER     = 0x2F, // Destroy all action cards used by attacker
  SET_HP               = 0x30, // Set HP to N
  NATIVE_SHIELD        = 0x31, // Block attacks from Native creatures
  A_BEAST_SHIELD       = 0x32, // Block attacks from A.Beast creatures
  MACHINE_SHIELD       = 0x33, // Block attacks from Machine creatures
  DARK_SHIELD          = 0x34, // Block attacks from Dark creatures
  SWORD_SHIELD         = 0x35, // Block attacks from Sword items
  GUN_SHIELD           = 0x36, // Block attacks from Gun items
  CANE_SHIELD          = 0x37, // Block attacks from Cane items
  UNKNOWN_38           = 0x38,
  UNKNOWN_39           = 0x39,
  DEFENDER             = 0x3A, // Make attacks go to setter of this card instead of original target
  SURVIVAL_DECOYS      = 0x3B, // Redirect damage for multi-sided attack
  GIVE_OR_TAKE_EXP     = 0x3C, // Give N EXP, or take if N is negative
  UNKNOWN_3D           = 0x3D,
  DEATH_COMPANION      = 0x3E, // If this card has 1 or 2 HP, set its HP to N
  EXP_DECOY            = 0x3F, // If defender has EXP, lose EXP instead of getting damage when attacked
  SET_MV               = 0x40, // Set MV to N
  GROUP                = 0x41, // Temporarily increase AP by N * number of this card on field, excluding itself
  BERSERK              = 0x42, // User of this card receives the same damage as target, and isn't helped by target's defense cards
  GUARD_CREATURE       = 0x43, // Attacks on controlling SC damage this card instead
  TECH                 = 0x44, // Technique cards cost 1 fewer ATK point
  BIG_SWING            = 0x45, // Increase all attacking ATK costs by 1
  UNKNOWN_46           = 0x46,
  SHIELD_WEAPON        = 0x47, // Limit attacker's choice of target to guard items
  ATK_DICE_BOOST       = 0x48, // Increase ATK dice roll by 1
  UNKNOWN_49           = 0x49,
  MAJOR_PIERCE         = 0x4A, // If SC has over half of max HP, attacks target SC instead of equipped items
  HEAVY_PIERCE         = 0x4B, // If SC has 3 or more items equipped, attacks target SC instead of equipped items
  MAJOR_RAMPAGE        = 0x4C, // If SC has over half of max HP, attacks target SC and all equipped items
  HEAVY_RAMPAGE        = 0x4D, // If SC has 3 or more items equipped, attacks target SC and all equipped items
  AP_GROWTH            = 0x4E, // Permanently increase AP by N
  TP_GROWTH            = 0x4F, // Permanently increase TP by N
  REBORN               = 0x50, // If any card of type N is on the field, this card goes to the hand when destroyed instead of being discarded
  COPY                 = 0x51, // Temporarily set AP/TP to N percent (or 100% if N is 0) of opponent's values
  UNKNOWN_52           = 0x52,
  MISC_GUARDS          = 0x53, // Add N to card's defense value
  AP_OVERRIDE          = 0x54, // Set AP to N temporarily
  TP_OVERRIDE          = 0x55, // Set TP to N temporarily
  RETURN               = 0x56, // Return card to hand on destruction instead of discarding
  A_T_SWAP_PERM        = 0x57, // Permanently swap AP and TP
  A_H_SWAP_PERM        = 0x58, // Permanently swap AP and HP
  SLAYERS_ASSASSINS    = 0x59, // Temporarily increase AP during attack
  ANTI_ABNORMALITY_2   = 0x5A, // Remove all conditions
  FIXED_RANGE          = 0x5B, // Use SC's range instead of weapon or attack card ranges
  ELUDE                = 0x5C, // SC does not lose HP when equipped items are destroyed
  PARRY                = 0x5D, // Forward attack to a random FC within one tile of original target, excluding attacker and original target
  BLOCK_ATTACK         = 0x5E, // Completely block attack
  UNKNOWN_5F           = 0x5F,
  UNKNOWN_60           = 0x60,
  COMBO_TP             = 0x61, // Gain TP equal to the number of cards of type N on the field
  MISC_AP_BONUSES      = 0x62, // Temporarily increase AP by N
  MISC_TP_BONUSES      = 0x63, // Temporarily increase TP by N
  UNKNOWN_64           = 0x64,
  MISC_DEFENSE_BONUSES = 0x65, // Decrease damage by N
  MOSTLY_HALFGUARDS    = 0x66, // Reduce damage from incoming attack by N
  PERIODIC_FIELD       = 0x67, // Swap immunity to tech or physical attacks
  FC_LIMIT_BY_COUNT    = 0x68, // Change FC limit from 8 ATK points total to 4 FCs total
  UNKNOWN_69           = 0x69,
  MV_BONUS             = 0x6A, // Increase MV by N
  FORWARD_DAMAGE       = 0x6B,
  WEAK_SPOT_INFLUENCE  = 0x6C, // Temporarily decrease AP by N
  DAMAGE_MODIFIER_2    = 0x6D, // Set attack damage / AP after action cards applied (step 2)
  WEAK_HIT_BLOCK       = 0x6E, // Block all attacks of N damage or less
  AP_SILENCE           = 0x6F, // Temporarily decrease AP of opponent by N
  TP_SILENCE           = 0x70, // Temporarily decrease TP of opponent by N
  A_T_SWAP             = 0x71, // Temporarily swap AP and TP
  HALFGUARD            = 0x72, // Halve damage from attacks that would inflict N or more damage
  UNKNOWN_73           = 0x73,
  RAMPAGE_AP_LOSS      = 0x74, // Temporarily reduce AP by N
  UNKNOWN_75           = 0x75,
  REFLECT              = 0x76, // Generate reverse attack
  UNKNOWN_77           = 0x77,
  ANY                  = 0x78, // Not a real condition; used as a wildcard in search functions
  UNKNOWN_79           = 0x79,
  UNKNOWN_7A           = 0x7A,
  UNKNOWN_7B           = 0x7B,
  UNKNOWN_7C           = 0x7C,
  UNKNOWN_7D           = 0x7D,
  INVALID_FF           = 0xFF,
  ANY_FF               = 0xFF, // Used as a wildcard in some search functions
};

const char* name_for_condition_type(ConditionType cond_type);

enum class AssistEffect : uint16_t {
  NONE              = 0x0000,
  DICE_HALF         = 0x0001,
  DICE_PLUS_1       = 0x0002,
  DICE_FEVER        = 0x0003,
  CARD_RETURN       = 0x0004,
  LAND_PRICE        = 0x0005,
  POWERLESS_RAIN    = 0x0006,
  BRAVE_WIND        = 0x0007,
  SILENT_COLOSSEUM  = 0x0008,
  RESISTANCE        = 0x0009,
  INDEPENDENT       = 0x000A,
  ASSISTLESS        = 0x000B,
  ATK_DICE_2        = 0x000C,
  DEFLATION         = 0x000D,
  INFLATION         = 0x000E,
  EXCHANGE          = 0x000F,
  INFLUENCE         = 0x0010,
  SKIP_SET          = 0x0011,
  SKIP_MOVE         = 0x0012,
  SKIP_ACT          = 0x0013,
  SKIP_DRAW         = 0x0014,
  FLY               = 0x0015,
  NECROMANCER       = 0x0016,
  PERMISSION        = 0x0017,
  SHUFFLE_ALL       = 0x0018,
  LEGACY            = 0x0019,
  ASSIST_REVERSE    = 0x001A,
  STAMINA           = 0x001B,
  AP_ABSORPTION     = 0x001C,
  HEAVY_FOG         = 0x001D,
  TRASH_1           = 0x001E,
  EMPTY_HAND        = 0x001F,
  HITMAN            = 0x0020,
  ASSIST_TRASH      = 0x0021,
  SHUFFLE_GROUP     = 0x0022,
  ASSIST_VANISH     = 0x0023,
  CHARITY           = 0x0024,
  INHERITANCE       = 0x0025,
  FIX               = 0x0026,
  MUSCULAR          = 0x0027,
  CHANGE_BODY       = 0x0028,
  GOD_WHIM          = 0x0029,
  GOLD_RUSH         = 0x002A,
  ASSIST_RETURN     = 0x002B,
  REQUIEM           = 0x002C,
  RANSOM            = 0x002D,
  SIMPLE            = 0x002E,
  SLOW_TIME         = 0x002F,
  QUICK_TIME        = 0x0030,
  TERRITORY         = 0x0031,
  OLD_TYPE          = 0x0032,
  FLATLAND          = 0x0033,
  IMMORTALITY       = 0x0034,
  SNAIL_PACE        = 0x0035,
  TECH_FIELD        = 0x0036,
  FOREST_RAIN       = 0x0037,
  CAVE_WIND         = 0x0038,
  MINE_BRIGHTNESS   = 0x0039,
  RUIN_DARKNESS     = 0x003A,
  SABER_DANCE       = 0x003B,
  BULLET_STORM      = 0x003C,
  CANE_PALACE       = 0x003D,
  GIANT_GARDEN      = 0x003E,
  MARCH_OF_THE_MEEK = 0x003F,
  SUPPORT           = 0x0040,
  RICH              = 0x0041,
  REVERSE_CARD      = 0x0042,
  VENGEANCE         = 0x0043,
  SQUEEZE           = 0x0044,
  HOMESICK          = 0x0045,
  BOMB              = 0x0046,
  SKIP_TURN         = 0x0047,
  BATTLE_ROYALE     = 0x0048,
  DICE_FEVER_PLUS   = 0x0049,
  RICH_PLUS         = 0x004A,
  CHARITY_PLUS      = 0x004B,
  ANY               = 0x004C, // Unused on cards; used in some search functions
};

enum class BattlePhase : uint8_t {
  INVALID_00 = 0,
  DICE = 1,
  SET = 2,
  MOVE = 3,
  ACTION = 4,
  DRAW = 5,
  INVALID_FF = 0xFF,
};

enum class ActionSubphase : uint8_t {
  ATTACK = 0,
  DEFENSE = 2,
  INVALID_FF = 0xFF,
};

const char* name_for_action_subphase(ActionSubphase subphase);

enum class SetupPhase : uint8_t {
  REGISTRATION = 0,
  STARTER_ROLLS = 1,
  HAND_REDRAW_OPTION = 2,
  MAIN_BATTLE = 3,
  BATTLE_ENDED = 4,
  INVALID_FF = 0xFF,
};

enum class RegistrationPhase : uint8_t {
  AWAITING_NUM_PLAYERS = 0, // num_players not set yet
  AWAITING_PLAYERS = 1, // num_players set, but some players not registered
  AWAITING_DECKS = 2, // all players registered, but some decks missing
  REGISTERED = 3, // All players/decks present, but battle not started yet
  BATTLE_STARTED = 4,
  INVALID_FF = 0xFF,
};



enum class Direction : uint8_t {
  RIGHT = 0,
  UP = 1,
  LEFT = 2,
  DOWN = 3,
  INVALID_FF = 0xFF,
};

Direction turn_left(Direction d);
Direction turn_right(Direction d);
Direction turn_around(Direction d);
const char* name_for_direction(Direction d);

struct Location {
  uint8_t x;
  uint8_t y;
  Direction direction;
  uint8_t unused;

  Location();
  Location(uint8_t x, uint8_t y);
  Location(uint8_t x, uint8_t y, Direction direction);
  bool operator==(const Location& other) const;
  bool operator!=(const Location& other) const;

  std::string str() const;

  void clear();
  void clear_FF();
} __attribute__((packed));

struct CardDefinition {
  struct Stat {
    enum Type : uint8_t {
      BLANK = 0,
      STAT = 1,
      PLUS_STAT = 2,
      MINUS_STAT = 3,
      EQUALS_STAT = 4,
      UNKNOWN = 5,
      PLUS_UNKNOWN = 6,
      MINUS_UNKNOWN = 7,
      EQUALS_UNKNOWN = 8,
    };
    be_uint16_t code;
    Type type;
    int8_t stat;

    void decode_code();
    std::string str() const;
  } __attribute__((packed));

  struct Effect {
    uint8_t effect_num;
    ConditionType type;
    ptext<char, 0x0F> expr; // May be blank if the condition type doesn't use it
    uint8_t when;
    ptext<char, 4> arg1;
    ptext<char, 4> arg2;
    ptext<char, 4> arg3;
    CriterionCode apply_criterion;
    uint8_t unknown_a2;

    bool is_empty() const;
    static std::string str_for_arg(const std::string& arg);
    std::string str() const;
  } __attribute__((packed));

  be_uint32_t card_id;
  parray<uint8_t, 0x40> jp_name;
  CardType type; // If <0 (signed), then this is the end of the card list
  uint8_t self_cost; // ATK dice points required
  uint8_t ally_cost; // ATK points from allies required; PBs use this
  uint8_t unused1;
  Stat hp;
  Stat ap;
  Stat tp;
  Stat mv;
  parray<uint8_t, 8> left_colors;
  parray<uint8_t, 8> right_colors;
  parray<uint8_t, 8> top_colors;
  parray<be_uint32_t, 6> range;
  be_uint32_t unused2;
  TargetMode target_mode;
  uint8_t assist_turns; // 90 (dec) = once, 99 (dec) = forever
  uint8_t cannot_move; // 0 for SC and creature cards; 1 for everything else
  uint8_t cannot_attack; // 1 for shields, mags, defense actions, and assist cards
  uint8_t unused3;
  uint8_t hide_in_deck_edit; // 0 = player can use this card (appears in deck edit)
  CriterionCode usable_criterion;
  CardRarity rarity;
  be_uint16_t unknown_a2;
  be_uint16_t be_card_class; // Used for checking attributes (e.g. item types)
  // These two fields seem to always contain the same value, and are always 0
  // for non-assist cards and nonzero for assists. Each assist card has a unique
  // value here and no effects, which makes it look like this is how assist
  // effects are implemented. There seems to be some 1k-modulation going on here
  // too; most cards are in the range 101-174 but a few have e.g. 1150, 2141. A
  // few pairs of cards have the same effect, which makes it look like some
  // other fields are also involved in determining their effects (see e.g. Skip
  // Draw / Skip Move, Dice Fever / Dice Fever +, Reverse Card / Rich +).
  parray<be_uint16_t, 2> assist_effect;
  // Drop rates are decimal-encoded with the following fields:
  // - rate % 10 (that is, the lowest decimal place) specifies the required game
  //   mode. 0 means any mode, 1 means offline only, 2 means 1P free-battle, 3
  //   means 2P+ free battle, 4 means story mode.
  // - (rate / 10) % 100 (that is, the tens and hundreds decimal places) specify
  //   something else, but it's not clear what exactly.
  // - rate / 1000 (the thousands decimal place) specifies the level class
  //   required to get this drop.
  // - rate / 10000 (the ten-thousands decimal place) must be either 0, 1, or 2,
  //   but it's not clear yet what each value means.
  // The drop rates are completely ignored if any of the following are true
  // (which means the card can never be found in a normal post-battle draw):
  // - type is SC_HUNTERS or SC_ARKZ
  // - unknown_a3 is 0x23 or 0x24
  // - rarity is E, D1, D2, or INVIS
  // - hide_in_deck_edit is 1 (specifically 1; other nonzero values here don't
  //   prevent the card from appearing in post-battle draws)
  parray<be_uint16_t, 2> drop_rates;
  ptext<char, 0x14> en_name;
  ptext<char, 0x0B> jp_short_name;
  ptext<char, 0x08> en_short_name;
  Effect effects[3];
  uint8_t unused4;

  bool is_sc() const;
  bool is_fc() const;
  bool is_named_android_sc() const;
  bool any_top_color_matches(const CardDefinition& other) const;
  CardClass card_class() const;

  void decode_range();
  std::string str() const;
} __attribute__((packed)); // 0x128 bytes in total

struct CardDefinitionsFooter {
  be_uint32_t num_cards1;
  be_uint32_t unknown_a1;
  be_uint32_t num_cards2;
  be_uint32_t unknown_a2[11];
  be_uint32_t unknown_offset_a3;
  be_uint32_t unknown_a4[3];
  be_uint32_t footer_offset;
  be_uint32_t unknown_a5[3];
} __attribute__((packed));

struct DeckDefinition {
  ptext<char, 0x10> name;
  be_uint32_t client_id; // 0-3
  // List of card IDs. The card count is the number of nonzero entries here
  // before a zero entry (or 50 if no entries are nonzero). The first card ID is
  // the SC card, which the game implicitly subtracts from the limit - so a
  // valid deck should actually have 31 cards in it.
  parray<le_uint16_t, 50> card_ids;
  be_uint32_t unknown_a1;
  // Last modification time
  le_uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t unknown_a2;
} __attribute__((packed)); // 0x84 bytes in total

struct PlayerConfig {
  // Offsets in comments in this struct are relative to start of 61/98 command
  /* 0728 */ parray<uint8_t, 0x154> unknown_a1;
  /* 087C */ uint8_t is_encrypted;
  /* 087D */ uint8_t basis;
  /* 087E */ parray<uint8_t, 2> unknown_a3;
  // The following fields (here through the beginning of decks) are encrypted
  // using the trivial algorithm, with the basis specified above, if
  // is_encrypted is equal to 1.
  /* 0880 */ parray<uint8_t, 0x2F0> card_counts;
  /* 0B70 */ parray<uint8_t, 0xF8> unknown_a4;
  /* 0C9A */ parray<be_uint16_t, 50> unknown_a5;
  // This field appears to be doubly-encrypted, likely with the same trivial
  // algorithm (but not the same basis).
  /* 0CCC */ parray<uint8_t, 0x70> unknown_a6;
  /* 0D3C */ parray<uint8_t, 0xE20> unknown_a7;
  /* 1B5C */ parray<DeckDefinition, 25> decks;
  /* 2840 */ uint64_t unknown_a8;
  /* 2848 */ be_uint32_t offline_clv_exp; // CLvOff = this / 100
  /* 284C */ be_uint32_t online_clv_exp; // CLvOn = this / 100
  /* 2850 */ parray<uint8_t, 0x14C> unknown_a9;
  /* 299C */ ptext<char, 0x10> name;
  // Other records are probably somewhere in here - e.g. win/loss, play time, etc.
  /* 29AC */ parray<uint8_t, 0xCC> unknown_a10;
} __attribute__((packed));

enum class HPType : uint8_t {
  DEFEAT_PLAYER = 0,
  DEFEAT_TEAM = 1,
  COMMON_HP = 2,
};

enum class DiceExchangeMode : uint8_t {
  HIGH_ATK = 0,
  HIGH_DEF = 1,
  NONE = 2,
};

enum class AllowedCards : uint8_t {
  ALL = 0,
  N_ONLY = 1,
  N_R_ONLY = 2,
  N_R_S_ONLY = 3,
};

struct Rules {
  // When this structure is used in a map/quest definition, FF in any of these
  // fields means the user is allowed to override it. Any non-FF fields are
  // fixed for the map/quest and cannot be overridden.
  uint8_t overall_time_limit; // In increments of 5 minutes; 0 = unlimited
  uint8_t phase_time_limit; // In seconds; 0 = unlimited
  AllowedCards allowed_cards;
  uint8_t min_dice; // 0 = default (1)
  // 4
  uint8_t max_dice; // 0 = default (6)
  uint8_t disable_deck_shuffle; // 0 = shuffle on, 1 = off
  uint8_t disable_deck_loop; // 0 = loop on, 1 = off
  uint8_t char_hp;
  // 8
  HPType hp_type;
  uint8_t no_assist_cards; // 1 = assist cards disallowed
  uint8_t disable_dialogue; // 0 = dialogue on, 1 = dialogue off
  DiceExchangeMode dice_exchange_mode;
  // C
  uint8_t disable_dice_boost; // 0 = dice boost on, 1 = off
  parray<uint8_t, 3> unused;

  Rules();
  explicit Rules(std::shared_ptr<const JSONObject> json);
  std::shared_ptr<JSONObject> json() const;
  bool operator==(const Rules& other) const;
  bool operator!=(const Rules& other) const;
  void clear();
  void set_defaults();

  bool check_invalid_fields() const;
  bool check_and_reset_invalid_fields();

  std::string str() const;
} __attribute__((packed));

struct StateFlags {
  le_uint16_t turn_num;
  BattlePhase battle_phase;
  uint8_t current_team_turn1;
  uint8_t current_team_turn2;
  ActionSubphase action_subphase;
  SetupPhase setup_phase;
  RegistrationPhase registration_phase;
  parray<le_uint32_t, 2> team_exp;
  parray<uint8_t, 2> team_dice_boost;
  uint8_t first_team_turn;
  uint8_t tournament_flag;
  parray<CardType, 4> client_sc_card_types;

  StateFlags();
  bool operator==(const StateFlags& other) const;
  bool operator!=(const StateFlags& other) const;
  void clear();
  void clear_FF();
} __attribute__((packed));



struct MapList {
  be_uint32_t num_maps;
  be_uint32_t unknown_a1; // Always 0?
  be_uint32_t strings_offset; // From after total_size field (add 0x10 to this value)
  be_uint32_t total_size; // Including header, entries, and strings

  struct Entry { // Should be 0x220 bytes in total
    be_uint16_t map_x;
    be_uint16_t map_y;
    be_uint16_t environment_number;
    be_uint16_t map_number;
    // Text offsets are from the beginning of the strings block after all map
    // entries (that is, add strings_offset to them to get the string offset)
    be_uint32_t name_offset;
    be_uint32_t location_name_offset;
    be_uint32_t quest_name_offset;
    be_uint32_t description_offset;
    be_uint16_t width;
    be_uint16_t height;
    parray<parray<uint8_t, 0x10>, 0x10> map_tiles;
    parray<parray<uint8_t, 0x10>, 0x10> modification_tiles;
    // This appears to be 0xFF000000 for free battle maps, and 0 for quests.
    // TODO: Figure out what this field's meaning actually is
    be_uint32_t unknown_a2;
  } __attribute__((packed));

  // Variable-length fields:
  // Entry entries[num_maps];
  // char strings[...EOF]; // Null-terminated strings, pointed to by offsets in Entry structs
} __attribute__((packed));

struct CompressedMapHeader { // .mnm file format
  le_uint32_t map_number;
  le_uint32_t compressed_data_size;
  // Compressed data immediately follows (which decompresses to a MapDefinition)
} __attribute__((packed));

struct MapDefinition { // .mnmd format; also the format of (decompressed) quests
  /* 0000 */ be_uint32_t unknown_a1;
  /* 0004 */ be_uint32_t map_number;
  /* 0008 */ uint8_t width;
  /* 0009 */ uint8_t height;
  // The environment number specifies several things:
  // - The model to load for the main battle stage
  // - The music to play during the main battle
  // - The color of the battle tile outlines (probably)
  // - The preview image to show in the upper-left corner in the map select menu
  // The environment numbers are:
  // 00 - Unguis Lapis
  // 01 - Nebula Montana (1)
  // 02 - Lupus Silva (1)
  // 03 - Lupus Silva (2)
  // 04 - Molae Venti
  // 05 - Nebula Montana (2)
  // 06 - Tener Sinus
  // 07 - Mortis Fons
  // 08 - Morgue (destroyed)
  // 09 - Tower of Caelum
  // 0A = ??? (referred to as "^mapname"; crashes)
  // 0B = Cyber
  // 0C = Morgue (not destroyed)
  // 0D = (Castor/Pollux map)
  // 0E - Dolor Odor
  // 0F = Ravum Aedes Sacra
  // 10 - (Amplum Umbla map)
  // 11 - Via Tubus
  // 12 = Morgue (same as 08?)
  // 13 = ??? (crashes)
  // Environment numbers beyond 13 are not used in any known quests or maps.
  /* 000A */ uint8_t environment_number;
  // All alt_maps fields (including the floats) past num_alt_maps are filled in
  // with FF. For example, if num_alt_maps == 8, the last two fields in each
  // alt_maps array are filled with FF.
  /* 000B */ uint8_t num_alt_maps; // TODO: What are the alt maps for?
  // In the map_tiles array, the values are:
  // 00 = not a valid tile
  // 01 = valid tile unless punched out (later)
  // 02 = team A start (1v1)
  // 03, 04 = team A start (2v2)
  // 05 = ???
  // 06, 07 = team B start (2v2)
  // 08 = team B start (1v1)
  // Note that the game displays the map reversed vertically in the preview
  // window. For example, player 1 is on team A, which usually starts at the top
  // of the map as defined in this struct, or at the bottom as shown in the
  // preview window.
  /* 000C */ parray<parray<uint8_t, 0x10>, 0x10> map_tiles;
  // The start_tile_definitions field is a list of 6 bytes for each team. The
  // low 6 bits of each byte match the starting location for the relevant player
  // in map_tiles; the high 2 bits are the player's initial facing direction.
  // - If the team has 1 player, only byte [0] is used.
  // - If the team has 2 players, bytes [1] and [2] are used.
  // - If the team has 3 players, bytes [3] through [5] are used.
  /* 010C */ parray<parray<uint8_t, 6>, 2> start_tile_definitions;
  /* 0118 */ parray<parray<uint8_t, 0x10>, 0x10> alt_maps1[2][0x0A];
  /* 1518 */ parray<be_float, 0x12> alt_maps_unknown_a3[2][0x0A];
  /* 1AB8 */ parray<be_float, 0x24> unknown_a5[3];
  // In the modification_tiles array, the values are:
  // 10 = blocked (as if the corresponding map_tiles value was 00)
  // 20 = blocked (maybe one of 10 or 20 are passable by Aerial characters)
  // 30-34 = teleporters (2 of each value may be present)
  // 40-44 = traps (one of each type is chosen at random to be a real trap at
  //         battle start time)
  // 50 = appears as improperly-z-buffered teal cube in preview, behaves as a
  //      blocked tile (like 10 and 20)
  /* 1C68 */ parray<parray<uint8_t, 0x10>, 0x10> modification_tiles;
  /* 1D68 */ parray<uint8_t, 0x74> unknown_a6;
  /* 1DDC */ Rules default_rules;
  /* 1DEC */ parray<uint8_t, 4> unknown_a7;
  /* 1DF0 */ ptext<char, 0x14> name;
  /* 1E04 */ ptext<char, 0x14> location_name;
  /* 1E18 */ ptext<char, 0x3C> quest_name; // == location_name if not a quest
  /* 1E54 */ ptext<char, 0x190> description;
  /* 1FE4 */ be_uint16_t map_x;
  /* 1FE6 */ be_uint16_t map_y;
  struct NPCDeck {
    ptext<char, 0x18> name;
    parray<be_uint16_t, 0x20> card_ids; // Last one appears to always be FFFF
  } __attribute__((packed));
  /* 1FE8 */ NPCDeck npc_decks[3]; // Unused if name[0] == 0
  struct NPCCharacter {
    parray<be_uint16_t, 2> unknown_a1;
    parray<uint8_t, 4> unknown_a2;
    ptext<char, 0x10> name;
    parray<be_uint16_t, 0x7E> unknown_a3;
  } __attribute__((packed));
  /* 20F0 */ NPCCharacter npc_chars[3]; // Unused if name[0] == 0
  /* 242C */ parray<uint8_t, 0x14> unknown_a8; // Always FF?
  /* 2440 */ ptext<char, 0x190> before_message;
  /* 25D0 */ ptext<char, 0x190> after_message;
  /* 2760 */ ptext<char, 0x190> dispatch_message; // Usually "You can only dispatch <character>" or blank
  struct DialogueSet {
    be_uint16_t unknown_a1;
    be_uint16_t unknown_a2; // Always 0x0064 if valid, 0xFFFF if unused?
    ptext<char, 0x40> strings[4];
  } __attribute__((packed)); // Total size: 0x104 bytes
  /* 28F0 */ DialogueSet dialogue_sets[3][0x10]; // Up to 0x10 per valid NPC
  /* 59B0 */ parray<be_uint16_t, 0x10> reward_card_ids;
  /* 59D0 */ parray<uint8_t, 0x0C> unknown_a9;
  /* 59DC */ uint8_t unknown_a10;
  /* 59DD */ parray<uint8_t, 0x3B> unknown_a11;
  /* 5A18 */

  std::string str(const DataIndex* data_index = nullptr) const;
} __attribute__((packed));



struct COMDeckDefinition {
  size_t index;
  std::string player_name;
  std::string deck_name;
  parray<le_uint16_t, 0x1F> card_ids;
};



class DataIndex {
public:
  DataIndex(const std::string& directory, uint32_t behavior_flags);

  struct CardEntry {
    CardDefinition def;
    std::string text;
    std::vector<std::string> debug_tags; // Empty unless debug == true
  };

  class MapEntry {
  public:
    MapDefinition map;
    bool is_quest;

    MapEntry(const MapDefinition& map, bool is_quest);
    MapEntry(const std::string& compressed_data, bool is_quest);

    std::string compressed() const;

  private:
    mutable std::string compressed_data;
  };

  const std::string& get_compressed_card_definitions() const;
  std::shared_ptr<const CardEntry> definition_for_card_id(uint32_t id) const;
  std::shared_ptr<const CardEntry> definition_for_card_name(
      const std::string& name) const;
  std::set<uint32_t> all_card_ids() const;

  const std::string& get_compressed_map_list() const;
  std::shared_ptr<const MapEntry> definition_for_map_number(uint32_t id) const;
  std::shared_ptr<const MapEntry> definition_for_map_name(
      const std::string& name) const;
  std::set<uint32_t> all_map_ids() const;

  size_t num_com_decks() const;
  std::shared_ptr<const COMDeckDefinition> com_deck(size_t which) const;
  std::shared_ptr<const COMDeckDefinition> com_deck(const std::string& name) const;
  std::shared_ptr<const COMDeckDefinition> random_com_deck() const;

  const uint32_t behavior_flags;

private:
  std::string compressed_card_definitions;
  std::unordered_map<uint32_t, std::shared_ptr<CardEntry>> card_definitions;
  std::unordered_map<std::string, std::shared_ptr<CardEntry>> card_definitions_by_name;

  // The compressed map list is generated on demand from the maps map below.
  // It's marked mutable because the logical consistency of the DataIndex object
  // is not violated from the caller's perspective even if we don't generate the
  // compressed map list at load time.
  mutable std::string compressed_map_list;
  std::map<uint32_t, std::shared_ptr<MapEntry>> maps;
  std::unordered_map<std::string, std::shared_ptr<MapEntry>> maps_by_name;

  std::vector<std::shared_ptr<COMDeckDefinition>> com_decks;
  std::unordered_map<std::string, std::shared_ptr<COMDeckDefinition>> com_decks_by_name;
};



} // namespace Episode3
