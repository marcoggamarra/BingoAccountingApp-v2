# API Reference

Public headers: `bingo.h`, `config.h`, `persist.h`, `types.h`.

## Types (`types.h`)
- `GameMode`: `GAME_NORMAL`, `GAME_FULL_HOUSE`
- `Record`: `{wins, losses, draws}`
- `Player`: see fields (id, name, balance, record, cards_owned, lifetime_cards, total_recharged, total_spent, total_won)
- `Match`: `{mode, card_cost, pot, saved_for_fullhouse, match_number, winners[64], winner_count, active}`
- `Accounting`: `{total_bank (currently unused placeholder), saved_pot, total_matches}`

## Engine (`bingo.h`)
- `void engine_init(Accounting* acc);`
  Initializes configuration defaults and zeroes accounting structure.
- `int engine_add_player(Player* roster, uint32_t* roster_count, const char* name, double initial_balance);`
  Adds new player, returns assigned ID or negative on failure.
- `int engine_remove_player(Player* roster, uint32_t* roster_count, uint32_t player_id);`
  Removes player by ID; compacts array.
- `Player* engine_find_player(Player* roster, uint32_t roster_count, uint32_t player_id);`
  Lookup helper.
- `void match_start(Match* m, GameMode mode, double card_cost);`
  Begins match; sets `active=1`; chooses default cost if zero.
- `void match_buy_cards(Match* m, Player* p, uint32_t count);`
  Deducts cost, increments pot and player spend tracking.
- `int match_add_winner(Match* m, uint32_t player_id);`
  Adds winner ID; enforces multi-winner rule for normal matches.
- `void match_end(Match* m, Accounting* acc, Player* roster, uint32_t roster_count);`
  Applies payouts (normal vs full house), resets per-match fields.
- `void apply_payouts_normal(Match* m, Player* roster, uint32_t roster_count);`
  Internal: distributes normal match pot (minus saved portion).
- `void apply_payouts_fullhouse(Accounting* acc, Match* m, Player* roster, uint32_t roster_count, uint32_t* winners, uint32_t winner_count);`
  Internal: splits (saved_pot + match pot) among winners.

## Configuration (`config.h`)
Getters/Setters for:
- Normal card cost
- Full house card cost
- Saved pot percentage
- Max players
- Allow multi winners
- `cfg_validate()` for sanity checks

## Persistence (`persist.h`)
- `persist_save_roster`, `persist_load_roster`
- `persist_save_accounting`, `persist_load_accounting`
- `persist_append_match`
- `persist_export_players_csv`

## Error Codes
- Negative returns for add/remove player and persistence indicate failure (e.g., max capacity, file issues).
- `match_add_winner` codes: `0` success, `-1` inactive, `-2` multi-winner disabled, `-3` winners array full.

## Extending
- Add new payout logic by creating a function and calling it from `match_end` based on mode or configuration flag.
- Add house rake: adjust pot before distribution.
- Include transaction logging: create `Transaction` struct and append to a file per event (purchase, recharge, payout).
