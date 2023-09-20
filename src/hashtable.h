
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef struct hash_key_t {
    uint8_t length;
    uint8_t *body;
} HashKey;

typedef struct hash_entry_t {
    struct hash_key_t key;
    uint32_t value;
    struct hash_entry_t* next;
} HashEntry;

typedef struct hash_table_t {
    size_t size;
    size_t count;
    HashEntry** entries;
} HashTable;

HashTable* hash_table_new(size_t initialSize);
void hash_table_put(HashTable* table, HashKey key, uint32_t value);
uint32_t hash_table_get(HashTable* table, HashKey key);
bool hash_table_contains(HashTable* table, HashKey key);