#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>     
#include <variant>     
#include <optional>    
#include <string_view> 

namespace GameConstants {
    const size_t MAX_NAME_LENGTH = 128;       // Logical length limit for names
    const size_t MAX_ITEMS = 128;             // Generic item limit (inventories, formulae count, etc.)
    const size_t MAX_RECIPE_INGREDIENTS = 64; // Max ingredients in a formula or items in loot/trade
    const size_t MAX_EFFECTIVE_ITEMS = 64;    // Max effective items per bestiary entry
}

// Specifies the type of effectiveness an item has against a monster
enum class EffectivenessType {
    POTION,
    SIGN
};

// Defines the types of commands that can be entered by the user
enum class CommandType {
    LOOT,
    TRADE,
    BREW,
    LEARN_EFFECTIVENESS,
    LEARN_FORMULA,
    ENCOUNTER,
    QUERY_TOTAL_SPECIFIC,
    QUERY_TOTAL_ALL,
    QUERY_EFFECTIVE_AGAINST,
    QUERY_WHAT_IS_IN,
    EXIT,
    INVALID,
    EMPTY
};

namespace Parsed { // Namespace for parsed command data structures

    // Basic structure to hold item name and quantity
    struct ItemInfo {
        std::string name;
        int quantity;

        ItemInfo(std::string n = "", int q = 0) : name(std::move(n)), quantity(q) {}
    };

    // Payload structures for different command types
    // These will be used within the std::variant in Parsed::Command.
    struct LootPayload {
        std::vector<ItemInfo> items; // Items looted
    };

    struct TradePayload {
        std::vector<ItemInfo> trophies_to_give;     // Trophies to give in a trade
        std::vector<ItemInfo> ingredients_to_receive; // Ingredients to receive
    };

    struct BrewPayload {
        std::string potion_name; // Name of the potion to brew
    };

    struct LearnEffectivenessPayload {
        std::string item_name;          // Name of the item (potion/sign) whose effectiveness is learned
        EffectivenessType item_type;    // Type of the item (POTION or SIGN)
        std::string monster_name;       // Name of the monster the item is effective against
    };

    struct LearnFormulaPayload {
        std::string potion_name;            // Name of the potion whose formula is learned
        std::vector<ItemInfo> requirements; // Ingredients required for the potion
    };

    struct EncounterPayload {
        std::string monster_name; // Name of the encountered monster
    };

    struct QueryTotalSpecificPayload {
        std::string category; // Category being queried ("ingredient", "potion", "trophy")
        std::string item_name;// Name of the specific item whose total is queried
    };

    struct QueryTotalAllPayload {
        std::string category; // Category for which all items are to be listed
    };

    struct QueryEffectiveAgainstPayload {
        std::string monster_name; // Monster whose effectiveness data is queried
    };

    struct QueryWhatIsInPayload {
        std::string potion_name; // Potion whose ingredients are queried
    };

    struct EmptyPayload {}; // For commands that don't have specific data (e.g., EXIT, EMPTY)

    // Main parsed command structure
    // Holds the command type and command-specific data using std::variant
    struct Command {
        CommandType type; // The type of the command
        std::variant<     // Command-specific data (C++ alternative to C's union)
            EmptyPayload,
            LootPayload,
            TradePayload,
            BrewPayload,
            LearnEffectivenessPayload,
            LearnFormulaPayload,
            EncounterPayload,
            QueryTotalSpecificPayload,
            QueryTotalAllPayload,
            QueryEffectiveAgainstPayload,
            QueryWhatIsInPayload
        > data;

        // Default constructor initializes the command as INVALID with an EmptyPayload
        Command() : type(CommandType::INVALID), data(EmptyPayload{}) {}
    };

} // namespace Parsed


namespace ParserUtils { // Namespace for parsing utility functions

    // Trims leading and trailing whitespace from a std::string in-place
    void trim_whitespace_in_place(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch); // Find first non-whitespace character
        }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch); // Find first non-whitespace character from the end
        }).base(), s.end());
    }
    
    // Returns a new std::string with leading and trailing whitespace removed
    std::string trim_whitespace_str(std::string s) {
        trim_whitespace_in_place(s); // Trim in-place and return the copy
        return s;
    }

    // Tries to parse a quantity (positive integer) from a string token
    // Returns the quantity if successful, std::nullopt otherwise
    std::optional<int> parse_quantity(const std::string& token_str) {
        if (token_str.empty()) return std::nullopt;
        try {
            size_t pos; // For std::stol, stores the position of the first unparsed char
            long val = std::stol(token_str, &pos); // Convert string to long
            // Validation: entire string parsed? positive? within int limits?
            if (pos != token_str.length() || val <= 0 || val > 2147483647) {
                return std::nullopt;
            }
            return static_cast<int>(val);
        } catch (const std::invalid_argument&) { // e.g., "abc"
            return std::nullopt;
        } catch (const std::out_of_range&) {     // Value out of long's range
            return std::nullopt;
        }
    }

    // Tries to parse a valid item/monster/potion name from a string token
    std::optional<std::string> parse_name(const std::string& token_str_in, bool allow_spaces) {
        std::string token_str = trim_whitespace_str(token_str_in); // Trim leading/trailing spaces first

        if (token_str.empty() || token_str.length() >= GameConstants::MAX_NAME_LENGTH) return std::nullopt;

        bool char_found = false;        // Has at least one alphabetic character been found?
        bool last_char_was_space = false; // Was the previous character a space (for consecutive space check)?
        for (char c_char : token_str) {
            unsigned char c = static_cast<unsigned char>(c_char);
            if (std::isalpha(c)) { // If character is a letter
                char_found = true;
                last_char_was_space = false;
            } else if (std::isspace(c)) { // If character is a space
                if (!allow_spaces) return std::nullopt; // Invalid if spaces are not allowed
                if (last_char_was_space) return std::nullopt; // Invalid if previous char was also a space
                last_char_was_space = true;
            } else { // Invalid character (e.g., number, special symbol)
                return std::nullopt; 
            }
        }
        if (!char_found) return std::nullopt; // Invalid if no alphabetic characters were found (e.g., string of only spaces)
        return token_str; // Valid name
    }

    // Splits a string by a delimiter and returns a vector of tokens
    std::vector<std::string> split_string(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s); // Use a string stream to read from the string
        while (std::getline(tokenStream, token, delimiter)) { // Read until the delimiter
            tokens.push_back(token);
        }
        return tokens;
    }

    // Parses a list of items in "quantity name, quantity name, ..." format
    // If `item_names_allow_spaces` is true, item names can contain spaces.
    std::optional<std::vector<Parsed::ItemInfo>> parse_item_list(const std::string& list_str_in, bool item_names_allow_spaces) {
        std::string list_str = trim_whitespace_str(list_str_in); // Trim the input list string
        if (list_str.empty()) return std::vector<Parsed::ItemInfo>{}; // An empty list is valid (0 items)

        std::vector<Parsed::ItemInfo> parsed_items;
        std::vector<std::string> tokens = split_string(list_str, ','); // Split by comma
        
        // If the string is not empty but split_string returned an empty vector (no commas),
        // it might be a single item.
        if (tokens.empty() && !list_str.empty()) {
             tokens.push_back(list_str); // Treat the whole string as a single token
        }

        for (std::string token : tokens) { // For each "quantity name" part
            trim_whitespace_in_place(token); // Trim the token
            if (token.empty()) return std::nullopt; // Empty token (e.g., "1 apple, , 2 pear") is invalid

            size_t first_space_pos = token.find(' '); // Find the first space between quantity and name
            if (first_space_pos == std::string::npos || first_space_pos == 0) return std::nullopt; // No space or space at start

            std::string qty_str = token.substr(0, first_space_pos); // Quantity string
            std::string name_str_raw = token.substr(first_space_pos + 1); // Raw name string
            trim_whitespace_in_place(name_str_raw); // Trim the name string

            if (name_str_raw.empty()) return std::nullopt; // Name part is empty after quantity

            std::optional<int> quantity_opt = parse_quantity(qty_str); // Parse quantity
            std::optional<std::string> name_opt = parse_name(name_str_raw, item_names_allow_spaces); // Parse name

            if (!quantity_opt || !name_opt) { // If quantity or name is invalid
                return std::nullopt; 
            }
            
            if (parsed_items.size() >= GameConstants::MAX_RECIPE_INGREDIENTS) return std::nullopt; // Too many items
            parsed_items.emplace_back(name_opt.value(), quantity_opt.value()); // Add valid item to list
        }
        return parsed_items; // Parsed list of items
    }
    
    // Complex potion name parsing. A potion name can be terminated by keywords like
    // " potion is effective against", " potion consists of", or a question mark '?'.
    // Returns: <parsed potion name (optional), remaining string_view>
    std::pair<std::optional<std::string>, std::string_view>
    parse_potion_name_complex(std::string_view full_text) {
        std::string_view current_view = full_text;
        // Skip leading whitespace
        size_t first_char = current_view.find_first_not_of(" \t\n\r\f\v");
        if (first_char == std::string_view::npos) return {std::nullopt, ""}; // Only whitespace or empty
        current_view.remove_prefix(first_char);

        if (current_view.empty()) return {std::nullopt, ""}; // Empty after skipping whitespace

        size_t end_pos = std::string_view::npos; // Index of the end of the potion name
        // Potential terminating keyword phrases
        const char* terminators[] = {" potion is effective against", " potion consists of"};
        
        for (const char* term_c_str : terminators) {
            std::string_view term_sv(term_c_str);
            // We need to find the "potion" part of the terminator, not " potion"
            size_t term_keyword_start = term_sv.find("potion"); 
            if (term_keyword_start == std::string_view::npos) continue; // Error case, should not happen

            std::string_view actual_term_to_find = term_sv.substr(term_keyword_start); // e.g., "potion is effective against"
            size_t found_pos = current_view.find(actual_term_to_find); // Search for this in the main text
            if (found_pos != std::string_view::npos) {
                // Check if "potion" is a whole word (preceded by space or start of string)
                if (found_pos == 0 || std::isspace(static_cast<unsigned char>(current_view[found_pos-1]))) {
                     // If no terminator found yet, or this one ends earlier, take this one
                     if (end_pos == std::string_view::npos || found_pos < end_pos) {
                        end_pos = found_pos;
                    }
                }
            }
        }
        
        // Also check for '?' as a terminator
        size_t q_mark_pos = current_view.find('?');
        if (q_mark_pos != std::string_view::npos) {
             if (end_pos == std::string_view::npos || q_mark_pos < end_pos) {
                end_pos = q_mark_pos;
            }
        }

        std::string potion_name_str;
        std::string_view remainder_view;

        if (end_pos == std::string_view::npos) { // If no specific terminator found, whole string is potion name
            potion_name_str = std::string(current_view);
            remainder_view = current_view.substr(current_view.length()); // Remainder is empty
        } else { // Terminator found
            potion_name_str = std::string(current_view.substr(0, end_pos)); // Potion name is up to terminator
            remainder_view = current_view.substr(end_pos); // Remainder is from terminator onwards
        }
        
        trim_whitespace_in_place(potion_name_str); // Trim the extracted potion name
        auto validated_name = parse_name(potion_name_str, true); // Potion names can have spaces
        if (!validated_name || validated_name.value().empty()) { // Invalid or empty name
            return {std::nullopt, full_text}; // Return original text on error
        }
        return {validated_name.value(), remainder_view}; // Success: name and remainder
    }

    // Advances a string_view past any leading whitespace
    void advance_past_whitespace(std::string_view& sv) {
        size_t first_char = sv.find_first_not_of(" \t\n\r\f\v");
        if (first_char == std::string_view::npos) { // All whitespace
            sv.remove_prefix(sv.length()); // Empty the string_view
        } else {
            sv.remove_prefix(first_char); // Advance to the first non-whitespace character
        }
    }

    // Tries to match a keyword at the beginning of a string_view.
    // If matched, advances the string_view past the keyword and subsequent whitespace.
    // Returns true if matched, false otherwise.
    bool match_and_advance(std::string_view& sv, const std::string& keyword) {
        advance_past_whitespace(sv); // Skip leading whitespace first
        if (sv.rfind(keyword, 0) == 0) { // Does string_view start with the keyword?
            // Check for whole word match (is it followed by space or end of string?)
            if (sv.length() == keyword.length() || std::isspace(static_cast<unsigned char>(sv[keyword.length()]))) {
                sv.remove_prefix(keyword.length()); // Advance past keyword
                advance_past_whitespace(sv); // Advance past subsequent whitespace
                return true;
            }
        }
        return false;
    }
    
    // Finds a "standalone" substring (keyword) within a text.
    // Standalone: surrounded by whitespace or string boundaries.
    // Returns: <string_view of text after the keyword, start_pos of keyword in original haystack>
    // Returns std::nullopt if not found.
    std::optional<std::pair<std::string_view, size_t>>
    find_standalone_substring(std::string_view haystack, const std::string& needle) {
        if (needle.empty()) return std::nullopt;
        size_t current_search_offset = 0; // Search start position within haystack
        while (current_search_offset < haystack.length()) {
            size_t found_pos = haystack.find(needle, current_search_offset); // Search for needle
            if (found_pos == std::string_view::npos) { // Not found
                return std::nullopt;
            }

            bool is_standalone = true;
            // Check character before needle
            if (found_pos > 0 && !std::isspace(static_cast<unsigned char>(haystack[found_pos - 1]))) {
                is_standalone = false;
            }
            // Check character after needle
            char char_after_needle = (found_pos + needle.length() < haystack.length()) ? haystack[found_pos + needle.length()] : '\0';
            if (char_after_needle != '\0' && !std::isspace(static_cast<unsigned char>(char_after_needle))) {
                is_standalone = false;
            }

            if (is_standalone) { // Found a standalone instance
                std::string_view remainder = haystack;
                remainder.remove_prefix(found_pos + needle.length()); // Get text after needle
                // advance_past_whitespace(remainder); // Let caller decide if they want to skip space after needle
                return std::make_pair(remainder, found_pos);
            }
            // Not standalone, continue search from the character after this found_pos
            current_search_offset = found_pos + 1; 
        }
        return std::nullopt; // Loop finished, not found
    }

    // Finds a sequence of keywords in a text.
    // Returns a pair:
    //   - first: optional string_view of the text *before* the first keyword of the sequence.
    //   - second: optional string_view of the text *after* the last matched keyword and its subsequent space.
    // Returns {std::nullopt, std::nullopt} if the sequence is not found.
    std::pair<std::optional<std::string_view>, std::optional<std::string_view>>
    find_keyword_sequence(std::string_view text, const std::vector<std::string>& keywords) {
        if (keywords.empty() || keywords[0].empty()) return {std::nullopt, std::nullopt};

        std::string_view current_search_origin = text; // The part of text where we are currently looking for keywords[0]
        size_t total_offset_from_original_text = 0;   // How far current_search_origin is from text.data()

        while (!current_search_origin.empty()) { // Until we run out of text to search
            // Find the first keyword (keywords[0]) in current_search_origin
            auto find_kw0_res = find_standalone_substring(current_search_origin, keywords[0]);
            if (!find_kw0_res) return {std::nullopt, std::nullopt}; // First keyword not found anywhere

            // find_kw0_res.value().second -> start of kw0 within current_search_origin
            // find_kw0_res.value().first  -> text after kw0 (relative to current_search_origin)
            size_t kw0_start_in_search_origin = find_kw0_res.value().second;
            std::string_view after_kw0_in_search_origin = find_kw0_res.value().first;
            advance_past_whitespace(after_kw0_in_search_origin); // Skip kw0 and its trailing space

            // Text part before the start of the keyword sequence (relative to original 'text')
            std::string_view text_before_kw0_sequence = text.substr(
                0, 
                total_offset_from_original_text + kw0_start_in_search_origin
            );

            bool sequence_match = true; // Did the entire keyword sequence match?
            std::string_view current_segment_after_matched_kw = after_kw0_in_search_origin; // Where to look for the next keyword

            // Check the rest of the keywords (from keywords[1] onwards)
            for (size_t i = 1; i < keywords.size(); ++i) {
                if (keywords[i].empty()) break; // Empty keyword means end of sequence definition

                // Does current_segment_after_matched_kw start with keywords[i]?
                if (current_segment_after_matched_kw.rfind(keywords[i], 0) == 0) { 
                    // Is it a whole word match?
                    if (current_segment_after_matched_kw.length() == keywords[i].length() || 
                        std::isspace(static_cast<unsigned char>(current_segment_after_matched_kw[keywords[i].length()]))) {
                        current_segment_after_matched_kw.remove_prefix(keywords[i].length()); // Skip matched keyword
                        advance_past_whitespace(current_segment_after_matched_kw); // Skip its trailing space
                    } else { // Not a whole word
                        sequence_match = false;
                        break;
                    }
                } else { // Does not start with the keyword
                    sequence_match = false;
                    break;
                }
            }

            if (sequence_match) { // Entire sequence matched
                return {text_before_kw0_sequence, current_segment_after_matched_kw};
            }

            // First keyword was found, but the rest of the sequence didn't match.
            // Advance search origin in the original 'text' to look for keywords[0] again.
            size_t advance_in_search_origin = kw0_start_in_search_origin + 1;
            if (advance_in_search_origin >= current_search_origin.length()) break; // Nothing left to advance into
            current_search_origin.remove_prefix(advance_in_search_origin);
            total_offset_from_original_text += advance_in_search_origin;
        }
        return {std::nullopt, std::nullopt}; // Sequence not found
    }
} // namespace ParserUtils


// This function takes a raw command line string and attempts to parse it into a Parsed::Command object.
Parsed::Command parse_command_internal(const std::string& original_line) {
    using namespace ParserUtils;

    Parsed::Command result; // Default initialized to INVALID
    std::string line_trimmed_str = trim_whitespace_str(original_line); // Make a trimmed copy
    std::string_view line_view(line_trimmed_str); // Work with a view for efficiency

    if (line_view.empty()) {
        result.type = CommandType::EMPTY;
        return result;
    }

    if (line_view == "Exit") {
        result.type = CommandType::EXIT;
        return result;
    }
    
    std::string_view p = line_view; // 'p' is our current parsing cursor (a string_view)

    // Check if the command starts with "Geralt"
    if (match_and_advance(p, "Geralt")) {
        std::string_view p_after_geralt = p; // Save state after "Geralt"

        // Geralt loots ...
        if (match_and_advance(p, "loots")) {
            auto items_opt = parse_item_list(std::string(p), false); // Looted item names don't have spaces
            if (items_opt && !items_opt.value().empty()) { // Successfully parsed a non-empty list
                result.type = CommandType::LOOT;
                result.data = Parsed::LootPayload{items_opt.value()};
            }
            return result; // Return, valid or invalid
        }
        p = p_after_geralt; // Reset for next "Geralt" command check

        // Geralt trades ... trophy for ...
        if (match_and_advance(p, "trades")) {
            std::string_view trade_content_view = p; // The part after "Geralt trades "
            // Find "trophy" then "for"
            auto trophy_kw_search_result = find_standalone_substring(trade_content_view, "trophy");
            
            if (trophy_kw_search_result) {
                size_t trophy_kw_start_offset = trophy_kw_search_result.value().second; // Where "trophy" starts
                std::string_view before_trophy_sv = trade_content_view.substr(0, trophy_kw_start_offset); // Text before "trophy"
                std::string_view after_trophy_kw_sv_temp = trophy_kw_search_result.value().first; // Text after "trophy"
                advance_past_whitespace(after_trophy_kw_sv_temp); // Skip space after "trophy "

                std::string temp_before_trophy_str = std::string(before_trophy_sv);
                trim_whitespace_in_place(temp_before_trophy_str);
                if (temp_before_trophy_str.empty()) return result; // Nothing before "trophy" keyword, invalid

                auto for_kw_search_result = find_standalone_substring(after_trophy_kw_sv_temp, "for");

                if (for_kw_search_result) {
                    std::string trophies_str = temp_before_trophy_str; // Items to give
                    std::string_view after_for_kw_sv = for_kw_search_result.value().first; // Text after "for"
                    advance_past_whitespace(after_for_kw_sv); // Skip space after "for "
                    std::string ingredients_str = std::string(after_for_kw_sv); // Items to receive

                    auto trophies_opt = parse_item_list(trophies_str, false); // Trophy names no spaces
                    auto ingredients_opt = parse_item_list(ingredients_str, false); // Ingredient names no spaces

                    if (trophies_opt && !trophies_opt.value().empty() &&
                        ingredients_opt && !ingredients_opt.value().empty()) {
                        result.type = CommandType::TRADE;
                        result.data = Parsed::TradePayload{trophies_opt.value(), ingredients_opt.value()};
                    }
                }
            }
            return result;
        }
        p = p_after_geralt;

        // Geralt brews Potion Name
        if (match_and_advance(p, "brews")) {
            // For "brews", the rest of the line is considered the potion name.
            std::string potion_name_str = std::string(p);
            trim_whitespace_in_place(potion_name_str);
            auto potion_name_opt = parse_name(potion_name_str, true); // Potion names can have spaces

            if (potion_name_opt && !potion_name_opt.value().empty()) {
                 result.type = CommandType::BREW;
                 result.data = Parsed::BrewPayload{potion_name_opt.value()};
            }
            return result;
        }
        p = p_after_geralt;

        // Geralt learns ...
        if (match_and_advance(p, "learns")) {
            std::string_view learn_content_start = p; // Text after "Geralt learns "
            
            // Geralt learns SignName sign is effective against MonsterName
            auto seq_res_sign = find_keyword_sequence(learn_content_start, {"sign", "is", "effective", "against"});
            if (seq_res_sign.first && seq_res_sign.second) { // Sequence found
                std::string item_name_str = std::string(seq_res_sign.first.value()); // Text before "sign..."
                trim_whitespace_in_place(item_name_str);
                std::string monster_name_str = std::string(seq_res_sign.second.value()); // Text after "...against "
                trim_whitespace_in_place(monster_name_str);

                auto item_name_opt = parse_name(item_name_str, false); // Sign names are single words
                auto monster_name_opt = parse_name(monster_name_str, false); // Monster names are single words

                if (item_name_opt && monster_name_opt && !item_name_opt.value().empty() && !monster_name_opt.value().empty()) {
                    result.type = CommandType::LEARN_EFFECTIVENESS;
                    result.data = Parsed::LearnEffectivenessPayload{item_name_opt.value(), EffectivenessType::SIGN, monster_name_opt.value()};
                }
                return result;
            }

            // Geralt learns Potion Name potion is effective against MonsterName
            auto seq_res_potion_eff = find_keyword_sequence(learn_content_start, {"potion", "is", "effective", "against"});
            if (seq_res_potion_eff.first && seq_res_potion_eff.second) {
                std::string item_name_str = std::string(seq_res_potion_eff.first.value());
                trim_whitespace_in_place(item_name_str);
                std::string monster_name_str = std::string(seq_res_potion_eff.second.value());
                trim_whitespace_in_place(monster_name_str);

                auto item_name_opt = parse_name(item_name_str, true); // Potion names can have spaces
                auto monster_name_opt = parse_name(monster_name_str, false);

                if (item_name_opt && monster_name_opt && !item_name_opt.value().empty() && !monster_name_opt.value().empty()) {
                    result.type = CommandType::LEARN_EFFECTIVENESS;
                    result.data = Parsed::LearnEffectivenessPayload{item_name_opt.value(), EffectivenessType::POTION, monster_name_opt.value()};
                }
                return result;
            }
            
            // Geralt learns Potion Name potion consists of Ing1, Ing2...
            auto seq_res_potion_formula = find_keyword_sequence(learn_content_start, {"potion", "consists", "of"});
             if (seq_res_potion_formula.first && seq_res_potion_formula.second) {
                std::string potion_name_str = std::string(seq_res_potion_formula.first.value());
                trim_whitespace_in_place(potion_name_str);
                std::string ingredients_list_str = std::string(seq_res_potion_formula.second.value()); // Text after "...of "
                
                auto potion_name_opt = parse_name(potion_name_str, true); // Potion names allow spaces
                auto ingredients_opt = parse_item_list(ingredients_list_str, false); // Ingredient names no spaces

                if (potion_name_opt && ingredients_opt && !potion_name_opt.value().empty() && 
                    ingredients_opt.has_value() && !ingredients_opt.value().empty()) { // Check ingredients_opt has value and is not empty
                    result.type = CommandType::LEARN_FORMULA;
                    result.data = Parsed::LearnFormulaPayload{potion_name_opt.value(), ingredients_opt.value()};
                }
                return result;
            }
            return result; // No "learns" pattern matched
        }
        p = p_after_geralt; // Reset if "learns" not matched

        // Geralt encounters a MonsterName
        if (match_and_advance(p, "encounters")) {
            if (match_and_advance(p, "a")) { // Must have "a"
                std::string monster_name_str = std::string(p); // Rest of the line is monster name
                trim_whitespace_in_place(monster_name_str);
                auto monster_name_opt = parse_name(monster_name_str, false); // Monster name single word
                if (monster_name_opt && !monster_name_opt.value().empty()) {
                    result.type = CommandType::ENCOUNTER;
                    result.data = Parsed::EncounterPayload{monster_name_opt.value()};
                }
            }
            return result;
        }
        return result; // Unrecognized command after "Geralt"
    }
    p = line_view; // Reset to beginning of line if not a "Geralt" command


    // --- Query Commands ---
    // Total category [Item Name]?
    if (match_and_advance(p, "Total")) {
        std::string_view query_body = p; // Text after "Total "
        if (!query_body.empty() && query_body.back() == '?') { // Must end with '?'
            query_body.remove_suffix(1); // Remove '?'
            
            std::string query_content_str = std::string(query_body); // Make a mutable copy
            trim_whitespace_in_place(query_content_str);

            if (query_content_str.empty()) return result; // "Total ?" is invalid

            size_t first_space = query_content_str.find(' ');
            std::string category_str;
            std::string item_name_str_query; // Renamed to avoid conflict with other item_name_str

            if (first_space == std::string::npos) { // Only category, e.g., "Total ingredient?"
                category_str = query_content_str;
            } else { // Category and item name, e.g., "Total potion Healing Potion?"
                category_str = query_content_str.substr(0, first_space);
                item_name_str_query = query_content_str.substr(first_space + 1);
                trim_whitespace_in_place(item_name_str_query);
            }
            trim_whitespace_in_place(category_str);

            if (category_str != "ingredient" && category_str != "potion" && category_str != "trophy") {
                return result; // Invalid category
            }

            if (!item_name_str_query.empty()) { // Query for a specific item
                bool name_allows_spaces = (category_str == "potion");
                auto item_name_opt = parse_name(item_name_str_query, name_allows_spaces);
                if (item_name_opt && !item_name_opt.value().empty()) {
                    result.type = CommandType::QUERY_TOTAL_SPECIFIC;
                    result.data = Parsed::QueryTotalSpecificPayload{category_str, item_name_opt.value()};
                }
            } else { // Query for all items in a category
                 if (!category_str.empty()){ // Category must exist
                    result.type = CommandType::QUERY_TOTAL_ALL;
                    result.data = Parsed::QueryTotalAllPayload{category_str};
                 }
            }
        }
        return result;
    }
    p = line_view; // Reset

    // What is ...?
    if (match_and_advance(p, "What")) {
        if (match_and_advance(p, "is")) {
            std::string_view p_after_what_is = p; // Save state after "What is "

            // What is effective against MonsterName?
            if (match_and_advance(p, "effective")) {
                if (match_and_advance(p, "against")) {
                    std::string_view monster_segment = p; // Text after "...against "
                     if (!monster_segment.empty() && monster_segment.back() == '?') {
                        monster_segment.remove_suffix(1);
                        std::string monster_name_str = std::string(monster_segment);
                        trim_whitespace_in_place(monster_name_str);
                        auto monster_name_opt = parse_name(monster_name_str, false); // Monster name single word
                        if (monster_name_opt && !monster_name_opt.value().empty()) {
                            result.type = CommandType::QUERY_EFFECTIVE_AGAINST;
                            result.data = Parsed::QueryEffectiveAgainstPayload{monster_name_opt.value()};
                        }
                    }
                    return result;
                }
            }
            p = p_after_what_is; // Reset to after "What is "

            // What is in Potion Name?
            if (match_and_advance(p, "in")) {
                std::string_view potion_segment = p; // Text after "...in "
                if (!potion_segment.empty() && potion_segment.back() == '?') {
                    potion_segment.remove_suffix(1);
                    std::string potion_name_str = std::string(potion_segment);
                    trim_whitespace_in_place(potion_name_str);
                    auto potion_name_opt = parse_name(potion_name_str, true); // Potion names allow spaces
                    if (potion_name_opt && !potion_name_opt.value().empty()) {
                        result.type = CommandType::QUERY_WHAT_IS_IN;
                        result.data = Parsed::QueryWhatIsInPayload{potion_name_opt.value()};
                    }
                }
                return result;
            }
             return result; // Unrecognized after "What is"
        }
         return result; // Unrecognized after "What"
    }

    return result; // Default: INVALID if no pattern matched
}


// These classes represent the game's state and logic.

class InventoryItem {
public:
    std::string name;
    int quantity;

    InventoryItem(std::string n, int q) : name(std::move(n)), quantity(q) {}

    static bool compareByName(const InventoryItem& a, const InventoryItem& b) {
        return a.name < b.name;
    }
};

class IngredientRequirement {
public:
    std::string ingredient_name;
    int quantity;

    IngredientRequirement(std::string name, int q) : ingredient_name(std::move(name)), quantity(q) {}

    // Custom comparison for sorting formula requirements
    static bool compareForFormula(const IngredientRequirement& a, const IngredientRequirement& b) {
        if (a.quantity != b.quantity) {
            return a.quantity > b.quantity; // Descending by quantity
        }
        return a.ingredient_name < b.ingredient_name; // Ascending by name
    }
};

class EffectiveItem {
public:
    std::string name;
    EffectivenessType type;

    EffectiveItem(std::string n, EffectivenessType t) : name(std::move(n)), type(t) {}

    static bool compareByName(const EffectiveItem& a, const EffectiveItem& b) {
        return a.name < b.name;
    }
};

// Manages Geralt's ingredients, potions, and trophies
class Inventory {
private:
    std::vector<InventoryItem> ingredients_;
    std::vector<InventoryItem> potions_;
    std::vector<InventoryItem> trophies_;

    // Helper to find an item in a given item list
    InventoryItem* findItemInternal(std::vector<InventoryItem>& items, const std::string& name) {
        for (auto& item : items) {
            if (item.name == name) {
                return &item;
            }
        }
        return nullptr;
    }
    // Const version of findItemInternal
    const InventoryItem* findItemInternal(const std::vector<InventoryItem>& items, const std::string& name) const {
        for (const auto& item : items) {
            if (item.name == name) {
                return &item;
            }
        }
        return nullptr;
    }

    // Adds or updates an item's quantity in a list. If quantity becomes < 0, it's set to 0.
    void addOrUpdateItemInternal(std::vector<InventoryItem>& items, const std::string& name, int quantity_change) {
        InventoryItem* item = findItemInternal(items, name);
        if (item) {
            item->quantity += quantity_change;
            if (item->quantity < 0) item->quantity = 0; // Prevent negative quantities
        } else {
            if (quantity_change > 0 && items.size() < GameConstants::MAX_ITEMS) { // Only add if new and positive quantity
                items.emplace_back(name, quantity_change);
            }
        }
    }

    int getItemQuantityInternal(const std::vector<InventoryItem>& items, const std::string& name) const {
        const InventoryItem* item = findItemInternal(items, name);
        return item ? item->quantity : 0;
    }

    // Tries to use (decrement) an item's quantity. Returns true if successful.
    bool useItemInternal(std::vector<InventoryItem>& items, const std::string& name, int quantity_to_use) {
        if (quantity_to_use <= 0) return false;
        InventoryItem* item = findItemInternal(items, name);
        if (item && item->quantity >= quantity_to_use) {
            item->quantity -= quantity_to_use;
            return true;
        }
        return false;
    }

    // Prints all items (with quantity > 0) from a list, sorted by name.
    void printAllItemsInternal(const std::vector<InventoryItem>& items_const, const std::string& none_message) const {
        std::vector<InventoryItem> items_to_print;
        for (const auto& item : items_const) {
            if (item.quantity > 0) {
                items_to_print.push_back(item);
            }
        }

        if (items_to_print.empty()) {
            std::cout << none_message << std::endl;
            return;
        }
        std::sort(items_to_print.begin(), items_to_print.end(), InventoryItem::compareByName);
        for (size_t i = 0; i < items_to_print.size(); ++i) {
            if (i > 0) {
                std::cout << ", ";
            }
            std::cout << items_to_print[i].quantity << " " << items_to_print[i].name;
        }
        std::cout << std::endl;
    }

public:
    // Public interface for ingredients
    void addIngredient(const std::string& name, int quantity) { addOrUpdateItemInternal(ingredients_, name, quantity); }
    int getIngredientQuantity(const std::string& name) const { return getItemQuantityInternal(ingredients_, name); }
    bool useIngredient(const std::string& name, int quantity) { return useItemInternal(ingredients_, name, quantity); }
    void printAllIngredients() const { printAllItemsInternal(ingredients_, "None"); }

    // Public interface for potions
    void addPotion(const std::string& name, int quantity) { addOrUpdateItemInternal(potions_, name, quantity); }
    int getPotionQuantity(const std::string& name) const { return getItemQuantityInternal(potions_, name); }
    bool usePotion(const std::string& name, int quantity) { return useItemInternal(potions_, name, quantity); }
    void printAllPotions() const { printAllItemsInternal(potions_, "None"); }

    // Public interface for trophies
    void addTrophy(const std::string& name, int quantity) { addOrUpdateItemInternal(trophies_, name, quantity); }
    int getTrophyQuantity(const std::string& name) const { return getItemQuantityInternal(trophies_, name); }
    bool useTrophy(const std::string& name, int quantity) { return useItemInternal(trophies_, name, quantity); }
    void printAllTrophies() const { printAllItemsInternal(trophies_, "None"); }
};

// Represents a single potion formula
class PotionFormula {
public:
    std::string potion_name;
    std::vector<IngredientRequirement> requirements;

    PotionFormula(std::string name, std::vector<IngredientRequirement> reqs)
        : potion_name(std::move(name)), requirements(std::move(reqs)) {}

    // Prints the formula's requirements in a sorted format
    void print() const {
        if (requirements.empty()) {
            return; // Should not happen for a valid formula
        }
        std::vector<IngredientRequirement> sorted_reqs = requirements; // Make a copy to sort
        std::sort(sorted_reqs.begin(), sorted_reqs.end(), IngredientRequirement::compareForFormula);
        for (size_t i = 0; i < sorted_reqs.size(); ++i) {
            if (i > 0) {
                std::cout << ", ";
            }
            std::cout << sorted_reqs[i].quantity << " " << sorted_reqs[i].ingredient_name;
        }
        std::cout << std::endl;
    }
};

// Manages known potion formulae
class AlchemyBase {
private:
    std::vector<PotionFormula> formulae_;

public:
    const PotionFormula* findFormula(const std::string& potion_name) const {
        for (const auto& formula : formulae_) {
            if (formula.potion_name == potion_name) {
                return &formula;
            }
        }
        return nullptr;
    }

    // Adds a new formula. Does not check if already known; caller should handle that.
    bool addFormula(const std::string& potion_name, const std::vector<IngredientRequirement>& reqs) {
        if (formulae_.size() >= GameConstants::MAX_ITEMS) { // Check capacity
            return false; 
        }
        if (reqs.empty() || reqs.size() > GameConstants::MAX_RECIPE_INGREDIENTS) { // Validate requirements
            return false; 
        }
        formulae_.emplace_back(potion_name, reqs);
        return true;
    }

    void printFormulaForPotion(const std::string& potion_name) const {
        const PotionFormula* formula = findFormula(potion_name);
        if (formula) {
            formula->print();
        } else {
            std::cout << "No formula for " << potion_name << std::endl;
        }
    }
};

// Represents an entry in the bestiary for a single monster
class BestiaryEntry {
public:
    std::string monster_name;
    std::vector<EffectiveItem> effective_items; // Items known to be effective against this monster

    BestiaryEntry(std::string name) : monster_name(std::move(name)) {}

    bool isEffectivenessKnown(const std::string& item_name) const {
        for (const auto& eff_item : effective_items) {
            if (eff_item.name == item_name) {
                return true;
            }
        }
        return false;
    }

    // Adds a known effective item. Returns false if already known or list is full.
    bool addKnownEffectiveness(const std::string& item_name, EffectivenessType type) {
        if (isEffectivenessKnown(item_name)) { // Should ideally be checked by Bestiary class
            return false; 
        }
        if (effective_items.size() < GameConstants::MAX_EFFECTIVE_ITEMS) {
            effective_items.emplace_back(item_name, type);
            return true;
        }
        return false; // List full
    }

    // Prints all known effective items for this monster, sorted by name.
    void printEffectiveness() const {
        if (effective_items.empty()) {
            // The "No knowledge" message is handled by the Bestiary class
            return;
        }
        std::vector<EffectiveItem> sorted_items = effective_items; // Make a copy to sort
        std::sort(sorted_items.begin(), sorted_items.end(), EffectiveItem::compareByName);
        for (size_t i = 0; i < sorted_items.size(); ++i) {
            if (i > 0) {
                std::cout << ", ";
            }
            std::cout << sorted_items[i].name;
        }
        std::cout << std::endl;
    }
};

// Manages all bestiary entries
class Bestiary {
private:
    std::vector<BestiaryEntry> entries_; // List of all known monster entries

    // Helper to find a bestiary entry by monster name
    BestiaryEntry* findEntryInternal(const std::string& monster_name) {
        for (auto& entry : entries_) {
            if (entry.monster_name == monster_name) {
                return &entry;
            }
        }
        return nullptr;
    }
    // Const version of findEntryInternal
     const BestiaryEntry* findEntryInternalConst(const std::string& monster_name) const {
        for (const auto& entry : entries_) {
            if (entry.monster_name == monster_name) {
                return &entry;
            }
        }
        return nullptr;
    }

public:
    const BestiaryEntry* findEntry(const std::string& monster_name) const {
        return findEntryInternalConst(monster_name);
    }

    // Adds or updates effectiveness data for a monster.
    // Returns:
    //   2: New monster entry created & item added
    //   1: Existing monster entry updated
    //   0: Item effectiveness already known for this monster
    //  -1: Could not add (e.g., Bestiary full, or monster's effective item list full)
    int addOrUpdateEffectiveness(const std::string& monster_name, const std::string& item_name, EffectivenessType type) {
        BestiaryEntry* entry = findEntryInternal(monster_name);
        if (entry) { // Monster already exists in bestiary
            if (entry->isEffectivenessKnown(item_name)) {
                return 0; // Already known
            }
            if (entry->addKnownEffectiveness(item_name, type)) { // Try to add to existing entry
                return 1; // Existing entry updated
            } else {
                return -1; // Monster's effective items list is full
            }
        } else { // New monster
            if (entries_.size() < GameConstants::MAX_ITEMS) { // Check if Bestiary itself is full
                entries_.emplace_back(monster_name); // Create new entry for the monster
                BestiaryEntry* new_entry = &entries_.back();
                if (new_entry->addKnownEffectiveness(item_name, type)) { // Add item to the new entry
                    return 2; // New entry added, item added
                } else {
                    // This case (new_entry's list full immediately) is unlikely unless MAX_EFFECTIVE_ITEMS is 0.
                    entries_.pop_back(); // Rollback creation of empty/unusable entry
                    return -1; // Treat as an error/limit reached
                }
            } else {
                return -1; // Bestiary (list of monsters) is full
            }
        }
    }

    void printEffectivenessForMonster(const std::string& monster_name) const {
        const BestiaryEntry* entry = findEntry(monster_name);
        if (entry && !entry->effective_items.empty()) {
            entry->printEffectiveness();
        } else {
            std::cout << "No knowledge of " << monster_name << std::endl;
        }
    }
};

// Parses command strings into Parsed::Command objects
class CommandParser {
public:
    CommandParser() = default;

    Parsed::Command parse(const std::string& line_str) {
        return parse_command_internal(line_str); // Calls the C++ style internal parser
    }
};

// Main Game Application Class
class WitcherGame {
private:
    Inventory inventory_;
    AlchemyBase alchemy_base_;
    Bestiary bestiary_;
    CommandParser parser_;

    // These methods process the data from Parsed::Command objects.

    void handleLoot(const Parsed::Command& cmd) {
        // Safely get the payload using std::get_if
        if (const auto* payload = std::get_if<Parsed::LootPayload>(&cmd.data)) {
            for (const auto& item_info : payload->items) {
                inventory_.addIngredient(item_info.name, item_info.quantity);
            }
            std::cout << "Alchemy ingredients obtained" << std::endl;
        } else {
            // This should not happen if cmd.type is LOOT. Indicates a logic error.
            std::cout << "INVALID" << std::endl;
        }
    }

    void handleTrade(const Parsed::Command& cmd) {
         if (const auto* payload = std::get_if<Parsed::TradePayload>(&cmd.data)) {
            // Check if Geralt has enough trophies to trade
            bool can_trade = true;
            for (const auto& trophy_to_give : payload->trophies_to_give) {
                if (inventory_.getTrophyQuantity(trophy_to_give.name) < trophy_to_give.quantity) {
                    can_trade = false;
                    break;
                }
            }
            if (!can_trade) {
                std::cout << "Not enough trophies" << std::endl;
                return;
            }
            // Perform the trade: use trophies, add ingredients
            for (const auto& trophy_to_give : payload->trophies_to_give) {
                if (!inventory_.useTrophy(trophy_to_give.name, trophy_to_give.quantity)) {
                     // This should ideally not happen if the check above passed.
                     return; 
                }
            }
            for (const auto& ingredient_to_receive : payload->ingredients_to_receive) {
                inventory_.addIngredient(ingredient_to_receive.name, ingredient_to_receive.quantity);
            }
            std::cout << "Trade successful" << std::endl;
        } else {
            std::cout << "INVALID" << std::endl;
        }
    }

    void handleBrew(const Parsed::Command& cmd) {
        if (const auto* payload = std::get_if<Parsed::BrewPayload>(&cmd.data)) {
            const std::string& potion_name = payload->potion_name;
            const PotionFormula* formula = alchemy_base_.findFormula(potion_name);
            if (!formula) {
                std::cout << "No formula for " << potion_name << std::endl;
                return;
            }
            // Check if Geralt has all required ingredients
            bool has_ingredients = true;
            for (const auto& req : formula->requirements) {
                if (inventory_.getIngredientQuantity(req.ingredient_name) < req.quantity) {
                    has_ingredients = false;
                    break;
                }
            }
            if (!has_ingredients) {
                std::cout << "Not enough ingredients" << std::endl;
                return;
            }
            // Consume ingredients and add potion
            for (const auto& req : formula->requirements) {
               if (!inventory_.useIngredient(req.ingredient_name, req.quantity)){
                     return; 
               }
            }
            inventory_.addPotion(potion_name, 1);
            std::cout << "Alchemy item created: " << potion_name << std::endl;
        } else {
             std::cout << "INVALID" << std::endl;
        }
    }

    void handleLearnEffectiveness(const Parsed::Command& cmd) {
        if (const auto* payload = std::get_if<Parsed::LearnEffectivenessPayload>(&cmd.data)) {
            const std::string& item_name = payload->item_name;
            const std::string& monster_name = payload->monster_name;
            EffectivenessType type = payload->item_type;
            int result_code = bestiary_.addOrUpdateEffectiveness(monster_name, item_name, type);
            switch (result_code) {
                case 2: std::cout << "New bestiary entry added: " << monster_name << std::endl; break;
                case 1: std::cout << "Bestiary entry updated: " << monster_name << std::endl; break;
                case 0: std::cout << "Already known effectiveness" << std::endl; break;
                case -1: std::cout << "INVALID" << std::endl; break;
                default: std::cout << "INVALID" << std::endl; break; // Should not be hit
            }
        } else {
            std::cout << "INVALID" << std::endl;
        }
    }

    void handleLearnFormula(const Parsed::Command& cmd) {
        if (const auto* payload = std::get_if<Parsed::LearnFormulaPayload>(&cmd.data)) {
            const std::string& potion_name = payload->potion_name;
            // First, check if formula is already known
            if (alchemy_base_.findFormula(potion_name) != nullptr) {
                std::cout << "Already known formula" << std::endl;
                return;
            }

            std::vector<IngredientRequirement> reqs_cpp; // This is WitcherGame's IngredientRequirement
            // Convert Parsed::ItemInfo to IngredientRequirement
            for (const auto& parsed_req : payload->requirements) {
                reqs_cpp.emplace_back(parsed_req.name, parsed_req.quantity);
            }

            bool success = alchemy_base_.addFormula(potion_name, reqs_cpp);
            if (success) {
                std::cout << "New alchemy formula obtained: " << potion_name << std::endl;
            } else {
                std::cout << "INVALID" << std::endl;
            }
        } else {
             std::cout << "INVALID" << std::endl;
        }
    }

    void handleEncounter(const Parsed::Command& cmd) {
        if (const auto* payload = std::get_if<Parsed::EncounterPayload>(&cmd.data)) {
            const std::string& monster_name = payload->monster_name;
            const BestiaryEntry* entry = bestiary_.findEntry(monster_name);
            bool success = false;
            bool potion_to_use_on_success = false;
            std::string effective_potion_name;

            if (entry) {
                // Check signs first
                for (const auto& eff_item : entry->effective_items) {
                    if (eff_item.type == EffectivenessType::SIGN) {
                        success = true;
                        break;
                    }
                }
                // If no sign worked, check potions
                if (!success) {
                    for (const auto& eff_item : entry->effective_items) {
                        if (eff_item.type == EffectivenessType::POTION) {
                            if (inventory_.getPotionQuantity(eff_item.name) > 0) { // Check if potion is available
                                success = true;
                                potion_to_use_on_success = true;
                                effective_potion_name = eff_item.name;
                                break;
                            }
                        }
                    }
                }
            }

            if (success) {
                std::cout << "Geralt defeats " << monster_name << std::endl;
                if (potion_to_use_on_success && !effective_potion_name.empty()) {
                    if (!inventory_.usePotion(effective_potion_name, 1)) {
                         std::cout << "INVALID" << std::endl;
                    }
                }
                inventory_.addTrophy(monster_name, 1); // Add monster trophy
            } else {
                std::cout << "Geralt is unprepared and barely escapes with his life" << std::endl;
            }
        } else {
            std::cout << "INVALID" << std::endl;
        }
    }

    void handleQueryTotalSpecific(const Parsed::Command& cmd) {
        if (const auto* payload = std::get_if<Parsed::QueryTotalSpecificPayload>(&cmd.data)) {
            const std::string& category = payload->category;
            const std::string& item_name = payload->item_name;
            int quantity = 0;
            if (category == "ingredient") {
                quantity = inventory_.getIngredientQuantity(item_name);
            } else if (category == "potion") {
                quantity = inventory_.getPotionQuantity(item_name);
            } else if (category == "trophy") {
                quantity = inventory_.getTrophyQuantity(item_name);
            } else {
                std::cout << "INVALID" << std::endl; // Should be caught by parser ideally
                return;
            }
            std::cout << quantity << std::endl;
        } else {
             std::cout << "INVALID" << std::endl;
        }
    }

    void handleQueryTotalAll(const Parsed::Command& cmd) {
        if (const auto* payload = std::get_if<Parsed::QueryTotalAllPayload>(&cmd.data)) {
            const std::string& category = payload->category;
            if (category == "ingredient") {
                inventory_.printAllIngredients();
            } else if (category == "potion") {
                inventory_.printAllPotions();
            } else if (category == "trophy") {
                inventory_.printAllTrophies();
            } else {
                std::cout << "INVALID" << std::endl; // Parser should catch invalid categories
            }
        } else {
            std::cout << "INVALID" << std::endl;
        }
    }

    void handleQueryEffectiveAgainst(const Parsed::Command& cmd) {
        if (const auto* payload = std::get_if<Parsed::QueryEffectiveAgainstPayload>(&cmd.data)) {
            const std::string& monster_name = payload->monster_name;
            bestiary_.printEffectivenessForMonster(monster_name);
        } else {
            std::cout << "INVALID" << std::endl;
        }
    }

    void handleQueryWhatIsIn(const Parsed::Command& cmd) {
        if (const auto* payload = std::get_if<Parsed::QueryWhatIsInPayload>(&cmd.data)) {
            const std::string& potion_name = payload->potion_name;
            alchemy_base_.printFormulaForPotion(potion_name);
        } else {
            std::cout << "INVALID" << std::endl;
        }
    }

public:
    WitcherGame() = default;

    // Main game loop
    void run() {
        std::string line_str;
        while (true) {
            std::cout << ">> " << std::flush; // Prompt

            if (!std::getline(std::cin, line_str)) { // Read a line of input
                if (std::cin.eof()) { // End of file (e.g., Ctrl+D)
                    break; 
                }
            }

            Parsed::Command cmd = parser_.parse(line_str); // Parse the input line

            if (cmd.type == CommandType::EXIT) {
                break; // Exit the loop
            }

            // Dispatch to the appropriate handler based on command type
            switch (cmd.type) {
                case CommandType::LOOT:                  handleLoot(cmd); break;
                case CommandType::TRADE:                 handleTrade(cmd); break;
                case CommandType::BREW:                  handleBrew(cmd); break;
                case CommandType::LEARN_EFFECTIVENESS:   handleLearnEffectiveness(cmd); break;
                case CommandType::LEARN_FORMULA:         handleLearnFormula(cmd); break;
                case CommandType::ENCOUNTER:             handleEncounter(cmd); break;
                case CommandType::QUERY_TOTAL_SPECIFIC:  handleQueryTotalSpecific(cmd); break;
                case CommandType::QUERY_TOTAL_ALL:       handleQueryTotalAll(cmd); break;
                case CommandType::QUERY_EFFECTIVE_AGAINST: handleQueryEffectiveAgainst(cmd); break;
                case CommandType::QUERY_WHAT_IS_IN:      handleQueryWhatIsIn(cmd); break;
                case CommandType::EMPTY:                 break; // Do nothing for empty lines
                case CommandType::INVALID:
                default:
                    std::cout << "INVALID" << std::endl;
                    break;
            }
        }
    }
};

int main() {
    WitcherGame game;
    game.run();
    return 0; 
}