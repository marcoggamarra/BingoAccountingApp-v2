#ifndef PERSIST_H
#define PERSIST_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

int persist_save_roster(const char* path, Player* roster, uint32_t roster_count);
int persist_load_roster(const char* path, Player* roster, uint32_t* roster_count, uint32_t max_players);

int persist_save_accounting(const char* path, const Accounting* acc);
int persist_load_accounting(const char* path, Accounting* acc);

// Simple match history append (CSV-like)
int persist_append_match(const char* path, const Match* m);

// Export roster financial summary to CSV
int persist_export_players_csv(const char* path, Player* roster, uint32_t roster_count);

// Append transactional log rows: type,timestamp,details
int persist_append_transaction(const char* path, const char* type, const char* details);

#ifdef __cplusplus
}
#endif

#endif // PERSIST_H
