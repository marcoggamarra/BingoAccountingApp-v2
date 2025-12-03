#include "config.h"

static double NORMAL_CARD_COST = 0.25; // rule 4
static double FULLHOUSE_CARD_COST = 0.0; // undefined until set
static double SAVED_POT_PERCENTAGE = 0.15; // rule 5 (example default 15%)
static uint32_t MAX_PLAYERS = 512; // capacity
static int ALLOW_MULTI_WINNERS = 1; // rule 3

void cfg_init_defaults(void) {
    NORMAL_CARD_COST = 0.25;
    FULLHOUSE_CARD_COST = 0.0;
    SAVED_POT_PERCENTAGE = 0.15;
    MAX_PLAYERS = 512;
    ALLOW_MULTI_WINNERS = 1;
}

double cfg_get_normal_card_cost(void) { return NORMAL_CARD_COST; }
void   cfg_set_normal_card_cost(double cost) { NORMAL_CARD_COST = (cost >= 0.0) ? cost : NORMAL_CARD_COST; }

double cfg_get_fullhouse_card_cost(void) { return FULLHOUSE_CARD_COST; }
void   cfg_set_fullhouse_card_cost(double cost) { FULLHOUSE_CARD_COST = (cost >= 0.0) ? cost : FULLHOUSE_CARD_COST; }

double cfg_get_saved_pot_percentage(void) { return SAVED_POT_PERCENTAGE; }
void   cfg_set_saved_pot_percentage(double pct) {
    if (pct < 0.0) pct = 0.0;
    if (pct > 1.0) pct = 1.0;
    SAVED_POT_PERCENTAGE = pct;
}

uint32_t cfg_get_max_players(void) { return MAX_PLAYERS; }
void     cfg_set_max_players(uint32_t maxp) { if (maxp > 0) MAX_PLAYERS = maxp; }

int cfg_get_allow_multi_winners(void) { return ALLOW_MULTI_WINNERS; }
void cfg_set_allow_multi_winners(int allow) { ALLOW_MULTI_WINNERS = allow ? 1 : 0; }

int cfg_validate(void) {
    return NORMAL_CARD_COST >= 0.0 && SAVED_POT_PERCENTAGE >= 0.0 && SAVED_POT_PERCENTAGE <= 1.0 && MAX_PLAYERS > 0;
}
