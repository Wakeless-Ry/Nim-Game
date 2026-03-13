// =============================================================
//  NIM — Single Row Interface (SFML 2.5+)
//  Build: g++ nim_interface.cpp -o nim -lsfml-graphics -lsfml-window
//  -lsfml-system
// =============================================================

#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>

// ─── Window ───────────────────────────────────
constexpr unsigned WIN_W = 1200;
constexpr unsigned WIN_H = 900;

// ─── Palette ──────────────────────────────────
const sf::Color BG{18, 16, 24};
const sf::Color PANEL{28, 25, 38};
const sf::Color AMBER{255, 183, 77};
const sf::Color AMBER_DIM{180, 120, 40};
const sf::Color CREAM{245, 235, 210};
const sf::Color MUTED{120, 110, 130};
const sf::Color RED_HEAD{220, 70, 70};
const sf::Color SEL_BODY{255, 210, 100};
const sf::Color SEL_HEAD{255, 140, 0};

// ─── Stick ────────────────────────────────────
struct Stick {
  sf::Vector2f pos;
  bool taken = false;
  bool hovered = false;
  bool selected = false;
  float wobble = 0.f;
};

// ─── Helpers ──────────────────────────────────

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

  // Body
  sf::RectangleShape body({10.f, 52.f});
  body.setOrigin(5.f, 26.f);
  body.setPosition(s.pos.x, s.pos.y + offY);
  body.setFillColor(bodyCol);
  body.setRotation(tilt);
  win.draw(body);

  // Head
  sf::CircleShape head(6.f);
  head.setOrigin(6.f, 6.f);
  head.setPosition(s.pos.x, s.pos.y - 26.f + offY);
  head.setFillColor(headCol);
  win.draw(head);
}

// Simple button — returns true if hovered
bool drawButton(sf::RenderWindow &win, sf::Font &font, const std::string &label,
                float x, float y, float w, float h, sf::Vector2f mouse,
                sf::Color fill, sf::Color hoverFill,
                sf::Color textCol = CREAM) {
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

// ─── Main ─────────────────────────────────────
int main() {
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Nim — Interface",
                          sf::Style::Titlebar | sf::Style::Close);
  window.setFramerateLimit(60);

  // Font loading
  sf::Font font;
  for (auto &p :
       {"/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
        "/System/Library/Fonts/Helvetica.ttc", "C:/Windows/Fonts/arial.ttf"}) {
    if (font.loadFromFile(p))
      break;
  }

  // ─── State ───────────────────────────────
  const int TOTAL_STICKS = 20;
  std::vector<Stick> sticks(TOTAL_STICKS);
  int selCount = 0;

  // Layout sticks centred horizontally
  auto layoutSticks = [&]() {
    const float gap = 46.f;
    const float totalW = (TOTAL_STICKS - 1) * gap;
    const float startX = WIN_W / 2.f - totalW / 2.f;
    const float y = WIN_H / 2.f - 10.f;
    for (int i = 0; i < TOTAL_STICKS; ++i) {
      sticks[i].pos = {startX + i * gap, y};
      sticks[i].taken = false;
      sticks[i].hovered = false;
      sticks[i].selected = false;
      sticks[i].wobble = 0.f;
    }
    selCount = 0;
  };
  layoutSticks();

  // Hover flags for buttons
  bool confirmHov = false, cancelHov = false, resetHov = false;

  sf::Clock clock;

  while (window.isOpen()) {
    float dt = clock.restart().asSeconds();
    sf::Vector2f mouse(sf::Mouse::getPosition(window));

    // ─── Events ──────────────────────────
    sf::Event ev;
    while (window.pollEvent(ev)) {
      if (ev.type == sf::Event::Closed)
        window.close();

      if (ev.type == sf::Event::MouseButtonPressed &&
          ev.mouseButton.button == sf::Mouse::Left) {
        // Confirm — remove selected sticks
        if (confirmHov && selCount > 0) {
          for (auto &s : sticks)
            if (s.selected) {
              s.taken = true;
              s.selected = false;
            }
          selCount = 0;
        }
        // Cancel — deselect all
        else if (cancelHov) {
          for (auto &s : sticks)
            s.selected = false;
          selCount = 0;
        }
        // Reset
        else if (resetHov) {
          layoutSticks();
        }
        // Toggle stick selection
        else {
          for (auto &s : sticks) {
            if (s.taken)
              continue;
            float dx = mouse.x - s.pos.x;
            float dy = mouse.y - s.pos.y;
            if (std::abs(dx) < 14.f && std::abs(dy) < 34.f) {
              s.selected = !s.selected;
              selCount += s.selected ? 1 : -1;
            }
          }
        }
      }
    }

    // ─── Update ──────────────────────────
    for (auto &s : sticks) {
      if (!s.taken) {
        float dx = mouse.x - s.pos.x;
        float dy = mouse.y - s.pos.y;
        s.hovered = (std::abs(dx) < 14.f && std::abs(dy) < 34.f);
        s.wobble += dt * (s.hovered ? 5.f : 1.5f);
      }
    }

    // Count remaining
    int remaining = 0;
    for (auto &s : sticks)
      if (!s.taken)
        ++remaining;

    // ─── Draw ────────────────────────────
    window.clear(BG);

    // Subtle horizontal grid lines
    for (int i = 0; i < 13; ++i) {
      sf::RectangleShape line({(float)WIN_W, 1.f});
      line.setPosition(0.f, 40.f * i);
      line.setFillColor({255, 183, 77, 10});
      window.draw(line);
    }

    // Header bar
    sf::RectangleShape header({(float)WIN_W, 62.f});
    header.setFillColor(PANEL);
    window.draw(header);
    drawTextCentered(window, font, "N I M", 30, AMBER, WIN_W / 2.f, 31.f);

    // Remaining count badge
    std::string badge = std::to_string(remaining) + " stick" +
                        (remaining != 1 ? "s" : "") + " left";
    drawTextCentered(window, font, badge, 16, MUTED, WIN_W / 2.f, 95.f);

    // Row background panel
    sf::RectangleShape rowBg({(float)WIN_W - 160.f, 110.f});
    rowBg.setPosition(80.f, WIN_H / 2.f - 68.f);
    rowBg.setFillColor(PANEL);
    rowBg.setOutlineColor(selCount > 0 ? AMBER_DIM : sf::Color{50, 45, 65});
    rowBg.setOutlineThickness(1.5f);
    window.draw(rowBg);

    // Sticks
    for (auto &s : sticks)
      drawStick(window, s);

    // Action buttons (always visible; CONFIRM dimmed when nothing selected)
    sf::Color confirmFill =
        selCount > 0 ? sf::Color{40, 100, 60} : sf::Color{30, 50, 35};
    sf::Color confirmHov2 =
        selCount > 0 ? sf::Color{55, 140, 80} : sf::Color{30, 50, 35};

    confirmHov =
        drawButton(window, font, "CONFIRM", WIN_W / 2.f - 115.f, WIN_H - 100.f,
                   105.f, 40.f, mouse, confirmFill, confirmHov2);

    cancelHov =
        drawButton(window, font, "CANCEL", WIN_W / 2.f + 10.f, WIN_H - 100.f,
                   105.f, 40.f, mouse, {90, 40, 40}, {130, 55, 55});

    resetHov = drawButton(window, font, "RESET", WIN_W - 130.f, WIN_H - 60.f,
                          90.f, 32.f, mouse, {40, 35, 55}, {60, 50, 80}, MUTED);

    // Selection hint
    if (selCount > 0) {
      std::string hint = "Selected: " + std::to_string(selCount);
      drawTextCentered(window, font, hint, 15, SEL_BODY, WIN_W / 2.f,
                       WIN_H - 130.f);
    } else {
      drawTextCentered(window, font, "Click sticks to select, then Confirm", 14,
                       MUTED, WIN_W / 2.f, WIN_H - 130.f);
    }

    window.display();
  }

  return 0;
}