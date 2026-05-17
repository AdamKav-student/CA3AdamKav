#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>

struct Button
{
    sf::FloatRect bounds;
    std::string label;
    std::function<void()> onClick;
    bool hovered;

    Button(const sf::FloatRect& b, const std::string& l)
        : bounds(b), label(l), hovered(false) {}
};

class MenuManager
{
public:
    static void StaticInit();
    static std::unique_ptr<MenuManager> sInstance;

    void Render();
    void HandleMouseClick(const sf::Vector2f& mousePos);
    void HandleMouseMove(const sf::Vector2f& mousePos);

    void AddButton(const sf::FloatRect& bounds, const std::string& label, std::function<void()> onClick);
    void SetButtonCallback(const std::string& label, std::function<void()> onClick);

private:
    MenuManager();

    std::vector<Button> mButtons;
    sf::Color mButtonColor;
    sf::Color mButtonHoverColor;
    sf::Color mButtonTextColor;
};
