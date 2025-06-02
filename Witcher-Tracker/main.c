#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h> // For ptrdiff_t

// --- Constants ---
#define MAX_LINE_LENGTH 1024        // Maximum length of a command line input
#define MAX_NAME_LENGTH 128         // Maximum length for item, monster, potion names
#define MAX_ITEMS 128               // Maximum number of distinct items in inventories, formulae, bestiary entries
#define MAX_RECIPE_INGREDIENTS 64   // Maximum ingredients per potion recipe or items per loot/trade
#define MAX_EFFECTIVE_ITEMS 64      // Maximum effective items (potions/signs) per bestiary entry

// --- Data Structures ---

// Represents a single item (ingredient, potion, trophy) with its quantity in the inventory.
typedef struct {
    char name[MAX_NAME_LENGTH];
    int quantity;
} InventoryItem;

// Represents Geralt's complete inventory, categorized into ingredients, potions, and trophies.
typedef struct {
    InventoryItem ingredients[MAX_ITEMS]; // Array of ingredients
    int ingredient_count;                 // Current number of distinct ingredients
    InventoryItem potions[MAX_ITEMS];     // Array of potions
    int potion_count;                     // Current number of distinct potions
    InventoryItem trophies[MAX_ITEMS];    // Array of trophies
    int trophy_count;                   // Current number of distinct trophies
} Inventory;

// Represents a single ingredient requirement (name and quantity) for a potion formula.
typedef struct {
    char ingredient_name[MAX_NAME_LENGTH];
    int quantity;
} IngredientRequirement;

// Represents the formula for a single potion, including its name and required ingredients.
typedef struct {
    char potion_name[MAX_NAME_LENGTH];
    IngredientRequirement requirements[MAX_RECIPE_INGREDIENTS]; // Array of required ingredients
    int requirement_count;                                    // Number of required ingredients
} PotionFormula;

// Represents the collection of all known potion formulae.
typedef struct {
    PotionFormula formulae[MAX_ITEMS]; // Array of known formulae
    int formula_count;                 // Current number of known formulae
} AlchemyBase;

// Enumeration to distinguish between effective potions and signs.
typedef enum {
    EFFECTIVE_POTION,
    EFFECTIVE_SIGN
} EffectivenessType;

// Represents an item (potion or sign) that is effective against a specific monster.
typedef struct {
    char name[MAX_NAME_LENGTH]; // Name of the effective potion or sign
    EffectivenessType type;     // Type (potion or sign)
} EffectiveItem;

// Represents an entry in the bestiary for a single monster, listing effective items.
typedef struct {
    char monster_name[MAX_NAME_LENGTH];
    EffectiveItem effective_items[MAX_EFFECTIVE_ITEMS]; // Array of effective items
    int effective_item_count;                         // Number of known effective items
} BestiaryEntry;

// Represents the collection of all known bestiary entries.
typedef struct {
    BestiaryEntry entries[MAX_ITEMS]; // Array of bestiary entries
    int entry_count;                  // Current number of entries
} Bestiary;

// Enumeration representing the different types of commands the program can process.
typedef enum {
    CMD_LOOT,
    CMD_TRADE,
    CMD_BREW,
    CMD_LEARN_EFFECTIVENESS,
    CMD_LEARN_FORMULA,
    CMD_ENCOUNTER,
    CMD_QUERY_TOTAL_SPECIFIC,
    CMD_QUERY_TOTAL_ALL,
    CMD_QUERY_EFFECTIVE_AGAINST,
    CMD_QUERY_WHAT_IS_IN,
    CMD_EXIT,
    CMD_INVALID, // Command syntax is incorrect or unrecognized
    CMD_EMPTY   // Input line was empty after trimming
} CommandType;

// Structure to hold the parsed command information.
// Uses a union to store data specific to each command type efficiently.
typedef struct {
    CommandType type; // The type of the parsed command
    union {
        // Data for LOOT command
        struct {
            InventoryItem items[MAX_RECIPE_INGREDIENTS];
            int item_count;
        } loot;
        // Data for TRADE command
        struct {
            InventoryItem trophies_to_give[MAX_RECIPE_INGREDIENTS];
            int give_count;
            InventoryItem ingredients_to_receive[MAX_RECIPE_INGREDIENTS];
            int receive_count;
        } trade;
        // Data for BREW command
        struct {
            char potion_name[MAX_NAME_LENGTH];
        } brew;
        // Data for LEARN EFFECTIVENESS command
        struct {
            char item_name[MAX_NAME_LENGTH];
            EffectivenessType item_type;
            char monster_name[MAX_NAME_LENGTH];
        } learn_effectiveness;
        // Data for LEARN FORMULA command
        struct {
            char potion_name[MAX_NAME_LENGTH];
            IngredientRequirement requirements[MAX_RECIPE_INGREDIENTS];
            int requirement_count;
        } learn_formula;
        // Data for ENCOUNTER command
        struct {
            char monster_name[MAX_NAME_LENGTH];
        } encounter;
        // Data for QUERY TOTAL SPECIFIC command
        struct {
            char category[20]; // "ingredient", "potion", or "trophy"
            char item_name[MAX_NAME_LENGTH];
        } query_total_specific;
        // Data for QUERY TOTAL ALL command
        struct {
            char category[20]; // "ingredient", "potion", or "trophy"
        } query_total_all;
        // Data for QUERY EFFECTIVE AGAINST command
        struct {
            char monster_name[MAX_NAME_LENGTH];
        } query_effective_against;
        // Data for QUERY WHAT IS IN command
        struct {
            char potion_name[MAX_NAME_LENGTH];
        } query_what_is_in;
        // No data needed for EXIT, INVALID, EMPTY
    } data;
} ParsedCommand;


// --- Function Declarations ---
// (Keep existing declarations)
ParsedCommand parse_command(char *line);
void init_inventory(Inventory *inv);
void init_alchemy_base(AlchemyBase *base);
void init_bestiary(Bestiary *bestiary);
void handle_loot(Inventory *inv, const ParsedCommand *cmd);
void handle_trade(Inventory *inv, const ParsedCommand *cmd);
void handle_brew(Inventory *inv, AlchemyBase *alchemy_base, const ParsedCommand *cmd);
void handle_learn_effectiveness(Bestiary *bestiary, const ParsedCommand *cmd);
void handle_learn_formula(AlchemyBase *alchemy_base, const ParsedCommand *cmd);
void handle_encounter(Inventory *inv, const Bestiary *bestiary, const ParsedCommand *cmd);
void handle_query_total_specific(const Inventory *inv, const ParsedCommand *cmd);
void handle_query_total_all(const Inventory *inv, const ParsedCommand *cmd);
void handle_query_effective_against(const Bestiary *bestiary, const ParsedCommand *cmd);
void handle_query_what_is_in(const AlchemyBase *alchemy_base, const ParsedCommand *cmd);
int get_ingredient_quantity(const Inventory *inv, const char *name);
int get_potion_quantity(const Inventory *inv, const char *name);
int get_trophy_quantity(const Inventory *inv, const char *name);
void add_ingredient(Inventory *inv, const char *name, int quantity);
void add_potion(Inventory *inv, const char *name, int quantity);
void add_trophy(Inventory *inv, const char *name, int quantity);
int use_ingredient(Inventory *inv, const char *name, int quantity);
int use_potion(Inventory *inv, const char *name, int quantity);
int use_trophy(Inventory *inv, const char *name, int quantity);
const PotionFormula* find_formula(const AlchemyBase *base, const char *potion_name);
int add_formula(AlchemyBase *base, const char *potion_name, const IngredientRequirement *reqs, int req_count);
void print_formula(const PotionFormula *formula);
const BestiaryEntry* find_bestiary_entry(const Bestiary *bestiary, const char *monster_name);
int add_effectiveness(Bestiary *bestiary, const char *monster_name, const char *item_name, EffectivenessType type);
void print_effectiveness(const BestiaryEntry *entry);
void print_all_ingredients(const Inventory *inv);
void print_all_potions(const Inventory *inv);
void print_all_trophies(const Inventory *inv);


// --- Helper Function Implementations ---

// Safely duplicates a string using malloc. Exits on failure.
char* safe_strdup(const char* s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strlen(s) + 1;
    char* new_s = malloc(len);
    if (new_s == NULL) {
        // Consider printing an error to stderr before exiting
        // fprintf(stderr, "Memory allocation failed in safe_strdup\n");
        exit(EXIT_FAILURE);
    }
    memcpy(new_s, s, len);
    return new_s;
}

// Removes leading and trailing whitespace from a string in-place.
void trim_whitespace(char *str) {
    if (str == NULL || *str == '\0') return; // Handle NULL or empty string
    char *start = str;
    // Find the first non-whitespace character
    while (isspace((unsigned char)*start)) start++;
    // If string is all whitespace, make it empty
    if (*start == '\0') {
        *str = '\0';
        return;
    }
    // Find the last non-whitespace character
    char *end = str + strlen(str) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    // Write new null terminator
    *(end + 1) = '\0';
    // Shift the trimmed string to the beginning if needed
    if (start != str) {
        memmove(str, start, (end - start) + 2); // +2 includes the null terminator and char itself
    }
}


// Finds an InventoryItem within an array by name. Returns pointer or NULL. (Static: Internal use)
static InventoryItem* find_item(InventoryItem items[], int count, const char *name) {
    for (int i = 0; i < count; ++i) {
        if (strcmp(items[i].name, name) == 0) {
            return &items[i];
        }
    }
    return NULL; // Not found
}

// Adds or updates the quantity of an item in an InventoryItem array. (Static: Internal use)
static void add_or_update_item(InventoryItem items[], int *count, const char *name, int quantity) {
    InventoryItem *item = find_item(items, *count, name);
    if (item) {
        // Item exists, update quantity
        item->quantity += quantity;
        // Ensure quantity doesn't go below zero if used for subtraction
        if (item->quantity < 0) item->quantity = 0;
    } else {
        // Item does not exist, add if quantity is positive and there's space
        if (quantity > 0 && *count < MAX_ITEMS) {
            strncpy(items[*count].name, name, MAX_NAME_LENGTH - 1);
            items[*count].name[MAX_NAME_LENGTH - 1] = '\0'; // Ensure null termination
            items[*count].quantity = quantity;
            (*count)++; // Increment the count of distinct items
        }
    }
}

// Gets the quantity of an item from an InventoryItem array by name. Returns 0 if not found. (Static: Internal use)
static int get_item_quantity(const InventoryItem items[], int count, const char *name) {
    // Using const_cast temporarily to reuse find_item, alternative is separate find_item_const
    const InventoryItem *item = find_item((InventoryItem*)items, count, name); // Safe as find_item doesn't modify if item exists
    return item ? item->quantity : 0;
}


// Attempts to use (subtract) a quantity of an item from an InventoryItem array.
// Returns 1 on success, 0 if not found or insufficient quantity. (Static: Internal use)
static int use_item(InventoryItem items[], int count, const char *name, int quantity) {
    if (quantity <= 0) return 0; // Cannot use non-positive quantity
    InventoryItem *item = find_item(items, count, name);
    if (item && item->quantity >= quantity) {
        item->quantity -= quantity;
        // Optional: Could implement logic here to remove the item if quantity becomes 0
        // For this project, keeping items with 0 quantity is fine and simpler.
        return 1; // Success
    }
    return 0; // Failure (not found or not enough)
}

// Comparison function for qsort to sort InventoryItems alphabetically by name.
int compare_inventory_items(const void *a, const void *b) {
    const InventoryItem *itemA = (const InventoryItem *)a;
    const InventoryItem *itemB = (const InventoryItem *)b;
    return strcmp(itemA->name, itemB->name);
}

// Prints all items from an InventoryItem array with quantity > 0, sorted alphabetically. (Static: Internal use)
static void print_all_items(const InventoryItem items[], int count, const char *none_message) {
    int active_count = 0;
    for(int i = 0; i < count; ++i) {
        if(items[i].quantity > 0) {
            active_count++;
        }
    }

    if (active_count == 0) {
        printf("%s\n", none_message);
        return;
    }

    InventoryItem *sorted_items = malloc(active_count * sizeof(InventoryItem));
    if (!sorted_items) {
         // Fallback if malloc fails (should be rare for small counts)
         // This fallback prints unsorted, to avoid crashing.
         int printed_count = 0;
         for (int i = 0; i < count; ++i) {
             if (items[i].quantity > 0) {
                printf("%s%d %s", (printed_count > 0 ? ", " : ""), items[i].quantity, items[i].name);
                printed_count++;
             }
         }
         if (printed_count > 0) printf("\n");
         else printf("%s\n", none_message); // Should not reach here if active_count > 0
        return;
    }

    int current_idx = 0;
    for(int i = 0; i < count; ++i) {
        if(items[i].quantity > 0) {
            memcpy(&sorted_items[current_idx], &items[i], sizeof(InventoryItem));
            current_idx++;
        }
    }

    qsort(sorted_items, active_count, sizeof(InventoryItem), compare_inventory_items);

    for (int i = 0; i < active_count; ++i) {
        printf("%s%d %s", (i > 0 ? ", " : ""), sorted_items[i].quantity, sorted_items[i].name);
    }
    printf("\n");

    free(sorted_items);
}


// --- Inventory Management Functions ---

void init_inventory(Inventory *inv) {
    inv->ingredient_count = 0;
    inv->potion_count = 0;
    inv->trophy_count = 0;
}

void add_ingredient(Inventory *inv, const char *name, int quantity) {
    add_or_update_item(inv->ingredients, &inv->ingredient_count, name, quantity);
}

void add_potion(Inventory *inv, const char *name, int quantity) {
    add_or_update_item(inv->potions, &inv->potion_count, name, quantity);
}

void add_trophy(Inventory *inv, const char *name, int quantity) {
    add_or_update_item(inv->trophies, &inv->trophy_count, name, quantity);
}

int get_ingredient_quantity(const Inventory *inv, const char *name) {
    return get_item_quantity(inv->ingredients, inv->ingredient_count, name);
}

int get_potion_quantity(const Inventory *inv, const char *name) {
    return get_item_quantity(inv->potions, inv->potion_count, name);
}

int get_trophy_quantity(const Inventory *inv, const char *name) {
    return get_item_quantity(inv->trophies, inv->trophy_count, name);
}

int use_ingredient(Inventory *inv, const char *name, int quantity) {
    return use_item(inv->ingredients, inv->ingredient_count, name, quantity);
}

int use_potion(Inventory *inv, const char *name, int quantity) {
    return use_item(inv->potions, inv->potion_count, name, quantity);
}

int use_trophy(Inventory *inv, const char *name, int quantity) {
    return use_item(inv->trophies, inv->trophy_count, name, quantity);
}

void print_all_ingredients(const Inventory *inv) {
    print_all_items(inv->ingredients, inv->ingredient_count, "None");
}

void print_all_potions(const Inventory *inv) {
    print_all_items(inv->potions, inv->potion_count, "None");
}

void print_all_trophies(const Inventory *inv) {
    print_all_items(inv->trophies, inv->trophy_count, "None");
}


// --- Alchemy Base Functions ---

void init_alchemy_base(AlchemyBase *base) {
    base->formula_count = 0;
}

const PotionFormula* find_formula(const AlchemyBase *base, const char *potion_name) {
    for (int i = 0; i < base->formula_count; ++i) {
        if (strcmp(base->formulae[i].potion_name, potion_name) == 0) {
            return &base->formulae[i];
        }
    }
    return NULL;
}

int add_formula(AlchemyBase *base, const char *potion_name, const IngredientRequirement *reqs, int req_count) {
    if (find_formula(base, potion_name) != NULL) {
        return 0; // Already known
    }
    if (base->formula_count >= MAX_ITEMS) {
        // fprintf(stderr, "Error: Cannot add formula '%s', alchemy base full.\n", potion_name);
        return 0;
    }
    if (req_count > MAX_RECIPE_INGREDIENTS) {
         // fprintf(stderr, "Warning: Formula '%s' has too many ingredients, truncating to %d.\n", potion_name, MAX_RECIPE_INGREDIENTS);
         req_count = MAX_RECIPE_INGREDIENTS;
    }
     if (req_count <= 0) {
        // fprintf(stderr, "Error: Cannot add formula '%s' with no ingredients.\n", potion_name);
        return 0;
    }

    PotionFormula *new_formula = &base->formulae[base->formula_count];
    strncpy(new_formula->potion_name, potion_name, MAX_NAME_LENGTH - 1);
    new_formula->potion_name[MAX_NAME_LENGTH - 1] = '\0';

    memcpy(new_formula->requirements, reqs, req_count * sizeof(IngredientRequirement));
    new_formula->requirement_count = req_count;

    base->formula_count++;
    return 1;
}

int compare_ingredient_requirements(const void *a, const void *b) {
    const IngredientRequirement *reqA = (const IngredientRequirement *)a;
    const IngredientRequirement *reqB = (const IngredientRequirement *)b;
    if (reqA->quantity > reqB->quantity) return -1;
    if (reqA->quantity < reqB->quantity) return 1;
    return strcmp(reqA->ingredient_name, reqB->ingredient_name);
}

void print_formula(const PotionFormula *formula) {
    if (!formula || formula->requirement_count == 0) {
        // This case should be handled by the caller usually (e.g. "No formula for...")
        return;
    }
    IngredientRequirement *sorted_reqs = malloc(formula->requirement_count * sizeof(IngredientRequirement));
    if (!sorted_reqs) {
        // Fallback if malloc fails
        for (int i = 0; i < formula->requirement_count; ++i) {
             printf("%s%d %s", (i > 0 ? ", " : ""), formula->requirements[i].quantity, formula->requirements[i].ingredient_name);
        }
        printf("\n");
        return;
    }
    memcpy(sorted_reqs, formula->requirements, formula->requirement_count * sizeof(IngredientRequirement));
    qsort(sorted_reqs, formula->requirement_count, sizeof(IngredientRequirement), compare_ingredient_requirements);
    for (int i = 0; i < formula->requirement_count; ++i) {
        printf("%s%d %s", (i > 0 ? ", " : ""), sorted_reqs[i].quantity, sorted_reqs[i].ingredient_name);
    }
    printf("\n");
    free(sorted_reqs);
}


// --- Bestiary Functions ---

void init_bestiary(Bestiary *bestiary) {
    bestiary->entry_count = 0;
}

const BestiaryEntry* find_bestiary_entry(const Bestiary *bestiary, const char *monster_name) {
    for (int i = 0; i < bestiary->entry_count; ++i) {
        if (strcmp(bestiary->entries[i].monster_name, monster_name) == 0) {
            return &bestiary->entries[i];
        }
    }
    return NULL;
}

static BestiaryEntry* find_bestiary_entry_mutable(Bestiary *bestiary, const char *monster_name) {
    for (int i = 0; i < bestiary->entry_count; ++i) {
        if (strcmp(bestiary->entries[i].monster_name, monster_name) == 0) {
            return &bestiary->entries[i];
        }
    }
    return NULL;
}

static int is_effectiveness_known(const BestiaryEntry *entry, const char *item_name) {
    if (!entry) return 0;
    for (int i = 0; i < entry->effective_item_count; ++i) {
        if (strcmp(entry->effective_items[i].name, item_name) == 0) {
            return 1;
        }
    }
    return 0;
}

int add_effectiveness(Bestiary *bestiary, const char *monster_name, const char *item_name, EffectivenessType type) {
    BestiaryEntry *entry = find_bestiary_entry_mutable(bestiary, monster_name);
    int return_code = -1;

    if (entry) {
        if (is_effectiveness_known(entry, item_name)) {
            return 0; // Already known
        }
        if (entry->effective_item_count < MAX_EFFECTIVE_ITEMS) {
            EffectiveItem *newItem = &entry->effective_items[entry->effective_item_count];
            strncpy(newItem->name, item_name, MAX_NAME_LENGTH - 1);
            newItem->name[MAX_NAME_LENGTH - 1] = '\0';
            newItem->type = type;
            entry->effective_item_count++;
            return_code = 1; // Existing entry updated
        } else {
             // fprintf(stderr, "Error: Cannot add effectiveness for '%s', entry for '%s' full.\n", item_name, monster_name);
             return_code = -1;
        }
    } else {
        if (bestiary->entry_count < MAX_ITEMS) {
            entry = &bestiary->entries[bestiary->entry_count];
            strncpy(entry->monster_name, monster_name, MAX_NAME_LENGTH - 1);
            entry->monster_name[MAX_NAME_LENGTH - 1] = '\0';
            entry->effective_item_count = 0; // Initialize for new entry

            EffectiveItem *newItem = &entry->effective_items[0]; // Add to the first slot
            strncpy(newItem->name, item_name, MAX_NAME_LENGTH - 1);
            newItem->name[MAX_NAME_LENGTH - 1] = '\0';
            newItem->type = type;
            entry->effective_item_count = 1; // Now has one effective item

            bestiary->entry_count++;
            return_code = 2; // New entry added
        } else {
            // fprintf(stderr, "Error: Cannot add new bestiary entry for '%s', bestiary full.\n", monster_name);
            return_code = -1;
        }
    }
    return return_code;
}

// int is_effective(const Bestiary *bestiary, const char *monster_name, const char *item_name) {
//     const BestiaryEntry *entry = find_bestiary_entry(bestiary, monster_name);
//     return is_effectiveness_known(entry, item_name);
// }

int compare_effective_items(const void *a, const void *b) {
    const EffectiveItem *itemA = (const EffectiveItem *)a;
    const EffectiveItem *itemB = (const EffectiveItem *)b;
    return strcmp(itemA->name, itemB->name);
}

void print_effectiveness(const BestiaryEntry *entry) {
    if (!entry || entry->effective_item_count == 0) {
        // This case should be handled by the caller usually (e.g. "No knowledge of...")
        return;
    }
    EffectiveItem *sorted_items = malloc(entry->effective_item_count * sizeof(EffectiveItem));
    if (!sorted_items) {
        // Fallback if malloc fails
        for (int i = 0; i < entry->effective_item_count; ++i) {
            printf("%s%s", (i > 0 ? ", " : ""), entry->effective_items[i].name);
        }
        printf("\n");
        return;
    }
    memcpy(sorted_items, entry->effective_items, entry->effective_item_count * sizeof(EffectiveItem));
    qsort(sorted_items, entry->effective_item_count, sizeof(EffectiveItem), compare_effective_items);
    for (int i = 0; i < entry->effective_item_count; ++i) {
        printf("%s%s", (i > 0 ? ", " : ""), sorted_items[i].name);
    }
    printf("\n");
    free(sorted_items);
}


// --- Command Parsing Helper Functions ---

static int parse_quantity(const char *token, int *quantity) {
    if (!token || *token == '\0') return 0;
    char *endptr;
    long val = strtol(token, &endptr, 10);
    if (endptr == token || *endptr != '\0' || val <= 0 || val > 2147483647 ) { // Quantities must be positive
        return 0;
    }
    *quantity = (int)val;
    return 1;
}

static int parse_name(const char *token, char *dest, size_t max_len, int allow_spaces) {
    if (!token || *token == '\0') return 0; // Empty token is invalid

    char temp_token_for_validation[MAX_NAME_LENGTH]; // Use MAX_NAME_LENGTH as a safe buffer
    strncpy(temp_token_for_validation, token, sizeof(temp_token_for_validation) - 1);
    temp_token_for_validation[sizeof(temp_token_for_validation) - 1] = '\0';
    // Original `token` is const, so operate on a copy for validation checks that might alter it (like a temp trim for validation)
    // The `trim_whitespace` should have been applied by the caller to `token` if it's a segment from a larger string.
    // This `parse_name` now expects `token` to be the candidate name, already isolated.

    size_t len = strlen(temp_token_for_validation);
    if (len == 0 || len >= max_len) return 0;

    // Explicitly check for leading/trailing spaces on the candidate name `token`
    if (isspace((unsigned char)temp_token_for_validation[0]) || isspace((unsigned char)temp_token_for_validation[len-1])) {
        return 0;
    }

    int char_found = 0;
    int last_char_was_space = 0; // Used to detect multiple spaces

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = temp_token_for_validation[i];
        if (isalpha(c)) {
            char_found = 1;
            last_char_was_space = 0;
        } else if (isspace(c)) {
            if (!allow_spaces) return 0; // Spaces not allowed at all
            if (last_char_was_space) return 0; // Multiple spaces in a row
            last_char_was_space = 1;
        } else {
            return 0; // Non-alpha, non-space character
        }
    }

    if (!char_found) return 0; // Must contain at least one alpha character

    // If last_char_was_space is true here, it means it ended with a space, which we've already disallowed by initial check.
    // So, no need for `if (allow_spaces && last_char_was_space) return 0;` again.

    strncpy(dest, temp_token_for_validation, max_len - 1);
    dest[max_len - 1] = '\0';
    return 1;
}


// Parses a "potion name" which can contain spaces. Does NOT modify start_ptr.
// Sets end_ptr_in_original_string to where parsing stopped in start_ptr.
// Used primarily for "Geralt brews Potion Name"
static int parse_potion_name(const char *start_ptr, char *dest, size_t max_len, const char **end_ptr_in_original_string) {
    if (!start_ptr || !dest || !end_ptr_in_original_string) return 0;

    const char *name_scan_start = start_ptr;
    while (*name_scan_start && isspace((unsigned char)*name_scan_start)) {
        name_scan_start++;
    }
    if (*name_scan_start == '\0') return 0; // Empty after skipping spaces

    // For brew, name goes to end of string or '?'
    const char *boundary = name_scan_start + strlen(name_scan_start); // Default to end of string

    // These keywords are not strictly terminators for "brew" but if they appear,
    // it's part of a malformed brew command. parse_name will later validate the extracted string.
    // The main terminator for brew is end-of-string or '?'.
    const char *eff_keyword_terminator = "potion is effective against";
    const char *cons_keyword_terminator = "potion consists of";

    const char *eff_loc = strstr(name_scan_start, eff_keyword_terminator);
     if (eff_loc) {
        // If "potion is effective against" is found within the brew command's name part
        if (eff_loc > name_scan_start && isspace((unsigned char)*(eff_loc-1))) {
             if (eff_loc < boundary) boundary = eff_loc;
        } else if (eff_loc == name_scan_start) {
             if (eff_loc < boundary) boundary = eff_loc;
        }
    }

    const char *cons_loc = strstr(name_scan_start, cons_keyword_terminator);
    if (cons_loc) {
        if (cons_loc > name_scan_start && isspace((unsigned char)*(cons_loc-1))) {
            if (cons_loc < boundary) boundary = cons_loc;
        } else if (cons_loc == name_scan_start) {
             if (cons_loc < boundary) boundary = cons_loc;
        }
    }

    const char *q_mark_loc = strchr(name_scan_start, '?');
    if (q_mark_loc && q_mark_loc < boundary) boundary = q_mark_loc;

    ptrdiff_t name_len_to_copy = boundary - name_scan_start;
    if (name_len_to_copy <= 0) return 0;

    char temp_name_buffer[MAX_LINE_LENGTH];
    if ((size_t)name_len_to_copy >= sizeof(temp_name_buffer)) {
        name_len_to_copy = sizeof(temp_name_buffer) - 1;
    }
    strncpy(temp_name_buffer, name_scan_start, name_len_to_copy);
    temp_name_buffer[name_len_to_copy] = '\0';

    trim_whitespace(temp_name_buffer); // Trim the extracted potential name

    if (strlen(temp_name_buffer) == 0) return 0; // Name became empty after trim

    if (!parse_name(temp_name_buffer, dest, max_len, 1)) { // allow_spaces = 1 for potions
        return 0;
    }

    *end_ptr_in_original_string = boundary; // Where parsing stopped in the original string segment
    return 1;
}


static int parse_ingredient_list(char *tokens, InventoryItem *items, int *item_count, int max_items) {
    *item_count = 0;
    if (!tokens) return 0;

    char *saveptr;
    char *token = strtok_r(tokens, ",", &saveptr);

    while (token != NULL) {
        if (*item_count >= max_items) {
             // fprintf(stderr, "Warning: Max ingredients reached in list.\n");
             return 0; // Too many items
        }
        trim_whitespace(token);
        if (*token == '\0') return 0; // Empty token after trim

        char *space_ptr = strchr(token, ' ');
        if (!space_ptr || space_ptr == token || *(space_ptr + 1) == '\0') {
             return 0; // Malformed "qty name" pair
        }
        *space_ptr = '\0'; // Separate qty and name

        char *qty_str = token;
        char *name_str = space_ptr + 1;
        trim_whitespace(name_str); // Trim the extracted name string specifically

        int quantity;
        // Ingredients don't allow spaces in their names
        if (!parse_quantity(qty_str, &quantity) || !parse_name(name_str, items[*item_count].name, MAX_NAME_LENGTH, 0)) {
            return 0;
        }

        items[*item_count].quantity = quantity;
        (*item_count)++;

        token = strtok_r(NULL, ",", &saveptr);
    }
    return (*item_count > 0); // Must have at least one valid item
}

static int parse_formula_ingredient_list(char *tokens, IngredientRequirement *reqs, int *req_count, int max_reqs) {
    *req_count = 0;
     if (!tokens) return 0;

    char *saveptr;
    char *token = strtok_r(tokens, ",", &saveptr);

    while (token != NULL) {
        if (*req_count >= max_reqs) {
            // fprintf(stderr, "Warning: Max formula ingredients reached.\n");
            return 0;
        }
        trim_whitespace(token);
        if (*token == '\0') return 0;

        char *space_ptr = strchr(token, ' ');
        if (!space_ptr || space_ptr == token || *(space_ptr + 1) == '\0') return 0;
        *space_ptr = '\0';

        char *qty_str = token;
        char *name_str = space_ptr + 1;
        trim_whitespace(name_str);

        int quantity;
        if (!parse_quantity(qty_str, &quantity) || !parse_name(name_str, reqs[*req_count].ingredient_name, MAX_NAME_LENGTH, 0)) {
            return 0;
        }

        reqs[*req_count].quantity = quantity;
        (*req_count)++;

        token = strtok_r(NULL, ",", &saveptr);
    }
    return (*req_count > 0);
}

// For trade command, items given (trophies) are single-word names
static int parse_single_word_item_list(char *tokens, InventoryItem *items, int *item_count, int max_items) {
    *item_count = 0;
    if (!tokens) return 0;

    char *saveptr;
    char *token = strtok_r(tokens, ",", &saveptr);

    while (token != NULL) {
        if (*item_count >= max_items) {
            // fprintf(stderr, "Warning: Max single-word items reached.\n");
            return 0;
        }
        trim_whitespace(token);
        if (*token == '\0') return 0;

        char *space_ptr = strchr(token, ' ');
        if (!space_ptr || space_ptr == token || *(space_ptr + 1) == '\0') {
            return 0;
        }
        *space_ptr = '\0';

        char *qty_str = token;
        char *name_str = space_ptr + 1;
        trim_whitespace(name_str);

        int quantity;
        if (!parse_quantity(qty_str, &quantity) || !parse_name(name_str, items[*item_count].name, MAX_NAME_LENGTH, 0)) { // allow_spaces = 0
            return 0;
        }

        items[*item_count].quantity = quantity;
        (*item_count)++;

        token = strtok_r(NULL, ",", &saveptr);
    }
    return (*item_count > 0);
}


// Skips leading whitespace for a const char*
static const char* skip_whitespace_const(const char* p) {
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    return p;
}

// Tries to match 'keyword' at the beginning of '*cursor'.
// If matched as a whole word (i.e., followed by space or EOS),
// advances '*cursor' past the keyword and any subsequent whitespace.
// Returns 1 on match, 0 otherwise. 'keyword' itself should not have leading/trailing spaces.
static int match_and_advance(const char **cursor, const char *keyword) {
    const char *p_temp = skip_whitespace_const(*cursor); // Work on a temp pointer for matching
    size_t kw_len = strlen(keyword);

    if (strncmp(p_temp, keyword, kw_len) == 0) {
        char char_after_kw = p_temp[kw_len];
        if (char_after_kw == '\0' || isspace((unsigned char)char_after_kw)) {
            *cursor = p_temp + kw_len; // Advance past the keyword
            *cursor = skip_whitespace_const(*cursor); // Skip whitespace *after* the keyword
            return 1; // Matched
        }
    }
    // If no match, *cursor is not changed by this function.
    return 0; // No match
}


// Finds a 'needle' substring within 'haystack'.
// Returns a pointer to the start of 'needle' in 'haystack' if 'needle' is found
// as a "standalone" word/phrase (i.e., bounded by whitespace or start/end of haystack).
// Otherwise, returns NULL.
static const char* find_standalone_substring(const char *haystack, const char *needle) {
    const char *found_at = haystack;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return NULL;

    while ((found_at = strstr(found_at, needle)) != NULL) {
        int is_standalone = 1;
        // Check character before needle
        if (found_at > haystack && !isspace((unsigned char)*(found_at - 1))) {
            is_standalone = 0;
        }
        // Check character after needle
        char char_after_needle = *(found_at + needle_len);
        if (char_after_needle != '\0' && !isspace((unsigned char)char_after_needle)) {
            is_standalone = 0;
        }

        if (is_standalone) {
            return found_at; // Found a standalone match
        }
        found_at++; // Advance search past the current non-standalone match
    }
    return NULL; // Not found as a standalone substring
}

// Helper function to find a sequence of keywords allowing variable whitespace
// Returns pointer to the start of kw1 if sequence found, NULL otherwise.
// Sets end_of_sequence_ptr to point after the last keyword of the matched sequence (and its trailing space).
static const char* find_keyword_sequence(const char *text, const char *kw1, const char *kw2, const char *kw3, const char *kw4, const char **end_of_sequence_ptr) {
    const char *p = text;
    const char *found_kw1_start = NULL;

    while ((found_kw1_start = find_standalone_substring(p, kw1)) != NULL) {
        const char *current_pos_after_kw = skip_whitespace_const(found_kw1_start + strlen(kw1));

        if (kw2 == NULL) { // Sequence is just kw1
            if (end_of_sequence_ptr) *end_of_sequence_ptr = current_pos_after_kw;
            return found_kw1_start;
        }

        // Check for kw2
        if (strncmp(current_pos_after_kw, kw2, strlen(kw2)) == 0) {
            char char_after_kw2_val = current_pos_after_kw[strlen(kw2)];
            if (char_after_kw2_val == '\0' || isspace((unsigned char)char_after_kw2_val)) {
                current_pos_after_kw = skip_whitespace_const(current_pos_after_kw + strlen(kw2));

                if (kw3 == NULL) { // Sequence is kw1, kw2
                    if (end_of_sequence_ptr) *end_of_sequence_ptr = current_pos_after_kw;
                    return found_kw1_start;
                }

                // Check for kw3
                if (strncmp(current_pos_after_kw, kw3, strlen(kw3)) == 0) {
                    char char_after_kw3_val = current_pos_after_kw[strlen(kw3)];
                    if (char_after_kw3_val == '\0' || isspace((unsigned char)char_after_kw3_val)) {
                        current_pos_after_kw = skip_whitespace_const(current_pos_after_kw + strlen(kw3));

                        if (kw4 == NULL) { // Sequence is kw1, kw2, kw3
                            if (end_of_sequence_ptr) *end_of_sequence_ptr = current_pos_after_kw;
                            return found_kw1_start;
                        }

                        // Check for kw4
                        if (strncmp(current_pos_after_kw, kw4, strlen(kw4)) == 0) {
                             char char_after_kw4_val = current_pos_after_kw[strlen(kw4)];
                             if (char_after_kw4_val == '\0' || isspace((unsigned char)char_after_kw4_val)) {
                                current_pos_after_kw = skip_whitespace_const(current_pos_after_kw + strlen(kw4));
                                if (end_of_sequence_ptr) *end_of_sequence_ptr = current_pos_after_kw;
                                return found_kw1_start;
                             }
                        }
                    }
                }
            }
        }
        p = found_kw1_start + 1; // Advance search in original text past current found_kw1_start
    }
    return NULL; // Sequence not found
}


// --- Main Command Parser ---
ParsedCommand parse_command(char *original_line_ptr) {
    ParsedCommand result;
    result.type = CMD_INVALID;

    char line_copy[MAX_LINE_LENGTH];
    strncpy(line_copy, original_line_ptr, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';
    trim_whitespace(line_copy);

    if (strlen(line_copy) == 0) {
        result.type = CMD_EMPTY;
        return result;
    }

    const char *p = line_copy;

    if (strcmp(p, "Exit") == 0) {
        result.type = CMD_EXIT;
        return result;
    }

    const char *original_p_for_geralt_cmds = p;
    if (match_and_advance(&p, "Geralt")) {
        const char *p_after_geralt = p;

        if (match_and_advance(&p, "loots")) {
            char list_part_for_strtok[MAX_LINE_LENGTH];
            strncpy(list_part_for_strtok, p, sizeof(list_part_for_strtok) - 1);
            list_part_for_strtok[sizeof(list_part_for_strtok) - 1] = '\0';
            if (parse_ingredient_list(list_part_for_strtok, result.data.loot.items, &result.data.loot.item_count, MAX_RECIPE_INGREDIENTS)) {
                result.type = CMD_LOOT;
            }
            return result; // Always return after attempting a Geralt command type
        }
        p = p_after_geralt;

        if (match_and_advance(&p, "trades")) {
            const char *trade_content_start = p;
            const char *trophy_keyword_loc = find_standalone_substring(trade_content_start, "trophy");

            if (trophy_keyword_loc) {
                const char *items_before_trophy_test = skip_whitespace_const(trade_content_start);
                if (items_before_trophy_test == trophy_keyword_loc || items_before_trophy_test >= trophy_keyword_loc ) { // Nothing but whitespace before "trophy", or empty
                     return result; // Invalid, items to give list is empty or malformed
                }

                const char *for_keyword_loc = find_standalone_substring(trophy_keyword_loc + strlen("trophy"), "for");

                if (for_keyword_loc && (for_keyword_loc > trophy_keyword_loc)) {
                    char trophies_str_buffer[MAX_LINE_LENGTH];
                    ptrdiff_t trophies_len = trophy_keyword_loc - trade_content_start;
                    // trophies_len must be > 0 for a valid list of items to give
                    if (trophies_len <=0) return result; // Empty trophy list part
                    strncpy(trophies_str_buffer, trade_content_start, trophies_len);
                    trophies_str_buffer[trophies_len] = '\0';

                    const char *ingredients_segment_start = skip_whitespace_const(for_keyword_loc + strlen("for"));

                    char ingredients_str_buffer[MAX_LINE_LENGTH];
                    strncpy(ingredients_str_buffer, ingredients_segment_start, sizeof(ingredients_str_buffer) - 1);
                    ingredients_str_buffer[sizeof(ingredients_str_buffer) - 1] = '\0';

                    if (parse_single_word_item_list(trophies_str_buffer, result.data.trade.trophies_to_give, &result.data.trade.give_count, MAX_RECIPE_INGREDIENTS) &&
                        result.data.trade.give_count > 0 && // Must give at least one trophy
                        parse_ingredient_list(ingredients_str_buffer, result.data.trade.ingredients_to_receive, &result.data.trade.receive_count, MAX_RECIPE_INGREDIENTS) &&
                        result.data.trade.receive_count > 0) { // Must receive at least one ingredient
                        result.type = CMD_TRADE;
                    }
                }
            }
            return result;
        }
        p = p_after_geralt;

        if (match_and_advance(&p, "brews")) {
            const char *end_of_parsed_name = NULL;
            // p now points after "brews "
            if (parse_potion_name(p, result.data.brew.potion_name, MAX_NAME_LENGTH, &end_of_parsed_name)) {
                const char *remainder = skip_whitespace_const(end_of_parsed_name);
                 // For a valid brew command, there should be nothing after the parsed potion name.
                if (*remainder == '\0') {
                   result.type = CMD_BREW;
                }
            }
            return result;
        }
        p = p_after_geralt;

        if (match_and_advance(&p, "learns")) {
            const char *learn_content_start = p; // p is after "learns " and its subsequent space
            const char *keyword_seq_start_ptr = NULL;
            const char *keyword_seq_end_ptr = NULL;

            // Try "sign is effective against"
            keyword_seq_start_ptr = find_keyword_sequence(learn_content_start, "sign", "is", "effective", "against", &keyword_seq_end_ptr);
            if (keyword_seq_start_ptr) {
                char item_name_segment[MAX_LINE_LENGTH];
                ptrdiff_t item_len = keyword_seq_start_ptr - learn_content_start;

                if (item_len >= 0) {
                    if (item_len > 0) {
                        strncpy(item_name_segment, learn_content_start, item_len);
                        item_name_segment[item_len] = '\0';
                    } else {
                        item_name_segment[0] = '\0'; // Item name part is empty
                    }
                    trim_whitespace(item_name_segment);

                    const char *details_segment_start = keyword_seq_end_ptr; 
                    char monster_name_buffer[MAX_LINE_LENGTH];
                    strncpy(monster_name_buffer, details_segment_start, sizeof(monster_name_buffer)-1);
                    monster_name_buffer[sizeof(monster_name_buffer)-1] = '\0';
                    trim_whitespace(monster_name_buffer);

                    if (strlen(item_name_segment) > 0 && // Item name must not be empty
                        parse_name(item_name_segment, result.data.learn_effectiveness.item_name, MAX_NAME_LENGTH, 0) && // Sign names no spaces
                        strlen(monster_name_buffer) > 0 && // Monster name must not be empty
                        parse_name(monster_name_buffer, result.data.learn_effectiveness.monster_name, MAX_NAME_LENGTH, 0)) {
                        result.data.learn_effectiveness.item_type = EFFECTIVE_SIGN;
                        result.type = CMD_LEARN_EFFECTIVENESS;
                    }
                }
                return result;
            }

            // Try "potion is effective against"
            keyword_seq_start_ptr = find_keyword_sequence(learn_content_start, "potion", "is", "effective", "against", &keyword_seq_end_ptr);
            if (keyword_seq_start_ptr) {
                char item_name_segment[MAX_LINE_LENGTH];
                ptrdiff_t item_len = keyword_seq_start_ptr - learn_content_start;

                if (item_len >= 0) {
                    if (item_len > 0) {
                        strncpy(item_name_segment, learn_content_start, item_len);
                        item_name_segment[item_len] = '\0';
                    } else {
                        item_name_segment[0] = '\0';
                    }
                    trim_whitespace(item_name_segment);

                    const char *details_segment_start = keyword_seq_end_ptr;
                    char monster_name_buffer[MAX_LINE_LENGTH];
                    strncpy(monster_name_buffer, details_segment_start, sizeof(monster_name_buffer)-1);
                    monster_name_buffer[sizeof(monster_name_buffer)-1] = '\0';
                    trim_whitespace(monster_name_buffer);

                    if (strlen(item_name_segment) > 0 &&
                        parse_name(item_name_segment, result.data.learn_effectiveness.item_name, MAX_NAME_LENGTH, 1) && // Potion names allow spaces
                        strlen(monster_name_buffer) > 0 &&
                        parse_name(monster_name_buffer, result.data.learn_effectiveness.monster_name, MAX_NAME_LENGTH, 0)) {
                        result.data.learn_effectiveness.item_type = EFFECTIVE_POTION;
                        result.type = CMD_LEARN_EFFECTIVENESS;
                    }
                }
                return result;
            }

            // Try "potion consists of"
            keyword_seq_start_ptr = find_keyword_sequence(learn_content_start, "potion", "consists", "of", NULL, &keyword_seq_end_ptr);
            if (keyword_seq_start_ptr) {
                char item_name_segment[MAX_LINE_LENGTH];
                ptrdiff_t item_len = keyword_seq_start_ptr - learn_content_start;

                if (item_len >= 0) {
                     if (item_len > 0) {
                        strncpy(item_name_segment, learn_content_start, item_len);
                        item_name_segment[item_len] = '\0';
                    } else {
                        item_name_segment[0] = '\0';
                    }
                    trim_whitespace(item_name_segment);

                    const char *details_segment_start = keyword_seq_end_ptr;
                    char ingredients_buffer_for_strtok[MAX_LINE_LENGTH]; // This buffer will be modified by strtok_r
                    strncpy(ingredients_buffer_for_strtok, details_segment_start, sizeof(ingredients_buffer_for_strtok)-1);
                    ingredients_buffer_for_strtok[sizeof(ingredients_buffer_for_strtok)-1] = '\0';
                    // Individual ingredient tokens will be trimmed by parse_formula_ingredient_list

                    if (strlen(item_name_segment) > 0 &&
                        parse_name(item_name_segment, result.data.learn_formula.potion_name, MAX_NAME_LENGTH, 1) && // Potion names allow spaces
                        parse_formula_ingredient_list(ingredients_buffer_for_strtok, result.data.learn_formula.requirements, &result.data.learn_formula.requirement_count, MAX_RECIPE_INGREDIENTS) &&
                        result.data.learn_formula.requirement_count > 0 ) { // Must have at least one ingredient
                        result.type = CMD_LEARN_FORMULA;
                    }
                }
                return result;
            }
            return result; // If no learn pattern matched
        }
        p = p_after_geralt;

        if (match_and_advance(&p, "encounters")) {
            if (match_and_advance(&p, "a")) {
                char monster_name_segment[MAX_LINE_LENGTH];
                strncpy(monster_name_segment, p, sizeof(monster_name_segment)-1);
                monster_name_segment[sizeof(monster_name_segment)-1] = '\0';
                trim_whitespace(monster_name_segment);
                if (strlen(monster_name_segment) > 0 &&
                    parse_name(monster_name_segment, result.data.encounter.monster_name, MAX_NAME_LENGTH, 0)) { // Monster names no spaces
                    result.type = CMD_ENCOUNTER;
                }
            }
            return result;
        }
        return result;
    }
    p = original_p_for_geralt_cmds; // Reset p if "Geralt" wasn't matched


    // --- Query Commands ---
    const char* original_p_for_query_cmds = p;
    if (match_and_advance(&p, "Total")) {
        const char *query_body_start = p;
        const char *question_mark_loc = strrchr(query_body_start, '?');

        if (question_mark_loc && *(skip_whitespace_const(question_mark_loc + 1)) == '\0') { // Must end with '?'
            char query_content_buffer[MAX_LINE_LENGTH];
            ptrdiff_t content_len = question_mark_loc - query_body_start;
            if (content_len < 0 && strlen(query_body_start) > 0 && query_body_start[0] == '?') { // Case: "Total?"
                 /* Allow "Total?" to be invalid, needs category */
                 return result;
            }
            if (content_len < 0) return result; // Invalid if '?' is before start or content is just '?'

            strncpy(query_content_buffer, query_body_start, content_len);
            query_content_buffer[content_len] = '\0';
            trim_whitespace(query_content_buffer);

            if (strlen(query_content_buffer) == 0) return result; // e.g. "Total ?"

            char *category_str_mut = query_content_buffer;
            char *item_name_str_mut = NULL;

            char *first_space_in_query_content = strchr(category_str_mut, ' ');

            if (first_space_in_query_content) {
                *first_space_in_query_content = '\0'; // Null-terminate category part
                item_name_str_mut = first_space_in_query_content + 1;
                item_name_str_mut = (char*)skip_whitespace_const(item_name_str_mut); 
            }

            if (item_name_str_mut && *item_name_str_mut != '\0') { // Specific item query
                char temp_item_name[MAX_NAME_LENGTH]; // Use a temp buffer for trimming item name
                strncpy(temp_item_name, item_name_str_mut, MAX_NAME_LENGTH -1);
                temp_item_name[MAX_NAME_LENGTH-1] = '\0';
                trim_whitespace(temp_item_name); // Trim the item name part

                if (strlen(temp_item_name) == 0) { /* e.g. "Total ingredient ?" becomes invalid */ return result; }


                if (strcmp(category_str_mut, "ingredient") == 0 || strcmp(category_str_mut, "potion") == 0 || strcmp(category_str_mut, "trophy") == 0) {
                    int is_valid_name = 0;
                    if (strcmp(category_str_mut, "potion") == 0) {
                        is_valid_name = parse_name(temp_item_name, result.data.query_total_specific.item_name, MAX_NAME_LENGTH, 1);
                    } else {
                        is_valid_name = parse_name(temp_item_name, result.data.query_total_specific.item_name, MAX_NAME_LENGTH, 0);
                    }
                    if (is_valid_name) {
                        strncpy(result.data.query_total_specific.category, category_str_mut, sizeof(result.data.query_total_specific.category) - 1);
                        result.data.query_total_specific.category[sizeof(result.data.query_total_specific.category) - 1] = '\0';
                        result.type = CMD_QUERY_TOTAL_SPECIFIC;
                    }
                }
            } else { // All items in category query
                if (strcmp(category_str_mut, "ingredient") == 0 || strcmp(category_str_mut, "potion") == 0 || strcmp(category_str_mut, "trophy") == 0) {
                    strncpy(result.data.query_total_all.category, category_str_mut, sizeof(result.data.query_total_all.category) - 1);
                    result.data.query_total_all.category[sizeof(result.data.query_total_all.category) - 1] = '\0';
                    result.type = CMD_QUERY_TOTAL_ALL;
                }
            }
        }
        return result;
    }
    p = original_p_for_query_cmds;


    if (match_and_advance(&p, "What")) {
        const char *p_after_what = p; // Save position after "What "
        if (match_and_advance(&p, "is")) {
            const char* p_after_what_is = p; // Save position after "What is "
            if (match_and_advance(&p, "effective")) {
                if (match_and_advance(&p, "against")) {
                    const char *monster_segment_start = p;
                    const char *question_mark_loc = strrchr(monster_segment_start, '?');
                    if (question_mark_loc && *(skip_whitespace_const(question_mark_loc + 1)) == '\0') {
                        char monster_name_buffer[MAX_LINE_LENGTH];
                        ptrdiff_t name_len = question_mark_loc - monster_segment_start;
                        if (name_len < 0 && strlen(monster_segment_start) > 0 && monster_segment_start[0] == '?') { return result;} // "What is effective against?"
                        if (name_len < 0) return result; // Malformed

                        strncpy(monster_name_buffer, monster_segment_start, name_len);
                        monster_name_buffer[name_len] = '\0';
                        trim_whitespace(monster_name_buffer);

                        if (strlen(monster_name_buffer) > 0 &&
                            parse_name(monster_name_buffer, result.data.query_effective_against.monster_name, MAX_NAME_LENGTH, 0)) { // Monster names no spaces
                            result.type = CMD_QUERY_EFFECTIVE_AGAINST;
                        }
                    }
                    return result;
                }
            }
            p = p_after_what_is; // Reset to after "What is "
            if (match_and_advance(&p, "in")) {
                const char *potion_segment_start = p;
                const char *question_mark_loc = strrchr(potion_segment_start, '?');
                 if (question_mark_loc && *(skip_whitespace_const(question_mark_loc + 1)) == '\0') {
                    char potion_name_buffer[MAX_LINE_LENGTH];
                    ptrdiff_t name_len = question_mark_loc - potion_segment_start;

                    if (name_len < 0 && strlen(potion_segment_start) > 0 && potion_segment_start[0] == '?') { return result;} // "What is in?"
                    if (name_len < 0) return result; // Malformed

                    strncpy(potion_name_buffer, potion_segment_start, name_len);
                    potion_name_buffer[name_len] = '\0';
                    trim_whitespace(potion_name_buffer);
                    if (strlen(potion_name_buffer) > 0 &&
                        parse_name(potion_name_buffer, result.data.query_what_is_in.potion_name, MAX_NAME_LENGTH, 1)) { // Potion names allow spaces
                        result.type = CMD_QUERY_WHAT_IS_IN;
                    }
                }
                return result;
            }
             return result; 
        }
         return result; 
    }

    return result; // Default to CMD_INVALID if no pattern matched
}


// --- Command Handler Functions ---
void handle_loot(Inventory *inv, const ParsedCommand *cmd) {
    for (int i = 0; i < cmd->data.loot.item_count; ++i) {
        add_ingredient(inv, cmd->data.loot.items[i].name, cmd->data.loot.items[i].quantity);
    }
    printf("Alchemy ingredients obtained\n");
}

void handle_trade(Inventory *inv, const ParsedCommand *cmd) {
    int can_trade = 1;
    for (int i = 0; i < cmd->data.trade.give_count; ++i) {
        if (get_trophy_quantity(inv, cmd->data.trade.trophies_to_give[i].name) < cmd->data.trade.trophies_to_give[i].quantity) {
            can_trade = 0;
            break;
        }
    }

    if (!can_trade) {
        printf("Not enough trophies\n");
        return;
    }

    for (int i = 0; i < cmd->data.trade.give_count; ++i) {
        if (!use_trophy(inv, cmd->data.trade.trophies_to_give[i].name, cmd->data.trade.trophies_to_give[i].quantity)) {
             printf("Trade failed internally (trophy use error)\n"); 
             return;
        }
    }
    for (int i = 0; i < cmd->data.trade.receive_count; ++i) {
        add_ingredient(inv, cmd->data.trade.ingredients_to_receive[i].name, cmd->data.trade.ingredients_to_receive[i].quantity);
    }

    printf("Trade successful\n");
}

void handle_brew(Inventory *inv, AlchemyBase *alchemy_base, const ParsedCommand *cmd) {
    const char *potion_name = cmd->data.brew.potion_name;
    const PotionFormula *formula = find_formula(alchemy_base, potion_name);
    if (!formula) {
        printf("No formula for %s\n", potion_name);
        return;
    }

    int has_ingredients = 1;
    for (int i = 0; i < formula->requirement_count; ++i) {
        if (get_ingredient_quantity(inv, formula->requirements[i].ingredient_name) < formula->requirements[i].quantity) {
            has_ingredients = 0;
            break;
        }
    }
    if (!has_ingredients) {
        printf("Not enough ingredients\n");
        return;
    }

    for (int i = 0; i < formula->requirement_count; ++i) {
       if (!use_ingredient(inv, formula->requirements[i].ingredient_name, formula->requirements[i].quantity)){
             printf("Brewing failed internally (ingredient use error)\n"); 
             return;
       }
    }

    add_potion(inv, potion_name, 1);
    printf("Alchemy item created: %s\n", potion_name);
}

void handle_learn_effectiveness(Bestiary *bestiary, const ParsedCommand *cmd) {
    const char *item_name = cmd->data.learn_effectiveness.item_name;
    const char *monster_name = cmd->data.learn_effectiveness.monster_name;
    EffectivenessType type = cmd->data.learn_effectiveness.item_type;
    int result = add_effectiveness(bestiary, monster_name, item_name, type);

    switch (result) {
        case 2: printf("New bestiary entry added: %s\n", monster_name); break;
        case 1: printf("Bestiary entry updated: %s\n", monster_name); break;
        case 0: printf("Already known effectiveness\n"); break;
        case -1:
             printf("Could not learn effectiveness (limit reached?)\n");
             break;
        default:
             printf("INVALID\n"); 
             break;
    }
}

void handle_learn_formula(AlchemyBase *alchemy_base, const ParsedCommand *cmd) {
    const char *potion_name = cmd->data.learn_formula.potion_name;
    
    // Check if already known BEFORE attempting to add, to give correct message
    if (find_formula(alchemy_base, potion_name) != NULL) {
        printf("Already known formula\n");
        return;
    }

    int result = add_formula(alchemy_base, potion_name, cmd->data.learn_formula.requirements, cmd->data.learn_formula.requirement_count);

    if (result == 1) {
        printf("New alchemy formula obtained: %s\n", potion_name);
    } else { 
        // If it wasn't already known, and add_formula failed, it's a capacity/other error
        printf("Could not learn formula (limit reached or invalid?)\n");
    }
}

void handle_encounter(Inventory *inv, const Bestiary *bestiary, const ParsedCommand *cmd) {
    const char *monster_name = cmd->data.encounter.monster_name;
    const BestiaryEntry *entry = find_bestiary_entry(bestiary, monster_name);
    int success = 0;
    int potion_to_use = 0;
    const char *effective_potion_name = NULL;

    if (entry) {
        for (int i = 0; i < entry->effective_item_count; ++i) {
            if (entry->effective_items[i].type == EFFECTIVE_SIGN) {
                success = 1;
                break;
            }
        }
        if (!success) {
            for (int i = 0; i < entry->effective_item_count; ++i) {
                if (entry->effective_items[i].type == EFFECTIVE_POTION) {
                    if (get_potion_quantity(inv, entry->effective_items[i].name) > 0) {
                        success = 1;
                        potion_to_use = 1;
                        effective_potion_name = entry->effective_items[i].name;
                        break;
                    }
                }
            }
        }
    }

    if (success) {
        printf("Geralt defeats %s\n", monster_name);
        if (potion_to_use && effective_potion_name) {
            if (!use_potion(inv, effective_potion_name, 1)) {
                 printf("Internal encounter error (potion use failed).\n"); 
            }
        }
        add_trophy(inv, monster_name, 1);
    } else {
        printf("Geralt is unprepared and barely escapes with his life\n");
    }
}

void handle_query_total_specific(const Inventory *inv, const ParsedCommand *cmd) {
    const char *category = cmd->data.query_total_specific.category;
    const char *item_name = cmd->data.query_total_specific.item_name;
    int quantity = 0;

    if (strcmp(category, "ingredient") == 0) {
        quantity = get_ingredient_quantity(inv, item_name);
    } else if (strcmp(category, "potion") == 0) {
        quantity = get_potion_quantity(inv, item_name);
    } else if (strcmp(category, "trophy") == 0) {
        quantity = get_trophy_quantity(inv, item_name);
    } else {
        printf("INVALID\n"); 
        return;
    }
    printf("%d\n", quantity);
}

void handle_query_total_all(const Inventory *inv, const ParsedCommand *cmd) {
    const char *category = cmd->data.query_total_all.category;

    if (strcmp(category, "ingredient") == 0) {
        print_all_ingredients(inv);
    } else if (strcmp(category, "potion") == 0) {
        print_all_potions(inv);
    } else if (strcmp(category, "trophy") == 0) {
        print_all_trophies(inv);
    } else {
        printf("INVALID\n"); 
    }
}

void handle_query_effective_against(const Bestiary *bestiary, const ParsedCommand *cmd) {
    const char *monster_name = cmd->data.query_effective_against.monster_name;
    const BestiaryEntry *entry = find_bestiary_entry(bestiary, monster_name);

    if (entry && entry->effective_item_count > 0) {
        print_effectiveness(entry);
    } else {
        printf("No knowledge of %s\n", monster_name);
    }
}

void handle_query_what_is_in(const AlchemyBase *alchemy_base, const ParsedCommand *cmd) {
    const char *potion_name = cmd->data.query_what_is_in.potion_name;
    const PotionFormula *formula = find_formula(alchemy_base, potion_name);

    if (formula) {
        print_formula(formula);
    } else {
        printf("No formula for %s\n", potion_name);
    }
}


// --- Main Program Entry Point ---
int main(void) {
    Inventory geralt_inventory;
    AlchemyBase geralt_alchemy;
    Bestiary geralt_bestiary;

    init_inventory(&geralt_inventory);
    init_alchemy_base(&geralt_alchemy);
    init_bestiary(&geralt_bestiary);

    char line[MAX_LINE_LENGTH];

    while (1) {
        printf(">> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (feof(stdin)) {
                break;
            } else {
                return EXIT_FAILURE;
            }
        }

        line[strcspn(line, "\n")] = 0;

        ParsedCommand cmd = parse_command(line);

        if (cmd.type == CMD_EXIT) {
            break;
        }

        if (cmd.type == CMD_LOOT) {
            handle_loot(&geralt_inventory, &cmd);
        } else if (cmd.type == CMD_TRADE) {
            handle_trade(&geralt_inventory, &cmd);
        } else if (cmd.type == CMD_BREW) {
            handle_brew(&geralt_inventory, &geralt_alchemy, &cmd);
        } else if (cmd.type == CMD_LEARN_EFFECTIVENESS) {
            handle_learn_effectiveness(&geralt_bestiary, &cmd);
        } else if (cmd.type == CMD_LEARN_FORMULA) {
            handle_learn_formula(&geralt_alchemy, &cmd);
        } else if (cmd.type == CMD_ENCOUNTER) {
            handle_encounter(&geralt_inventory, &geralt_bestiary, &cmd);
        } else if (cmd.type == CMD_QUERY_TOTAL_SPECIFIC) {
            handle_query_total_specific(&geralt_inventory, &cmd);
        } else if (cmd.type == CMD_QUERY_TOTAL_ALL) {
            handle_query_total_all(&geralt_inventory, &cmd);
        } else if (cmd.type == CMD_QUERY_EFFECTIVE_AGAINST) {
            handle_query_effective_against(&geralt_bestiary, &cmd);
        } else if (cmd.type == CMD_QUERY_WHAT_IS_IN) {
            handle_query_what_is_in(&geralt_alchemy, &cmd);
        } else if (cmd.type == CMD_EMPTY) {
            // continue; // Do nothing specific for empty, main loop continues
        } else if (cmd.type == CMD_INVALID) {
            printf("INVALID\n");
        } else {
             printf("INVALID\n"); // Fallback for any unhandled properly parsed type
        }
        fflush(stdout);
    }
    return 0;
}