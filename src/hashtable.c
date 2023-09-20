#include "hashtable.h"

#define LOAD_FACTOR 0.75

uint32_t hash(HashKey key)
{
    // djb2 by Dan Bernstein; http://www.cse.yorku.ca/~oz/hash.html
    uint32_t hash = 5381;

    for(int i = 0; i < key.length; i++)
        hash = ((hash << 5) + hash) + (uint32_t) key.body[i];

    return hash;
}

void resize_if_needed(HashTable* table) {
    if (table->count >= LOAD_FACTOR * table->size) {
        size_t newSize = 2 * table->size;
        size_t derp = 0;
        HashEntry** newEntries = (HashEntry**)calloc(newSize, sizeof(HashEntry*));
        for (size_t i = 0; i < table->size; i++) {
            HashEntry* entry = table->entries[i];
            while (entry != NULL) {
                uint32_t hashValue = hash(entry->key) % newSize;
                HashEntry *nextEntry = entry->next;
                entry->next = newEntries[hashValue];
                newEntries[hashValue] = entry;
                entry = nextEntry;
                derp++;
            }
        }
        free(table->entries);
        table->entries = newEntries;
        table->size = newSize;
    }
}

HashTable* hash_table_new(size_t initialSize) {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->size = initialSize;
    table->count = 0;
    table->entries = (HashEntry**)calloc(initialSize, sizeof(HashEntry*));
    return table;
}

HashKey keydup(HashKey alfa)
{
    HashKey bravo;
    bravo.length = alfa.length;
    bravo.body = (uint8_t *) malloc(alfa.length);
    memcpy(bravo.body, alfa.body, alfa.length);
    return bravo;
}

bool keycmp(HashKey alfa, HashKey bravo)
{
    if(alfa.length != bravo.length)
    {
        return false;
    }
    for(int i = 0; i < alfa.length; i++)
    {
        if (alfa.body[i] != bravo.body[i])
        {
            return false;
        }
    }
    return true;
}

void hash_table_put(HashTable* table, HashKey key, uint32_t value) {
    uint32_t hashValue = hash(key) % table->size;
    HashEntry* entry = table->entries[hashValue];
    while (entry != NULL) {
        if (keycmp(entry->key, key)) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
    entry = (HashEntry*)malloc(sizeof(HashEntry));
    entry->key = keydup(key);
    entry->value = value;
    entry->next = table->entries[hashValue];
    table->entries[hashValue] = entry;
    table->count++;
    resize_if_needed(table);
}

uint32_t hash_table_get(HashTable* table, HashKey key) {
    uint32_t hashValue = hash(key) % table->size;
    HashEntry* entry = table->entries[hashValue];
    while (entry != NULL) {
        if (keycmp(entry->key, key)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return 0;
}

bool hash_table_contains(HashTable* table, HashKey key) {
    uint32_t hashValue = hash(key) % table->size;
    HashEntry* entry = table->entries[hashValue];
    while (entry != NULL) {
        if (keycmp(entry->key, key)) {
            return true;
        }
        entry = entry->next;
    }
    return false;
}