#include "RoboCatClientPCH.hpp"

std::unique_ptr<MenuManager> MenuManager::sInstance;

MenuManager::MenuManager()
    : mButtonColor(100, 100, 200, 255)
    , mButtonHoverColor(150, 150, 255, 255)
    , mButtonTextColor(255, 255, 255, 255)
{
    // Buttons will be added after initialization
}

void MenuManager::StaticInit()
{
    sInstance.reset(new MenuManager());
}

void MenuManager::AddButton(const sf::FloatRect& bounds, const std::string& label, std::function<void()> onClick)
{
    Button button(bounds, label);
    button.onClick = onClick;
    mButtons.push_back(button);
}

void MenuManager::HandleMouseMove(const sf::Vector2f& mousePos)
{
    for (auto& button : mButtons)
    {
        button.hovered = button.bounds.contains(mousePos);
    }
}

void MenuManager::HandleMouseClick(const sf::Vector2f& mousePos)
{
    for (auto& button : mButtons)
    {
        if (button.bounds.contains(mousePos))
        {
            button.onClick();
        }
    }
}

void MenuManager::Render()
{
    // Render title
    sf::Text titleText;
    titleText.setString("Armoured Warfare 1944");
    titleText.setCharacterSize(80);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(250, 80);

    if (FontManager::sInstance)
    {
        auto font = FontManager::sInstance->GetFont("BOMBARDMENT");
        if (font)
        {
            titleText.setFont(*font);
        }
    }

    WindowManager::sInstance->draw(titleText);

    // Render buttons
    for (auto& button : mButtons)
    {
        sf::RectangleShape buttonShape(sf::Vector2f(button.bounds.width, button.bounds.height));
        buttonShape.setPosition(button.bounds.left, button.bounds.top);
        buttonShape.setFillColor(button.hovered ? mButtonHoverColor : mButtonColor);
        WindowManager::sInstance->draw(buttonShape);

        // Render button text
        sf::Text buttonText;
        buttonText.setString(button.label);
        buttonText.setCharacterSize(40);
        buttonText.setFillColor(mButtonTextColor);

        if (FontManager::sInstance)
        {
            auto font = FontManager::sInstance->GetFont("BOMBARDMENT");
            if (font)
            {
                buttonText.setFont(*font);
            }
        }

        // Center text in button
        sf::FloatRect textBounds = buttonText.getLocalBounds();
        buttonText.setPosition(
            button.bounds.left + (button.bounds.width - textBounds.width) / 2.0f,
            button.bounds.top + (button.bounds.height - textBounds.height) / 2.0f - 10.0f
        );
        WindowManager::sInstance->draw(buttonText);
    }
}

void MenuManager::SetButtonCallback(const std::string& label, std::function<void()> onClick)
{
    for (auto& button : mButtons)
    {
        if (button.label == label)
        {
            button.onClick = onClick;
            return;
        }
    }
    // optionally: add a new button if not found
    // AddButton(..., label, onClick);
}
