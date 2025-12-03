#include <stdio.h>
#include <string.h>
#include "bingo.h"
#include "config.h"

void engine_init(Accounting* acc) {
    cfg_init_defaults();
    acc->total_bank = 0.0;
    acc->saved_pot = 0.0;
    acc->total_matches = 0;
}

int engine_add_player(Player* roster, uint32_t* roster_count, const char* name, double initial_balance) {
    uint32_t maxp = cfg_get_max_players();
    if (*roster_count >= maxp) return -1;
    uint32_t id = *roster_count ? roster[*roster_count - 1].id + 1 : 1;
    Player p = {0};
    p.id = id;
    strncpy(p.name, name ? name : "Player", sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0';
    p.balance = initial_balance;
    p.record.wins = p.record.losses = p.record.draws = 0;
    p.cards_owned = 0;
    p.lifetime_cards = 0;
    roster[*roster_count] = p;
    (*roster_count)++;
    return (int)id;
}

int engine_remove_player(Player* roster, uint32_t* roster_count, uint32_t player_id) {
    for (uint32_t i = 0; i < *roster_count; ++i) {
        if (roster[i].id == player_id) {
            // compact
            for (uint32_t j = i + 1; j < *roster_count; ++j) roster[j - 1] = roster[j];
            (*roster_count)--;
            return 0;
        }
    }
    return -1;
}

Player* engine_find_player(Player* roster, uint32_t roster_count, uint32_t player_id) {
    for (uint32_t i = 0; i < roster_count; ++i) if (roster[i].id == player_id) return &roster[i];
    return NULL;
}

void match_start(Match* m, GameMode mode, double card_cost) {
    m->mode = mode;
    m->card_cost = card_cost > 0.0 ? card_cost : (mode == GAME_FULL_HOUSE ? cfg_get_fullhouse_card_cost() : cfg_get_normal_card_cost());
    m->pot = 0.0;
    m->saved_for_fullhouse = 0.0;
    m->winner_count = 0;
    m->active = 1;
}

void match_buy_cards(Match* m, Player* p, uint32_t count) {
    if (!m->active || count == 0) return;
    double cost = m->card_cost * (double)count;
    // player pays cost, pot increases
    p->balance -= cost;
    p->cards_owned += count;
    p->lifetime_cards += count;
    p->total_spent += cost;
    m->pot += cost;
}

int match_add_winner(Match* m, Player* roster, uint32_t roster_count, uint32_t player_id) {
    if (!m->active) return -1;
    if (m->mode != GAME_FULL_HOUSE && !cfg_get_allow_multi_winners() && m->winner_count > 0) return -2;
    if (m->winner_count >= sizeof(m->winners)/sizeof(m->winners[0])) return -3;
    // duplicate check
    for (uint32_t i = 0; i < m->winner_count; ++i) if (m->winners[i] == player_id) return -6;
    Player* p = engine_find_player(roster, roster_count, player_id);
    if (!p) return -4;
    if (p->cards_owned == 0) return -5; // did not participate
    m->winners[m->winner_count++] = player_id;
    return 0;
}

int match_remove_winner(Match* m, uint32_t player_id) {
    if (!m->active) return -1;
    for (uint32_t i = 0; i < m->winner_count; ++i) {
        if (m->winners[i] == player_id) {
            for (uint32_t j = i + 1; j < m->winner_count; ++j) m->winners[j - 1] = m->winners[j];
            m->winner_count--;
            return 0;
        }
    }
    return -1;
}

void apply_payouts_normal(Match* m, Player* roster, uint32_t roster_count) {
    // Save a percentage for final full house
    double save_pct = cfg_get_saved_pot_percentage();
    double to_save = m->pot * save_pct;
    double distributable = m->pot - to_save;
    if (m->winner_count == 0) return; // draw (no payout)
    double per_winner = distributable / (double)m->winner_count;
    for (uint32_t i = 0; i < m->winner_count; ++i) {
        Player* p = engine_find_player(roster, roster_count, m->winners[i]);
        if (p) {
            p->balance += per_winner;
            p->record.wins++;
            p->total_won += per_winner;
        }
    }
    // losers increment losses, winners handled above; draws handled elsewhere
    for (uint32_t i = 0; i < roster_count; ++i) {
        int is_winner = 0;
        for (uint32_t w = 0; w < m->winner_count; ++w) if (roster[i].id == m->winners[w]) { is_winner = 1; break; }
        if (!is_winner) {
            // only consider players who purchased cards this match
            if (roster[i].cards_owned > 0) roster[i].record.losses++;
        }
    }
    m->saved_for_fullhouse = to_save;
}

void apply_payouts_fullhouse(Accounting* acc, Match* m, Player* roster, uint32_t roster_count, uint32_t* winners, uint32_t winner_count) {
    if (winner_count == 0) return;
    // Distribute both accumulated saved pot and current full house match pot
    double total_distributable = acc->saved_pot + m->pot;
    double per_winner = total_distributable / (double)winner_count;
    for (uint32_t i = 0; i < winner_count; ++i) {
        Player* p = engine_find_player(roster, roster_count, winners[i]);
        if (p) {
            p->balance += per_winner;
            p->record.wins++;
            p->total_won += per_winner;
        }
    }
    // Mark losses for participants who are not winners
    for (uint32_t i = 0; i < roster_count; ++i) {
        int is_winner = 0;
        for (uint32_t w = 0; w < winner_count; ++w) if (roster[i].id == winners[w]) { is_winner = 1; break; }
        if (!is_winner && roster[i].cards_owned > 0) roster[i].record.losses++;
    }
    // Clear saved pot and match pot after distribution
    acc->saved_pot = 0.0;
    m->pot = 0.0;
}

void match_end(Match* m, Accounting* acc, Player* roster, uint32_t roster_count) {
    if (!m->active) return;
    if (m->mode == GAME_FULL_HOUSE) {
        // Winners get saved pot + current match pot
        apply_payouts_fullhouse(acc, m, roster, roster_count, m->winners, m->winner_count);
    } else {
        apply_payouts_normal(m, roster, roster_count);
        acc->saved_pot += m->saved_for_fullhouse;
    }
    // reset per-match player state
    for (uint32_t i = 0; i < roster_count; ++i) roster[i].cards_owned = 0;
    m->active = 0;
    acc->total_matches++;
}

void match_cancel(Match* m, Player* roster, uint32_t roster_count) {
    if (!m->active) return;
    // Refund purchases: each player's cards_owned * card_cost back to balance
    for (uint32_t i = 0; i < roster_count; ++i) {
        if (roster[i].cards_owned > 0) {
            double refund = (double)roster[i].cards_owned * m->card_cost;
            roster[i].balance += refund;
            // Adjust total_spent, since cancellation negates spend
            roster[i].total_spent -= refund;
            roster[i].cards_owned = 0;
        }
    }
    // Reset match
    m->pot = 0.0;
    m->saved_for_fullhouse = 0.0;
    m->winner_count = 0;
    m->active = 0;
}
