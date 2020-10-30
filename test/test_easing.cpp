#include "easing/core.hpp"
#include "utility/vecref.hpp"
#include <cassert>
#include <stdio.h>
#include <vector>

////////////////////////////////////////////////////////////////////////////////

void test_make_debounce_switch_creates_one_element()
{
    easing::Easer easer;
    auto  debounce = make_debounce_switch(easer, 10);

    assert(easer.debouncers.size() == 1);
    assert(easer.debouncers[0].timeout_ms == 10);
}

////////////////////////////////////////////////////////////////////////////////

void test_get_set_and_step_example()
{
    easing::Easer easer;
    auto          debounce = easing::make_debounce_switch(easer, 20);

    // Starts in default state.
    assert(easer.debouncers[0].state == easing::DebounceState::DEFAULT);

    // Set changes state to running.
    debounce.set();
    assert(easer.debouncers[0].state == easing::DebounceState::RUNNING);

    // Stepping decreases time.
    easer.step(10);
    assert(easer.debouncers[0].time_ms == 10);

    // Getting after set returns true.
    auto value = debounce.get();
    assert(easer.debouncers[0].state == easing::DebounceState::RUNNING_GOT);
    assert(value);

    // Getting for a second time returns false.
    auto value_2nd = debounce.get();
    assert(easer.debouncers[0].state == easing::DebounceState::RUNNING_GOT);
    assert(!value_2nd);

    // When time reaches zero enters default state.
    easer.step(10);
    assert(easer.debouncers[0].time_ms == 0);
    assert(easer.debouncers[0].state == easing::DebounceState::DEFAULT);

    // check does not decrement further.
    easer.step(100);
    assert(easer.debouncers[0].time_ms == 0);
}

////////////////////////////////////////////////////////////////////////////////

#ifdef TEST_EASING
int main()
{
    test_make_debounce_switch_creates_one_element();
    test_get_set_and_step_example();
    printf("Test easing complete.\n");
}
#endif
