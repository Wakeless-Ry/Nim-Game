// =============================================================
// Drawing Utilities - Helper functions for SFML rendering
// =============================================================

#include "../includes/DrawingUtils.hpp"
#include <cmath>

// Color definitions
const sf::Color BG{18, 16, 24};
const sf::Color PANEL{28, 25, 38};
const sf::Color AMBER{255, 183, 77};
const sf::Color AMBER_DIM{180, 120, 40};
const sf::Color CREAM{245, 235, 210};
const sf::Color MUTED{120, 110, 130};
const sf::Color RED_HEAD{220, 70, 70};
const sf::Color SEL_BODY{255, 210, 100};
const sf::Color SEL_HEAD{255, 140, 0};

void drawTextCentered(sf::RenderWindow &win, sf::Font &font,
                      const std::string &str, unsigned size, sf::Color col,
                      float cx, float cy) {
  sf::Text t(str, font, size);
  t.setFillColor(col);
  auto b = t.getLocalBounds();
  t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
  t.setPosition(cx, cy);
  win.draw(t);
}

void drawStick(sf::RenderWindow &win, const Stick &s) {
  if (s.taken)
    return;
  float offY = std::sin(s.wobble) * 3.f;
  float tilt = 2.f * std::sin(s.wobble * 0.7f);
  sf::Color bodyCol = s.selected ? SEL_BODY : s.hovered ? AMBER : CREAM;
  sf::Color headCol = s.selected ? SEL_HEAD : RED_HEAD;

  sf::RectangleShape body({10.f, 52.f});
  body.setOrigin(5.f, 26.f);
  body.setPosition(s.pos.x, s.pos.y + offY);
  body.setFillColor(bodyCol);
  body.setRotation(tilt);
  win.draw(body);

  sf::CircleShape head(6.f);
  head.setOrigin(6.f, 6.f);
  head.setPosition(s.pos.x, s.pos.y - 26.f + offY);
  head.setFillColor(headCol);
  win.draw(head);
}

bool drawButton(sf::RenderWindow &win, sf::Font &font, const std::string &label,
                float x, float y, float w, float h, sf::Vector2f mouse,
                sf::Color fill, sf::Color hoverFill, sf::Color textCol) {
  bool hov = sf::FloatRect{x, y, w, h}.contains(mouse);
  sf::RectangleShape bg({w, h});
  bg.setPosition(x, y);
  bg.setFillColor(hov ? hoverFill : fill);
  bg.setOutlineColor(AMBER_DIM);
  bg.setOutlineThickness(1.5f);
  win.draw(bg);
  drawTextCentered(win, font, label, 17, textCol, x + w / 2.f, y + h / 2.f);
  return hov;
}
