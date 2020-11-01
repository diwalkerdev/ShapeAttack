#pragma once

#include "utility/vecref.hpp"
#include <vector>

namespace easing {

////////////////////////////////////////////////////////////////////////////////

enum class DebounceState {
    DEFAULT,
    RUNNING_GOT,
    RUNNING
};

struct DebounceData {
    int           timeout_ms;
    int           ref_count;
    int           time_ms;
    DebounceState state;
};

////////////////////////////////////////////////////////////////////////////////

struct Easer {
    std::vector<DebounceData> debouncers;

    void step(int ms)
    {
        for (auto& item : debouncers)
        {
            if (item.state != DebounceState::DEFAULT)
            {
                item.time_ms -= ms;
                if (item.time_ms <= 0)
                {
                    item.state = DebounceState::DEFAULT;
                }
            }
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

struct Debounce {
    // Ctor
    Debounce(vecref<DebounceData> item)
        : ref(item)
    {
        ++ref_count();
    }

    // Dtor
    ~Debounce()
    {
        --ref_count();
    }

    // Copy
    Debounce(Debounce const& other)
        : ref(other.ref)
    {
        ++ref_count();
    }
    auto operator=(Debounce const& other) -> Debounce&
    {
        ref = other.ref;
        ++ref_count();
        return *this;
    }

    // Move - deleted as same as copy construction.
    Debounce(Debounce&& other) = delete;
    auto operator=(Debounce&& other) -> Debounce& = delete;

    // Accessors
    auto get() -> bool
    {
        auto& data = ref.get();
        switch (data.state)
        {
        case DebounceState::RUNNING: {
            data.state = DebounceState::RUNNING_GOT;
            return true;
        }
        default: {
            return false;
        }
        }
    }

    // Modifiers
    auto set() -> bool
    {
        auto& data = ref.get();
        switch (data.state)
        {
        case DebounceState::DEFAULT: {
            data.state   = DebounceState::RUNNING;
            data.time_ms = data.timeout_ms;
            return true;
        }
        default: {
            return false;
        }
        }
    }

private:
    // You must never take a persistent copy of the reference as it
    // points to element in a vector which can resize.
    int& ref_count()
    {
        auto& data = ref.get();
        return data.ref_count;
    }

private:
    vecref<DebounceData> ref;
};

////////////////////////////////////////////////////////////////////////////////

inline auto make_debounce_switch(Easer& easer, int timeout_ms) -> Debounce
{
    DebounceData data{
        timeout_ms,
        0,
        0,
        DebounceState::DEFAULT};

    easer.debouncers.push_back(data);

    auto& vec = easer.debouncers;
    return {vecref{vec, vec.size() - 1}};
}

////////////////////////////////////////////////////////////////////////////////

}
