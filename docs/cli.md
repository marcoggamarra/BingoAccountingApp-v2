# CLI Usage

Interactive menu (`main.c`) allows operating the engine manually.

## Menu Options

| # | Action |
|---|--------|
| 1 | List players |
| 2 | Add player |
| 3 | Remove player |
| 4 | Start match |
| 5 | Buy cards for player |
| 6 | Add winner |
| 7 | End match (payout + persist) |
| 8 | Show accounting & players |
| 9 | Set normal card cost |
| 10 | Set full house card cost |
| 11 | Set saved pot percentage |
| 12 | Toggle multi-winner normal matches |
| 13 | Show configuration summary |
| 14 | Preview current match distribution |
| 15 | Recharge player balance |
| 16 | Export players summary CSV |
| 0 | Exit / final save |

## Typical Session

1. Add players (2). Provide name and initial balance.
2. Start a Normal match (4). Choose mode 1, accept default cost or override.
3. Select participation or reuse previous config; cards deducted, pot grows.
4. Add winners (6).
5. End match (7) to distribute payouts and save.
6. Repeat Normal matches; saved pot accrues.
7. Start Full House (4, mode 2) and finish for grand payout.
8. Export financial summary (16) if needed.

## Reuse Participation
- When starting a match you may reuse last participation and card counts to accelerate repeated play.

## Preview Distribution
- Option 14 shows projected per-winner payout for normal OR total distributable for full house before ending.

## Data Safety
- Roster and accounting saved after ending matches and on exit.
- Exported CSV is overwritten each time option 16 is used.

## Error Handling
- Invalid IDs or insufficient balance produce messages and skip actions.
- Menu consumes basic numeric input; flushing stdin handled minimally (avoid stray characters).

## Extension Ideas
- Add option to reset saved pot.
- Add transaction log export.
- Add batch recharge utility.
