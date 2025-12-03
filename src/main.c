// Interactive CLI for Bingo accounting
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bingo.h"
#include "config.h"
#include "persist.h"

#define MAX_ROSTER 512

static void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void wait_for_enter(void) {
    printf("\nPress Enter to continue...");
    int c;
    // Consume leftover characters up to newline
    while ((c = getchar()) != '\n' && c != EOF) {}
    // Read one line to pause
    char buf[4];
    fgets(buf, sizeof(buf), stdin);
}

static void print_menu(void) {
    printf("\n=== Bingo Accounting Menu ===\n");
    printf("1  - List players\n");
    printf("2  - Add player\n");
    printf("3  - Remove player\n");
    printf("4  - Start match\n");
    printf("5  - Buy cards for player\n");
    printf("6  - Add winner\n");
    printf("106- Remove winner\n");
    printf("7  - End match\n");
    printf("107- Cancel match (refund purchases)\n");
    printf("8  - Show accounting / saved pot\n");
    printf("9  - Set normal card cost\n");
    printf("10 - Set full house card cost\n");
    printf("11 - Set saved pot percentage\n");
    printf("12 - Toggle multi winners (normal matches)\n");
    printf("13 - Show config\n");
    printf("14 - Preview current match distribution\n");
    printf("15 - Recharge player balance\n");
    printf("16 - Export players CSV (financial summary)\n");
    printf("17 - Save now (checkpoint)\n");
    printf("0  - Exit\n");
    printf("Select: ");
}

static void list_players(Player* roster, uint32_t roster_count) {
    printf("\nPlayers (%u):\n", roster_count);
    for (uint32_t i = 0; i < roster_count; ++i) {
        Player* p = &roster[i];
        printf("ID:%u Name:%s Bal:%.2f W:%u L:%u D:%u CardsThisMatch:%u\n", p->id, p->name, p->balance, p->record.wins, p->record.losses, p->record.draws, p->cards_owned);
    }
}

static GameMode ask_game_mode(void) {
    printf("Game Modes: 1-Normal (line/diagonal/corners) 2-FullHouse\nEnter mode number: ");
    int m = 0; if (scanf("%d", &m) != 1) { while (getchar()!='\n'); return GAME_NORMAL; }
    switch (m) {
        case 1: return GAME_NORMAL;
        case 2: return GAME_FULL_HOUSE;
        default: return GAME_NORMAL;
    }
}

int main(void) {
    Accounting acc; engine_init(&acc);
    // Attempt to load previous accounting
    persist_load_accounting("data/accounting.bin", &acc);
    Player roster[MAX_ROSTER];
    uint32_t roster_count = 0;
    // Attempt to load previous roster
    persist_load_roster("data/roster.bin", roster, &roster_count, MAX_ROSTER);
    Match current_match; memset(&current_match, 0, sizeof(current_match));
    int has_active_match = 0;

    // Persistent participation configuration between matches
    int last_participate[MAX_ROSTER];
    uint32_t last_cards[MAX_ROSTER];
    memset(last_participate, 0, sizeof(last_participate));
    memset(last_cards, 0, sizeof(last_cards));

    // Removed automatic seed players (Alice, Bob) to keep tests deterministic.

    int running = 1;
    while (running) {
        clear_screen();
        print_menu();
        int choice = -1;
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); continue; }
        switch (choice) {
            case 1: // list players
                clear_screen();
                list_players(roster, roster_count);
                wait_for_enter();
                break;
            case 2: { // add player
                clear_screen();
                char name[64];
                double bal;
                printf("Enter name: ");
                scanf("%63s", name);
                printf("Initial balance: ");
                if (scanf("%lf", &bal) != 1) { bal = 0.0; }
                int id = engine_add_player(roster, &roster_count, name, bal);
                if (id < 0) printf("Failed to add player (max reached).\n");
                else printf("Added player ID %d.\n", id);
                wait_for_enter();
            } break;
            case 3: { // remove
                clear_screen();
                uint32_t id; printf("Player ID to remove: ");
                if (scanf("%u", &id) == 1) {
                    if (engine_remove_player(roster, &roster_count, id) == 0) printf("Removed player %u.\n", id);
                    else printf("Player not found.\n");
                }
                wait_for_enter();
            } break;
            case 4: { // start match
                clear_screen();
                if (has_active_match) { printf("A match is already active. End it first.\n"); break; }
                GameMode gm = ask_game_mode();
                double override_cost = 0.0;
                printf("Override card cost (0 to use default): ");
                scanf("%lf", &override_cost);
                match_start(&current_match, gm, override_cost);
                has_active_match = 1;
                printf("Match started (mode=%d, card_cost=%.2f).\n", gm, current_match.card_cost);

                // Ask to reuse last configuration for participation & card counts
                if (roster_count > 0) {
                    printf("Reuse last participation & card counts? (y/n): ");
                    char ans = 'n';
                    scanf(" %c", &ans);
                    if (ans == 'y' || ans == 'Y') {
                        for (uint32_t i = 0; i < roster_count; ++i) {
                            if (last_participate[i] && last_cards[i] > 0) {
                                Player* p = &roster[i];
                                double total_cost = current_match.card_cost * (double)last_cards[i];
                                if (p->balance >= total_cost) {
                                    match_buy_cards(&current_match, p, last_cards[i]);
                                } else {
                                    printf("Player %s lacks balance for %u cards (needs %.2f). Skipped.\n", p->name, last_cards[i], total_cost);
                                }
                            }
                        }
                    } else {
                        // Gather new participation config
                        for (uint32_t i = 0; i < roster_count; ++i) {
                            Player* p = &roster[i];
                            printf("Include player %s (ID:%u)? (y/n): ", p->name, p->id);
                            char inc = 'n';
                            scanf(" %c", &inc);
                            if (inc == 'y' || inc == 'Y') {
                                last_participate[i] = 1;
                                uint32_t cc = 0;
                                printf("Cards to buy for %s: ", p->name);
                                if (scanf("%u", &cc) != 1) cc = 0;
                                last_cards[i] = cc;
                                if (cc > 0) {
                                    double total_cost = current_match.card_cost * (double)cc;
                                    if (p->balance >= total_cost) {
                                        match_buy_cards(&current_match, p, cc);
                                    } else {
                                        printf("Insufficient balance (needs %.2f). Purchase skipped.\n", total_cost);
                                        last_cards[i] = 0; // revert
                                    }
                                }
                            } else {
                                last_participate[i] = 0; last_cards[i] = 0;
                            }
                        }
                    }
                }
                // Show quick summary
                printf("Match pot so far: %.2f\n", current_match.pot);
                double save_pct = (gm == GAME_FULL_HOUSE) ? 0.0 : cfg_get_saved_pot_percentage();
                double to_save = current_match.pot * save_pct;
                double distributable = current_match.pot - to_save;
                printf("Projected save: %.2f, distributable (before winners): %.2f\n", to_save, distributable);
                wait_for_enter();
            } break;
            case 5: { // buy cards
                clear_screen();
                if (!has_active_match) { printf("No active match.\n"); break; }
                uint32_t id, count; printf("Player ID: ");
                if (scanf("%u", &id) != 1) break;
                printf("Cards to buy: ");
                if (scanf("%u", &count) != 1) break;
                Player* p = engine_find_player(roster, roster_count, id);
                if (!p) { printf("Player not found.\n"); break; }
                double total_cost = current_match.card_cost * (double)count;
                if (p->balance < total_cost) { printf("Insufficient balance (need %.2f).\n", total_cost); break; }
                match_buy_cards(&current_match, p, count);
                printf("Player %u bought %u cards.\n", id, count);
                {
                    char details[128]; snprintf(details, sizeof(details), "buy,id=%u,count=%u,cost=%.2f", id, count, total_cost);
                    persist_append_transaction("data/transactions.csv", "buy", details);
                }
                wait_for_enter();
            } break;
            case 6: { // add winner
                clear_screen();
                if (!has_active_match) { printf("No active match.\n"); break; }
                uint32_t id; printf("Winner player ID: ");
                if (scanf("%u", &id) != 1) break;
                int r = match_add_winner(&current_match, roster, roster_count, id);
                switch (r) {
                    case 0: printf("Added winner %u.\n", id); break;
                    case -1: printf("Match inactive.\n"); break;
                    case -2: printf("Multi-winner disabled; winner already set.\n"); break;
                    case -3: printf("Winner list full.\n"); break;
                    case -4: printf("Player not found.\n"); break;
                    case -5: printf("Player did not buy cards this match; cannot win.\n"); break;
                    case -6: printf("Duplicate winner rejected.\n"); break;
                    default: printf("Unknown error adding winner (%d).\n", r); break;
                }
                wait_for_enter();
            } break;
            case 106: { // remove winner
                clear_screen();
                if (!has_active_match) { printf("No active match.\n"); break; }
                uint32_t id; printf("Winner player ID to remove: ");
                if (scanf("%u", &id) != 1) break;
                int r = match_remove_winner(&current_match, id);
                if (r == 0) printf("Removed winner %u.\n", id);
                else printf("Winner not found in current match.\n");
                wait_for_enter();
            } break;
            case 7: { // end match
                clear_screen();
                if (!has_active_match) { printf("No active match.\n"); break; }
                // Enforce at least one winner if there were participants (any cards bought)
                int had_participants = 0;
                for (uint32_t i = 0; i < roster_count; ++i) { if (roster[i].cards_owned > 0) { had_participants = 1; break; } }
                if (had_participants && current_match.winner_count == 0) {
                    printf("Cannot end match: at least one winner required (participants detected).\n");
                    wait_for_enter();
                    break;
                }
                match_end(&current_match, &acc, roster, roster_count);
                has_active_match = 0;
                printf("Match ended. Saved pot total: %.2f\n", acc.saved_pot);
                // Ensure data directory exists (best-effort via system call omitted); save state
                persist_save_roster("data/roster.bin", roster, roster_count);
                persist_save_accounting("data/accounting.bin", &acc);
                // Append match history (CSV-like)
                persist_append_match("data/matches.csv", &current_match);
                wait_for_enter();
            } break;
            case 107: { // cancel match
                clear_screen();
                if (!has_active_match) { printf("No active match.\n"); break; }
                match_cancel(&current_match, roster, roster_count);
                has_active_match = 0;
                printf("Match cancelled. Purchases refunded.\n");
                persist_save_roster("data/roster.bin", roster, roster_count);
                persist_save_accounting("data/accounting.bin", &acc);
                persist_append_transaction("data/transactions.csv", "cancel_match", "refunds issued");
                wait_for_enter();
            } break;
            case 8: { // show accounting
                clear_screen();
                printf("Total matches: %u\n", acc.total_matches);
                printf("Saved pot (for full house): %.2f\n", acc.saved_pot);
                list_players(roster, roster_count);
                wait_for_enter();
            } break;
            case 9: { // set normal cost
                clear_screen();
                double c; printf("New normal card cost: ");
                if (scanf("%lf", &c) == 1) { cfg_set_normal_card_cost(c); printf("Updated normal card cost to %.2f\n", cfg_get_normal_card_cost()); }
                wait_for_enter();
            } break;
            case 10: { // set full house cost
                clear_screen();
                double c; printf("New full house card cost: ");
                if (scanf("%lf", &c) == 1) { cfg_set_fullhouse_card_cost(c); printf("Updated full house card cost to %.2f\n", cfg_get_fullhouse_card_cost()); }
                wait_for_enter();
            } break;
            case 11: { // set saved pct
                clear_screen();
                double p; printf("New saved pot percentage (0-1): ");
                if (scanf("%lf", &p) == 1) { cfg_set_saved_pot_percentage(p); printf("Saved pot percentage now %.2f\n", cfg_get_saved_pot_percentage()); }
                wait_for_enter();
            } break;
            case 12: { // toggle multi winners
                clear_screen();
                int cur = cfg_get_allow_multi_winners();
                cfg_set_allow_multi_winners(!cur);
                printf("Multi winners now %s for normal matches.\n", cfg_get_allow_multi_winners() ? "ENABLED" : "DISABLED");
                wait_for_enter();
            } break;
            case 13: { // show config
                clear_screen();
                printf("Normal card cost: %.2f\n", cfg_get_normal_card_cost());
                printf("Full house card cost: %.2f\n", cfg_get_fullhouse_card_cost());
                printf("Saved pot percentage: %.2f\n", cfg_get_saved_pot_percentage());
                printf("Allow multi winners: %d\n", cfg_get_allow_multi_winners());
                printf("Max players: %u\n", cfg_get_max_players());
                wait_for_enter();
            } break;
            case 14: { // preview distribution
                clear_screen();
                if (!has_active_match) { printf("No active match.\n"); break; }
                double save_pct = (current_match.mode == GAME_FULL_HOUSE) ? 0.0 : cfg_get_saved_pot_percentage();
                double to_save = current_match.pot * save_pct;
                double distributable = current_match.pot - to_save;
                printf("Pot: %.2f SavePct: %.2f Saved: %.2f Distributable: %.2f WinnersSoFar:%u\n", current_match.pot, save_pct, to_save, distributable, current_match.winner_count);
                if (current_match.winner_count > 0 && current_match.mode != GAME_FULL_HOUSE) {
                    double per_winner = distributable / (double)current_match.winner_count;
                    printf("Per winner payout (if ended now): %.2f\n", per_winner);
                } else if (current_match.mode == GAME_FULL_HOUSE) {
                    printf("Full house uses accumulated saved pot: %.2f (not current match pot unless configured).\n", acc.saved_pot);
                }
                wait_for_enter();
            } break;
            case 15: { // recharge player balance
                clear_screen();
                uint32_t id; double amount;
                printf("Player ID to recharge: ");
                if (scanf("%u", &id) != 1) break;
                printf("Amount to add: ");
                if (scanf("%lf", &amount) != 1) break;
                if (amount <= 0.0) { printf("Amount must be positive.\n"); break; }
                Player* p = engine_find_player(roster, roster_count, id);
                if (!p) { printf("Player not found.\n"); break; }
                p->balance += amount;
                p->total_recharged += amount;
                printf("Added %.2f to %s. New balance: %.2f\n", amount, p->name, p->balance);
                // Persist immediately
                persist_save_roster("data/roster.bin", roster, roster_count);
                persist_save_accounting("data/accounting.bin", &acc);
                {
                    char details[128];
                    snprintf(details, sizeof(details), "recharge,id=%u,amount=%.2f", id, amount);
                    persist_append_transaction("data/transactions.csv", "recharge", details);
                }
                wait_for_enter();
            } break;
            case 16: { // export CSV
                clear_screen();
                if (persist_export_players_csv("data/players_summary.csv", roster, roster_count) == 0) {
                    printf("Exported players summary to data/players_summary.csv\n");
                } else {
                    printf("Failed to export CSV.\n");
                }
                persist_append_transaction("data/transactions.csv", "export_players", "players_summary.csv written");
                wait_for_enter();
            } break;
            case 17: { // manual checkpoint save
                clear_screen();
                persist_save_roster("data/roster.bin", roster, roster_count);
                persist_save_accounting("data/accounting.bin", &acc);
                printf("Checkpoint saved.\n");
                persist_append_transaction("data/transactions.csv", "checkpoint", "manual save");
                wait_for_enter();
            } break;
            case 0:
                running = 0; break;
            default:
                printf("Unknown option.\n");
        }
    }

    // Save on exit
    persist_save_roster("data/roster.bin", roster, roster_count);
    persist_save_accounting("data/accounting.bin", &acc);
    printf("Exiting.\n");
    return 0;
}
