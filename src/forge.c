#include <stdio.h>
#include <stdbool.h>
#include "hashtable.h"
#include "encoding.h"
#include "model.h"

typedef struct sortable_fragment_t {
    uint8_t length;
    uint8_t *body;
    uint32_t count;
} SortableFragment;

uint32_t hash_fragments(Fragment *fragments, size_t fragment_count);

int compute_value(SortableFragment *f) {
    return f->count * f->length;
}

int compare_fragment(const void* a, const void* b) {
    SortableFragment* fragmentA = (SortableFragment*) a;
    SortableFragment* fragmentB = (SortableFragment*) b;

    const uint32_t valueA = compute_value(fragmentA);
    const uint32_t valueB = compute_value(fragmentB);

    if (valueA > valueB) return -1;
    if (valueA < valueB) return 1;
    
    return 0;
}

int main() {
    HashTable *table;
    {
        const int initial_table_size = 128;
        table = hash_table_new(initial_table_size);
    }

    const size_t buffer_size = 4096;
    uint8_t buffer[buffer_size];

    HashKey tentativeKey;
    tentativeKey.length = 0;
    tentativeKey.body = alloca(buffer_size);

    size_t bytes_read;

    size_t maximum_fragment_length = 255;

    while((bytes_read = fread((void *)buffer, sizeof(uint8_t), buffer_size, stdin)) > 0)
    {
        for(int i = 0; i < bytes_read; i++)
        {
            tentativeKey.body[
                tentativeKey.length] =
                    buffer[i];
            tentativeKey.length++;
            const bool found_in_table = hash_table_contains(table, tentativeKey);
            const bool is_new = found_in_table == false;
            const bool overrun = tentativeKey.length > maximum_fragment_length;
            const bool doIncrement = overrun || (is_new && (tentativeKey.length - 1) > 0);
            const bool doReset = is_new || overrun;
            if (is_new)
            {
                hash_table_put(table, tentativeKey, 1);
            }
            if (doIncrement) {
                if (is_new) {
                    // the relevant counter will exclude the latest figure
                    tentativeKey.length--;
                }
                const uint32_t value = hash_table_get(table, tentativeKey);
                hash_table_put(table, tentativeKey, value + 1);
            }
            if(doReset) {
                tentativeKey.length = 0;
            }
        }
    }

    SortableFragment sortable_fragments[table->count];

    {
        size_t j = 0;
        for(size_t i = 0; i < table->size; i++)
        {
            HashEntry *entry = table->entries[i];
            while (entry != NULL) {
                HashKey key = entry->key;
                SortableFragment f;
                f.length = key.length;
                f.body = key.body;
                f.count = entry->value;
                sortable_fragments[j] = f;
                j++;
                entry = entry->next;
            }
        }
    }

    qsort(sortable_fragments, table->count, sizeof(SortableFragment), compare_fragment);
    
    Fragment fragments[table->count];
    size_t fragment_count = 0;

    for(int i = 0; i < table->count; i++)
    {
        SortableFragment sf = sortable_fragments[i];

        const size_t cost = encoding_get_byte_count(i);
        const size_t alternative_cost = 1 + sf.length;

        if (cost < alternative_cost)
        {
            Fragment f;

            f.length = sf.length;
            f.body = sf.body;

            fragments[
                fragment_count] =
                    f;
            fragment_count++;
        }
    }

    Model m;

    m.hash = hash_fragments((Fragment *)&fragments[0], fragment_count);
    m.fragment_count = fragment_count;
    m.fragments = fragments;

    model_write(&m, stdout);

    return 0;
}

uint32_t hash_fragments(Fragment *fragments, size_t fragmentCount)
{
    // djb2 by Dan Bernstein; http://www.cse.yorku.ca/~oz/hash.html
    uint32_t hash = 5381;

    for(size_t i = 0; i < fragmentCount; i++)
        for(size_t j = 0; j < fragments[i].length; j++)
            hash = ((hash << 5) + hash) + (uint32_t) fragments[i].body[j];

    return hash;
}
