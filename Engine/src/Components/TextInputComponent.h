#pragma once

#include <cstddef>
#include <string>
#include <string_view>

class TextInputSystem;

struct TextInputComponent
{
public:
    explicit TextInputComponent(std::size_t limit = 32)
        : maxCharacters(limit) {
    }

    bool IsEnabled() const { return enabled; }
    const std::string& GetText() const { return value; }
    std::size_t GetMaxCharacters() const { return maxCharacters; }

    void Clear() { value.clear(); }
    void SetText(std::string_view text);
    void SetMaxCharacters(std::size_t limit);

private:
    friend class TextInputSystem;

    void SetEnabled(bool on) { enabled = on; }
    bool Append(std::string_view input);
    bool Backspace();

    std::string value;
    std::size_t maxCharacters = 32;
    bool enabled = false;
};