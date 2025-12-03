# Bingo Game Engine (C)

Lightweight Bingo accounting (not number drawing) engine in C. External systems can integrate via the data files or by linking against the library source. The engine focuses on: player balances, card purchases, pot accumulation, payouts (Normal vs Full House), and financial reporting.

## Feature Highlights

- Two game modes: Normal (`GAME_NORMAL`) and Full House (`GAME_FULL_HOUSE`).
- Configurable card costs and saved pot percentage for Normal matches.
- Multi-winner support (toggleable for Normal matches).
- Accumulated saved pot rolled into Full House payout along with its own pot.
- Player financial tracking: recharged, spent, won, net gain.
- Persistence: versioned binary roster + accounting, append-only match history, CSV exports.
- Interactive CLI for manual operation.

## Quick Start (Windows PowerShell)

```powershell
$SRC = "src"; $OUT = "bin"; if (!(Test-Path $OUT)) { New-Item -ItemType Directory -Path $OUT | Out-Null }
if (!(Test-Path "data")) { New-Item -ItemType Directory -Path "data" | Out-Null }

gcc "$SRC/main.c" "$SRC/bingo.c" "$SRC/config.c" "$SRC/persist.c" -I include -o "$OUT/bingo.exe"
./bin/bingo.exe
```

## Core Flow

1. Add players (option 2) or load persisted ones.
2. Start Normal match (4) → buy cards automatically via participation config.
3. Add winners (6) → end match (7) to distribute pot and save part for Full House.
4. Repeat Normal matches; saved pot grows.
5. Start Full House (mode 2) → end match to distribute saved pot + match pot.
6. Export financial summary (16) for auditing.

## Financial Integrity

- Normal payout: `(pot - saved_fraction)` split among winners.
- Saved fraction accumulates for eventual Full House.
- Full House payout: `saved_pot + full_house_pot` split among winners; saved pot resets.
- Player `balance` evolves: `initial + recharged - spent + won`.

## Documentation

| Topic | File |
|-------|------|
| Overview & concepts | `docs/overview.md` |
| Configuration & tunables | `docs/configuration.md` |
| Persistence formats | `docs/persistence.md` |
| CLI operations | `docs/cli.md` |
| API reference | `docs/api.md` |
| Accounting & invariants | `docs/accounting.md` |

## Data Files

- `data/roster.bin` (versioned binary, magic BGOP).
- `data/accounting.bin` (raw struct).
- `data/matches.csv` (append-only history).
- `data/players_summary.csv` (exported on demand).

## Extending

- Add transaction log for each purchase/recharge/payout.
- Implement atomic saves with temp file + rename.
- Add rake/fee logic in payout functions.
- Provide JSON export/import for cross-platform portability.

## License

Internal usage example; no explicit license header added. Add one if distributing.

## Build (Alt: MSVC)

```powershell
$SRC = "src"; $OUT = "bin"; if (!(Test-Path $OUT)) { New-Item -ItemType Directory -Path $OUT | Out-Null }
if (!(Test-Path "data")) { New-Item -ItemType Directory -Path "data" | Out-Null }

cl /Fe:"$OUT/bingo.exe" /I include "$SRC/main.c" "$SRC/bingo.c" "$SRC/config.c" "$SRC/persist.c"
```

---
See `docs/api.md` for function-level detail.
