#ifndef DRAWING_UTILS_HPP
#define DRAWING_UTILS_HPP

#include <SFML/Graphics.hpp>
#include <string>

// Window dimensions
inline constexpr unsigned WIN_W = 1200;
inline constexpr unsigned WIN_H = 900;

// Color constants
extern const sf::Color BG;
extern const sf::Color PANEL;
extern const sf::Color AMBER;
extern const sf::Color AMBER_DIM;
extern const sf::Color CREAM;
extern const sf::Color MUTED;
extern const sf::Color RED_HEAD;
extern const sf::Color SEL_BODY;
extern const sf::Color SEL_HEAD;

// Game stick structure
struct Stick {
  sf::Vector2f pos;
  bool taken = false;
  bool hovered = false;
  bool selected = false;
  float wobble = 0.f;
};

// Helper drawing functions
void drawTextCentered(sf::RenderWindow &win, sf::Font &font,
                      const std::string &str, unsigned size, sf::Color col,
                      float cx, float cy);

void drawStick(sf::RenderWindow &win, const Stick &s);

bool drawButton(sf::RenderWindow &win, sf::Font &font, const std::string &label,
                float x, float y, float w, float h, sf::Vector2f mouse,
                sf::Color fill, sf::Color hoverFill,
                sf::Color textCol = sf::Color(245, 235, 210));

#endif
