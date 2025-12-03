# Accounting Logic

This document details how money flows through the Bingo engine.

## Sources of Funds

- Player recharges (manual top-ups via CLI option 15).
- Card purchases (Normal or Full House matches).

## Sinks / Distributions

- Normal match payouts (distributable portion of pot).
- Full House match payouts (saved pot + full house pot).

## Normal Match Flow

1. Players buy cards: balances decrease, `Match.pot` increases.
2. Save portion: `saved_for_fullhouse = pot * saved_percentage`.
3. Distributable: `pot - saved_for_fullhouse`.
4. Payout: Distributable split evenly among winners (if any). No winners → regarded as loss for participants (recorded losses) OR potential draw if needed (currently just losses except winners). Saved portion accumulates in `Accounting.saved_pot`.

## Full House Match Flow

1. Players buy cards: balances decrease, `Match.pot` increases.
2. On match end: total distributable = `Accounting.saved_pot + Match.pot`.
3. Split evenly among winners.
4. Reset `Accounting.saved_pot` to 0 and `Match.pot` cleared.

## Player Financial Fields

| Field | Meaning |
|-------|---------|
| `balance` | Current available funds. |
| `total_recharged` | Sum of manual external currency injections. |
| `total_spent` | Cumulative card purchases cost. |
| `total_won` | Cumulative payouts received. |
| `record.wins/losses/draws` | Outcome counters. |

## Invariants (Ideal)

Let Σ(balance_i) + saved_pot + outstanding_match_pot (active only) = Σ(initial_balances + recharges) - Σ(spent) + Σ(won)
Given all payouts come from pots built by spending or saved pot accumulation, money is conserved except for external recharges.

## Edge Cases

- No winners: Distributable not paid; participants record losses; money stays in the saved portion? (Current logic: if winner_count==0, no payout and saved portion still reserved. This can grow funds; you may add a rule to mark draws or refund.)
- Multiple winners disabled: Only first winner rewarded; others ignored until enabled.
- Full House with zero winners: Currently no payout and saved pot remains; consider enforcing at least one winner before ending.

## Recommended Integrity Checks

Implement periodic audit:
1. Compute expected sum = Σ(total_recharged) - Σ(total_spent) + Σ(total_won).
2. Compare with Σ(balance_i). They should match unless a match is active or saved pot is pending payout. Add saved_pot and active match pot to reconcile.

## Future Enhancements

- House rake: percentage removed before payouts.
- Refund scenario for matches with no winners.
- Separate tracking for each match's per-player spend/win to enable detailed history.
