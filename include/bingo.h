#ifndef BINGO_H
#define BINGO_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Engine lifecycle
void engine_init(Accounting* acc);

// Roster management
int  engine_add_player(Player* roster, uint32_t* roster_count, const char* name, double initial_balance);
int  engine_remove_player(Player* roster, uint32_t* roster_count, uint32_t player_id);
Player* engine_find_player(Player* roster, uint32_t roster_count, uint32_t player_id);

// Match management
void match_start(Match* m, GameMode mode, double card_cost);
void match_buy_cards(Match* m, Player* p, uint32_t count);
void match_end(Match* m, Accounting* acc, Player* roster, uint32_t roster_count);
void match_cancel(Match* m, Player* roster, uint32_t roster_count); // refunds purchases and resets match

// Results input
// Adds a winner only if player exists and purchased at least one card this match.
// Returns codes:
//  0  success
// -1  match inactive
// -2  multi-winner disabled (normal mode and already has a winner)
// -3  winners array full
// -4  player not found
// -5  player has no cards this match (not solvent/participated)
// -6  duplicate winner
int  match_add_winner(Match* m, Player* roster, uint32_t roster_count, uint32_t player_id);
int  match_remove_winner(Match* m, uint32_t player_id); // returns 0 if removed, -1 not found

// Accounting utilities
void apply_payouts_normal(Match* m, Player* roster, uint32_t roster_count);
// Full house now distributes (saved_pot + current match pot)
void apply_payouts_fullhouse(Accounting* acc, Match* m, Player* roster, uint32_t roster_count, uint32_t* winners, uint32_t winner_count);

#ifdef __cplusplus
}
#endif

#endif // BINGO_H
