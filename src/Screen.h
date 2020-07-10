#pragma once

#include <memory>
#include <stack>
#include <vector>

class Screen;

class ScreenStack {
    struct Action {
        enum class Kind {
            Push,
            Pop,
            Change,
        };
        Kind kind;
        std::unique_ptr<Screen> screen;
    };
  public:
    void pushScreen(std::unique_ptr<Screen> screen);
    void popScreen();
    void changeScreen(std::unique_ptr<Screen> screen);

    void update();

    Screen& peekScreen();

    bool hasScreen() const;


//  private:
    std::stack<std::unique_ptr<Screen>> m_screens;
    std::vector<Action> m_actions;
};

class Screen {
  public:
    Screen(ScreenStack* screens);
    virtual ~Screen() = default;
    virtual void onInput() = 0;
    virtual void onUpdate(float dt) = 0;
    virtual void onRender() = 0;

protected:
    ScreenStack* m_pScreens;
};

// Common and Handy ImGUI stuff for screens
bool imguiBeginCustom(const char* name);
bool imguiButtonCustom(const char* text);