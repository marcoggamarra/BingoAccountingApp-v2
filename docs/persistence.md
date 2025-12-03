# Persistence

The engine persists roster and accounting state in binary files plus a CSV for match history and optional player summary exports.

## Files

| File | Purpose | Format |
|------|---------|--------|
| `data/roster.bin` | Player roster | Binary (versioned) |
| `data/accounting.bin` | Saved pot + match count | Binary (raw struct) |
| `data/matches.csv` | Append-only match summary | CSV lines |
| `data/players_summary.csv` | On-demand export of financial metrics | CSV overwrite |

## Roster Binary Format (v2)
Header (16 bytes):
- `uint32_t magic` = `0x42474F50` ('BGOP')
- `uint16_t version` = 2
- `uint16_t reserved` = 0
- `uint32_t count` = number of players

Then for each player (field order):
1. `uint32_t id`
2. `char name[64]`
3. `double balance`
4. `uint32_t wins`
5. `uint32_t losses`
6. `uint32_t draws`
7. `uint32_t cards_owned`
8. `uint32_t lifetime_cards`
9. `double total_recharged`
10. `double total_spent`
11. `double total_won`

## Legacy Format (v1)
- First 4 bytes: count
- Then contiguous legacy structs (without monetary tracking). Monetary fields load as zero.

## Match History CSV Line Format
`match_number,mode,card_cost,pot,saved_for_fullhouse,winner_count,<winner_ids...>`

Example:
`12,1,0.25,10.00,1.50,2,5,7`

## Player Summary CSV
Columns: `id,name,balance,total_recharged,total_spent,total_won,wins,losses,draws,lifetime_cards,net_gain`
`net_gain = total_won - total_spent`

## Atomicity & Corruption
Current writes are direct and not atomic. Recommended future enhancement:
1. Write to `*.tmp` file.
2. Flush & close.
3. Rename over original.

## Versioning Strategy
- Increment `ROSTER_VERSION` when adding/removing fields.
- Maintain legacy loader paths for older versions.
- Consider adding checksum for integrity.

## Portability
Binary format assumes same endianness and alignment (x86_64 little-endian). For cross-platform portability, switch to explicit scalar write in fixed endianness or adopt a text (JSON/CSV) format.
