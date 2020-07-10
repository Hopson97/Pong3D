#include "Screen.h"

void ScreenStack::pushScreen(std::unique_ptr<Screen> screen)
{
    screens.push(std::move(screen));
}

void ScreenStack::popScreen(std::unique_ptr<Screen> screen)
{
    screens.pop();
}

void ScreenStack::changeScreen(std::unique_ptr<Screen> screen)
{
    while (!screens.empty()) {
        screens.pop();
    }
    pushScreen(std::move(screen));
}

Screen& ScreenStack::peekScreen()
{
    return *screens.top();
}

Screen::Screen(ScreenStack* screens)
    : m_pScreens(screens)
{
}
