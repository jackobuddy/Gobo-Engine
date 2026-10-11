#pragma once

#include <sdl3/SDL.h>
#include <cstddef>
#include <string>
#include <string_view>

class TextInputSystem; 


struct TextInputComponent
{
public:
	explicit TextInputComponent(std::size_t limit = 32) : maxCharacters(limit) {}

    bool IsEnabled() const { return enabled; }
    const std::string& GetText() const { return value; }
    std::size_t GetMaxCharacters() const { return maxCharacters; }
    void Clear() { value.clear(); }
    void SetText(std::string_view text) { value.clear(); Append(text); }

    void SetMaxCharacters(std::size_t limit)
    {
        maxCharacters = limit;
        while (SDL_utf8strlen(value.c_str()) > maxCharacters) Backspace();
    }


private:
    friend class TextInputSystem;

    void SetEnabled(bool on) { enabled = on; }

    bool Append(std::string_view input)
    {
        const std::string copy(input);
        const char* cursor = copy.c_str();
        std::size_t remaining = copy.size();
        std::size_t count = SDL_utf8strlen(value.c_str());
        bool changed = false;
        while (remaining && count < maxCharacters)
        {
            const char* first = cursor;
            const Uint32 point = SDL_StepUTF8(&cursor, &remaining);
            if (point == 0) break;
            if (point == SDL_INVALID_UNICODE_CODEPOINT ||
                point < 0x20 || point == 0x7f) continue;
            value.append(first, static_cast<std::size_t>(cursor - first));
            ++count; changed = true;
        }
        return changed;
    }

    bool Backspace()
    {
        if (value.empty()) return false;
        const char* first = value.c_str();
        const char* cursor = first + value.size();
        SDL_StepBackUTF8(first, &cursor);
        value.resize(static_cast<std::size_t>(cursor - first));
        return true;
    }

    std::string value;
    std::size_t maxCharacters = 32;
    bool enabled = false;
};