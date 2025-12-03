# Overview

This project provides a Bingo accounting engine in C. It does NOT draw numbers or evaluate card patterns. Instead, it:

- Tracks players, balances, and match participation.
- Records purchases (cards), winnings, losses, and saved pot accumulation.
- Distributes payouts for Normal and Full House matches.
- Persists roster and accounting state between runs and can export financial summaries.

## Concepts

| Concept | Description |
|---------|-------------|
| Player | Participant with an ID, balance, win/loss record, and financial tracking fields. |
| Match | A single game instance: mode (Normal or Full House), card cost, pot, winners. |
| Normal Match | Any non-full-house pattern (line, diagonal, corners) represented as `GAME_NORMAL`. Saves a percentage of pot to the Full House fund. |
| Full House Match | Final game payout: distributes saved pot + current match pot among winners. Does not save additional funds. |
| Saved Pot | Accumulated value from Normal matches reserved for the next Full House. |
| Accounting | Global counters: total matches played and saved pot value. |

## Mode Rules

- `GAME_NORMAL`: Pot collected from card purchases. A configurable percentage (`cfg_get_saved_pot_percentage`) is set aside for future Full House.
- `GAME_FULL_HOUSE`: Payout uses both the accumulated saved pot and the current match's pot.

## High-Level Flow

1. Start match → choose mode and card cost (override or default from config).
2. Select participating players and purchase cards → pot increases, balances decrease.
3. Input winners manually.
4. End match → apply payouts:
   - Normal: Split distributable pot among winners; save percentage.
   - Full House: Split (saved pot + current pot) among winners; reset saved pot.
5. Persist state automatically.

## Financial Tracking Fields

Each `Player` accumulates:

- `total_recharged`: Sum of manual top-ups.
- `total_spent`: Sum spent buying cards.
- `total_won`: Sum received in payouts.
- `balance`: Current total credits available.

Net gain = `total_won - total_spent` (ignoring recharged funds as external input).

## Integrity Considerations

- Purchases reduce player balance and increase match pot.
- Normal match payout excludes saved fraction; saved amount added to `Accounting.saved_pot`.
- Full House payout empties saved pot and its own pot.
- Binary persistence uses versioned header (`BGOP` magic, version 2). Legacy (v1) files load with monetary fields defaulting to zero.

See other documents for details:

- `configuration.md` – Tunable parameters.
- `persistence.md` – File formats.
- `cli.md` – Interactive commands.
- `api.md` – Public engine interfaces.
- `accounting.md` – Payout and invariants.
