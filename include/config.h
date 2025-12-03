#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Global configuration accessible via functions
void cfg_init_defaults(void);

// Tunables (mutable via setters)
double cfg_get_normal_card_cost(void);
void   cfg_set_normal_card_cost(double cost);

double cfg_get_fullhouse_card_cost(void); // can be undefined (<=0) if not set
void   cfg_set_fullhouse_card_cost(double cost);

// Percentage of each normal match pot saved for final full house
// value in [0,1]
double cfg_get_saved_pot_percentage(void);
void   cfg_set_saved_pot_percentage(double pct);

// Max players supported in roster
uint32_t cfg_get_max_players(void);
void     cfg_set_max_players(uint32_t maxp);

// Enable/disable multiple winners in normal games
int  cfg_get_allow_multi_winners(void);
void cfg_set_allow_multi_winners(int allow);

// Validation helpers
int  cfg_validate(void);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H
