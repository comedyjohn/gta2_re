#include "core/function.hpp"
#include <cstdio>

#include <cstdlib>
#include <string>
#include <vector>

// For C++98, we'll use these typedefs
using u32 = unsigned int;
using u8 = unsigned char;

// Structure to store one function's metadata.
struct FunctionMetadata
{
    std::string mangled_name;
    u32 func_va;
    u32 func_fo;
    u32 og_addr;
    u32 func_status;
};

// A simple trim function to remove whitespace from the beginning and end of a string.
std::string Trim(const std::string& s)
{
    std::string::size_type start = s.find_first_not_of(" \t\n\r");
    std::string::size_type end = s.find_last_not_of(" \t\n\r");
    if (start == std::string::npos)
    {
        return "";
    }
    return s.substr(start, end - start + 1);
}

// Converts a hex string (optionally starting with "0x" or "0X") into a u32.
u32 HexStringToU32(const std::string& hex_str)
{
    std::string s = Trim(hex_str);
    if (s.size() >= 2 && (s.substr(0, 2) == "0x" || s.substr(0, 2) == "0X"))
    {
        s = s.substr(2);
    }
    return static_cast<u32>(strtoul(s.c_str(), nullptr, 16));
}

// Extracts the value of a given key (assumed to be in quotes) from a JSON object string.
std::string ExtractJsonStringValue(const std::string& object_str, const std::string& key)
{
    std::string pattern = "\"" + key + "\"";
    std::string value;
    std::string::size_type key_pos = object_str.find(pattern);
    if (key_pos == std::string::npos)
    {
        return value;
    }
    // Find the colon after the key.
    std::string::size_type colon_pos = object_str.find(':', key_pos);
    if (colon_pos == std::string::npos)
    {
        return value;
    }
    // Find the starting quote for the value.
    std::string::size_type value_start = object_str.find('\"', colon_pos);
    if (value_start == std::string::npos)
    {
        return value;
    }
    ++value_start; // Skip the quote.
    // Find the ending quote.
    std::string::size_type value_end = object_str.find('\"', value_start);
    if (value_end == std::string::npos)
    {
        return value;
    }
    value = object_str.substr(value_start, value_end - value_start);
    return Trim(value);
}

// Loads and parses the JSON from file "new_data.json", reading only the specific subset.
// It expects a key "functions" with an array of objects, where each object has keys:
// "mangled_name", "func_va", "func_fo", "og_addr", and "func_status".
std::vector<FunctionMetadata> LoadFunctionMetadataFromFile(const std::string& filename)
{
    std::vector<FunctionMetadata> functions;
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp)
    {
        printf("Error opening file: %s\n", filename.c_str());
        return functions;
    }

    // Determine the file size.
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Read the entire file into a buffer.
    char* buffer = new char[fsize + 1];
    size_t read_bytes = fread(buffer, 1, fsize, fp);
    buffer[read_bytes] = '\0';
    fclose(fp);

    std::string file_content(buffer);
    delete[] buffer;

    // Locate the "functions" key in the JSON.
    std::string::size_type functions_pos = file_content.find("\"functions\"");
    if (functions_pos == std::string::npos)
    {
        printf("No \"functions\" key found.\n");
        return functions;
    }
    // Locate the start of the array '[' following the "functions" key.
    std::string::size_type array_start = file_content.find('[', functions_pos);
    if (array_start == std::string::npos)
    {
        printf("No array '[' found after \"functions\" key.\n");
        return functions;
    }
    // Start searching from just after the '['.
    std::string::size_type pos = array_start + 1;

    // Loop through each JSON object in the array.
    while (true)
    {
        std::string::size_type obj_start = file_content.find('{', pos);
        if (obj_start == std::string::npos)
        {
            break;
        }
        std::string::size_type obj_end = file_content.find('}', obj_start);
        if (obj_end == std::string::npos)
        {
            break;
        }

        // Extract the JSON object text.
        std::string obj_text = file_content.substr(obj_start, obj_end - obj_start + 1);

        // Extract all required values.
        std::string mangled_name = ExtractJsonStringValue(obj_text, "mangled_name");
        std::string func_va_str = ExtractJsonStringValue(obj_text, "func_va");
        std::string func_fo_str = ExtractJsonStringValue(obj_text, "func_fo");
        std::string og_addr_str = ExtractJsonStringValue(obj_text, "og_addr");
        std::string func_status_str = ExtractJsonStringValue(obj_text, "func_status");

        // If every field is non-empty, then convert the hex strings to u32 values and add the metadata.
        if (!mangled_name.empty() && !func_va_str.empty() && !func_fo_str.empty() && !og_addr_str.empty() && !func_status_str.empty())
        {
            FunctionMetadata meta;
            meta.mangled_name = mangled_name;
            meta.func_va = HexStringToU32(func_va_str);
            meta.func_fo = HexStringToU32(func_fo_str);
            meta.og_addr = HexStringToU32(og_addr_str);
            meta.func_status = HexStringToU32(func_status_str);
            functions.push_back(meta);
        }

        pos = obj_end + 1;
        std::string::size_type array_end = file_content.find(']', pos);
        if (array_end != std::string::npos && pos > array_end)
        {
            break;
        }
    }
    return functions;
}

// Searches backward from the given code address for a metadata signature.
// The assumed signature is: two NOPs (0x90, 0x90) followed by a MOV EAX immediate opcode (0xB8).
// If found, returns the address (as u32) where the pattern starts; otherwise returns 0.
namespace
{
u32 FindFuncMetaData(u32 code_addr)
{
    // Search backwards up to 10 bytes.
    for (u32 offset = 1; offset <= 100; ++offset)
    {
        u32 candidate_addr = code_addr - offset;
        // Cast the candidate address into a pointer to bytes.
        const u8* candidate = reinterpret_cast<const u8*>(candidate_addr);
        // Check for the pattern.
        // (Note: Make sure that reading these 3 bytes is safe in your environment.)
        if (candidate[0] == 0x90 && candidate[1] == 0x90 && candidate[2] == 0xB8)
        {
            return (candidate[3]) | (candidate[4] << 8) | (candidate[5] << 16) | (candidate[6] << 24);
        }
    }
    return 0; // Not found.
}
} // namespace

void LogFuncAddr(u32 code_addr, s32 mode)
{
    const u32 og_addr = FindFuncMetaData(code_addr);
    if (og_addr == 0)
    {
        printf("CANT FIND OG ADDR!!!! 0x%X\n", code_addr);
    }
    else
    {
        static std::vector<FunctionMetadata> functions = LoadFunctionMetadataFromFile("new_data.json");
        for (auto& function : functions)
        {
            if (function.og_addr == og_addr)
            {
                if (mode == 0)
                {
                    printf("NOT IMPLEMENTED: %s (0x%X)\n", function.mangled_name.c_str(), og_addr);
                }
                else
                {
                    printf("WIP IMPLEMENTED: %s (0x%X)\n", function.mangled_name.c_str(), og_addr);
                }
                return;
            }
        }
        if (mode == 0)
        {
            printf("NOT IMPLEMENTED: (can't get func name) 0x%X\n", og_addr);
        }
        else
        {
            printf("WIP IMPLEMENTED: (can't get func name) 0x%X\n", og_addr);
        }
    }
}
