#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef enum {
    GAME_NORMAL = 1,     // includes line/diagonal/corners
    GAME_FULL_HOUSE = 2
} GameMode;

typedef struct {
    uint32_t wins;
    uint32_t losses;
    uint32_t draws;
} Record;

typedef struct {
    uint32_t id;
    char name[64];
    double balance;    // money owned by player
    Record record;     // wins/losses/draws
    uint32_t cards_owned; // cards purchased in current match
    uint32_t lifetime_cards; // for stats
    double total_recharged; // cumulative added funds
    double total_spent;     // cumulative spent on cards
    double total_won;       // cumulative winnings from payouts
} Player;

typedef struct {
    GameMode mode;
    double card_cost;          // cost per card for this match
    double pot;                // total money collected in this match
    double saved_for_fullhouse;// amount saved from this match for final full house
    uint32_t match_number;
    uint32_t winners[64];      // player ids who won
    uint32_t winner_count;     // number of winners
    uint8_t active;            // 1 when active/in-progress, 0 otherwise
} Match;

typedef struct {
    double total_bank;         // house/account balance tracking
    double saved_pot;          // accumulated pot reserved for final full house
    uint32_t total_matches;
} Accounting;

#endif // TYPES_H
