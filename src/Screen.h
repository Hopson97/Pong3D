#pragma once

#include <memory>
#include <stack>

class Screen;

class ScreenStack {
  public:
   
    void pushScreen(std::unique_ptr<Screen> screen);
    void popScreen(std::unique_ptr<Screen> screen);
    void changeScreen(std::unique_ptr<Screen> screen);

    Screen& peekScreen();


  private:
    std::stack<std::unique_ptr<Screen>> screens;
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