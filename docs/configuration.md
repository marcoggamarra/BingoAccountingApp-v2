# Configuration

All tunables live in `config.c` / `config.h` and are accessed via getters/setters.

## Parameters

- Normal card cost: `cfg_get_normal_card_cost()` / `cfg_set_normal_card_cost(double)`
- Full House card cost: `cfg_get_fullhouse_card_cost()` / `cfg_set_fullhouse_card_cost(double)`
- Saved pot percentage (0..1): `cfg_get_saved_pot_percentage()` / `cfg_set_saved_pot_percentage(double)`
- Max players: `cfg_get_max_players()` / `cfg_set_max_players(uint32_t)`
- Allow multiple winners (Normal): `cfg_get_allow_multi_winners()` / `cfg_set_allow_multi_winners(int)`

## Defaults

| Setting | Default | Notes |
|---------|---------|-------|
| Normal card cost | 0.25 | Rule: base match card cost. |
| Full House card cost | 0.0 | Must be set before FH match if override desired. |
| Saved pot percentage | 0.15 | Portion of Normal pot reserved. |
| Max players | 512 | Upper bound of roster array. |
| Multi winners | 1 | Normal matches can have >1 winner. |

## Validation

Call `cfg_validate()` to confirm ranges before starting a session.

## Strategy Notes

- Higher saved percentage increases Full House payout but reduces regular match winner returns.
- Setting Full House card cost high creates its own pot in addition to saved pot.
- Disabling multi winners increases variance (only one winner receives distributable).
