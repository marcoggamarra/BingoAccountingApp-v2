#include <stdio.h>
#include <string.h>
#include <time.h>
#include "persist.h"
#include "types.h"

#define ROSTER_MAGIC 0x42474F50 /* 'BGOP' */
#define ROSTER_VERSION 2

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t count;
} RosterHeader;

/* Legacy player (v1) layout used for backward load (no monetary tracking) */
typedef struct {
    uint32_t id;
    char name[64];
    double balance;
    uint32_t wins; uint32_t losses; uint32_t draws;
    uint32_t cards_owned; uint32_t lifetime_cards;
} PlayerLegacy;

int persist_save_roster(const char* path, Player* roster, uint32_t roster_count) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    RosterHeader hdr; hdr.magic = ROSTER_MAGIC; hdr.version = ROSTER_VERSION; hdr.reserved = 0; hdr.count = roster_count;
    fwrite(&hdr, sizeof(hdr), 1, f);
    for (uint32_t i = 0; i < roster_count; ++i) {
        Player* p = &roster[i];
        fwrite(&p->id, sizeof(p->id), 1, f);
        fwrite(p->name, sizeof(p->name), 1, f);
        fwrite(&p->balance, sizeof(p->balance), 1, f);
        fwrite(&p->record.wins, sizeof(p->record.wins), 1, f);
        fwrite(&p->record.losses, sizeof(p->record.losses), 1, f);
        fwrite(&p->record.draws, sizeof(p->record.draws), 1, f);
        fwrite(&p->cards_owned, sizeof(p->cards_owned), 1, f);
        fwrite(&p->lifetime_cards, sizeof(p->lifetime_cards), 1, f);
        fwrite(&p->total_recharged, sizeof(p->total_recharged), 1, f);
        fwrite(&p->total_spent, sizeof(p->total_spent), 1, f);
        fwrite(&p->total_won, sizeof(p->total_won), 1, f);
    }
    fclose(f);
    return 0;
}

int persist_load_roster(const char* path, Player* roster, uint32_t* roster_count, uint32_t max_players) {
    FILE* f = fopen(path, "rb");
    if (!f) return -1;
    RosterHeader hdr; size_t rh = fread(&hdr, sizeof(hdr), 1, f);
    if (rh == 1 && hdr.magic == ROSTER_MAGIC && hdr.version >= 2) {
        if (hdr.count > max_players) { fclose(f); return -2; }
        for (uint32_t i = 0; i < hdr.count; ++i) {
            Player* p = &roster[i]; memset(p, 0, sizeof(Player));
            fread(&p->id, sizeof(p->id), 1, f);
            fread(p->name, sizeof(p->name), 1, f);
            fread(&p->balance, sizeof(p->balance), 1, f);
            fread(&p->record.wins, sizeof(p->record.wins), 1, f);
            fread(&p->record.losses, sizeof(p->record.losses), 1, f);
            fread(&p->record.draws, sizeof(p->record.draws), 1, f);
            fread(&p->cards_owned, sizeof(p->cards_owned), 1, f);
            fread(&p->lifetime_cards, sizeof(p->lifetime_cards), 1, f);
            fread(&p->total_recharged, sizeof(p->total_recharged), 1, f);
            fread(&p->total_spent, sizeof(p->total_spent), 1, f);
            fread(&p->total_won, sizeof(p->total_won), 1, f);
        }
        *roster_count = hdr.count;
        fclose(f);
        return 0;
    }
    // Legacy fallback: rewind and read old format
    fseek(f, 0, SEEK_SET);
    uint32_t count = 0; fread(&count, sizeof(uint32_t), 1, f);
    if (count > max_players) { fclose(f); return -2; }
    for (uint32_t i = 0; i < count; ++i) {
        PlayerLegacy lp; size_t rd = fread(&lp, sizeof(lp), 1, f);
        if (rd != 1) { fclose(f); return -3; }
        Player* p = &roster[i]; memset(p, 0, sizeof(Player));
        p->id = lp.id; strncpy(p->name, lp.name, sizeof(p->name)); p->name[63] = '\0';
        p->balance = lp.balance; p->record.wins = lp.wins; p->record.losses = lp.losses; p->record.draws = lp.draws;
        p->cards_owned = lp.cards_owned; p->lifetime_cards = lp.lifetime_cards;
        p->total_recharged = 0.0; p->total_spent = 0.0; p->total_won = 0.0; // unknown for legacy
    }
    *roster_count = count;
    fclose(f);
    return 0;
}

int persist_save_accounting(const char* path, const Accounting* acc) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    fwrite(acc, sizeof(Accounting), 1, f);
    fclose(f);
    return 0;
}

int persist_load_accounting(const char* path, Accounting* acc) {
    FILE* f = fopen(path, "rb");
    if (!f) return -1;
    size_t rd = fread(acc, sizeof(Accounting), 1, f);
    fclose(f);
    return rd == 1 ? 0 : -2;
}

int persist_append_match(const char* path, const Match* m) {
    FILE* f = fopen(path, "ab");
    if (!f) return -1;
    // Write a simple CSV-like line: match_number,mode,card_cost,pot,saved_for_fullhouse,winner_count,winners...
    fprintf(f, "%u,%d,%.2f,%.2f,%.2f,%u", m->match_number, (int)m->mode, m->card_cost, m->pot, m->saved_for_fullhouse, m->winner_count);
    for (uint32_t i = 0; i < m->winner_count; ++i) fprintf(f, ",%u", m->winners[i]);
    fprintf(f, "\n");
    fclose(f);
    return 0;
}

int persist_export_players_csv(const char* path, Player* roster, uint32_t roster_count) {
    FILE* f = fopen(path, "w");
    if (!f) return -1;
    fprintf(f, "id,name,balance,total_recharged,total_spent,total_won,wins,losses,draws,lifetime_cards,net_gain\n");
    for (uint32_t i = 0; i < roster_count; ++i) {
        Player* p = &roster[i];
        double net = p->total_won - p->total_spent;
        fprintf(f, "%u,%s,%.2f,%.2f,%.2f,%.2f,%u,%u,%u,%u,%.2f\n", p->id, p->name, p->balance, p->total_recharged, p->total_spent, p->total_won, p->record.wins, p->record.losses, p->record.draws, p->lifetime_cards, net);
    }
    fclose(f);
    return 0;
}

int persist_append_transaction(const char* path, const char* type, const char* details) {
    FILE* f = fopen(path, "ab");
    if (!f) return -1;
    // naive timestamp using time(NULL)
    time_t t = time(NULL);
    fprintf(f, "%s,%lld,%s\n", type, (long long)t, details ? details : "");
    fclose(f);
    return 0;
}
