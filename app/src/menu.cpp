// =============================================================
// NIM — Menu principal (difficulté + nombre d'allumettes)
// =============================================================

#include "../includes/menu.hpp"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>

constexpr unsigned WIN_W = 1200;
constexpr unsigned WIN_H = 900;

// ── Palette (cohérente avec Interface.cpp) ──────────────────
const sf::Color M_BG{18, 16, 24};
const sf::Color M_PANEL{28, 25, 38};
const sf::Color M_AMBER{255, 183, 77};
const sf::Color M_AMBER_DIM{180, 120, 40};
const sf::Color M_CREAM{245, 235, 210};
const sf::Color M_MUTED{120, 110, 130};
const sf::Color M_RED{220, 70, 70};
const sf::Color M_SEL_BODY{255, 210, 100};

// ── Difficulté ───────────────────────────────────────────────
struct Difficulty {
  std::string label;
  std::string desc;
  int chances; // 0-100 → AI_pick
  sf::Color accent;
};

static const Difficulty DIFFICULTIES[4] = {
    {"FACILE", "L'IA joue au hasard", 25, {80, 180, 90}},
    {"MOYEN", "L'IA joue bien parfois", 50, {255, 200, 60}},
    {"DIFFICILE", "L'IA joue souvent juste", 75, {240, 130, 40}},
    {"IMPOSSIBLE", "L'IA joue toujours le meilleur", 100, {220, 60, 60}},
};

// ── Helpers ──────────────────────────────────────────────────
static void drawTextCentered(sf::RenderWindow &win, sf::Font &font,
                             const std::string &s, unsigned sz, sf::Color col,
                             float cx, float cy) {
  sf::Text t(s, font, sz);
  t.setFillColor(col);
  auto b = t.getLocalBounds();
  t.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
  t.setPosition(cx, cy);
  win.draw(t);
}

static void drawText(sf::RenderWindow &win, sf::Font &font,
                     const std::string &s, unsigned sz, sf::Color col, float x,
                     float y) {
  sf::Text t(s, font, sz);
  t.setFillColor(col);
  t.setPosition(x, y);
  win.draw(t);
}

// Returns true if mouse is inside the rect
static bool isHovered(float x, float y, float w, float h, sf::Vector2f mouse) {
  return sf::FloatRect{x, y, w, h}.contains(mouse);
}

// Draws a rounded-looking button, returns hovered state
static bool drawButton(sf::RenderWindow &win, sf::Font &font,
                       const std::string &label, float x, float y, float w,
                       float h, sf::Vector2f mouse, sf::Color fill,
                       sf::Color hoverFill, sf::Color textCol = {245, 235, 210},
                       unsigned fontSize = 17) {
  bool hov = isHovered(x, y, w, h, mouse);
  sf::RectangleShape bg({w, h});
  bg.setPosition(x, y);
  bg.setFillColor(hov ? hoverFill : fill);
  bg.setOutlineColor(M_AMBER_DIM);
  bg.setOutlineThickness(1.5f);
  win.draw(bg);
  drawTextCentered(win, font, label, fontSize, textCol, x + w / 2.f,
                   y + h / 2.f);
  return hov;
}

// ── Decorative floating matchstick ───────────────────────────
struct FloatStick {
  float x, y, vx, vy, rot, vrot, phase;
};

static void drawFloatStick(sf::RenderWindow &win, const FloatStick &fs, float t,
                           float alpha) {
  float wobble = std::sin(t * 1.2f + fs.phase) * 6.f;
  sf::Color bodyCol(180, 155, 100, (sf::Uint8)alpha);
  sf::Color headCol(180, 55, 55, (sf::Uint8)alpha);

  sf::RectangleShape body({7.f, 38.f});
  body.setOrigin(3.5f, 19.f);
  body.setPosition(fs.x, fs.y + wobble);
  body.setFillColor(bodyCol);
  body.setRotation(fs.rot + std::sin(t * 0.5f + fs.phase) * 4.f);
  win.draw(body);

  sf::CircleShape head(4.5f);
  head.setOrigin(4.5f, 4.5f);
  // head is at top of body (approx)
  float rad = (fs.rot) * 3.14159f / 180.f;
  head.setPosition(fs.x - 19.f * std::sin(rad),
                   fs.y + wobble - 19.f * std::cos(rad));
  head.setFillColor(headCol);
  win.draw(head);
}

// ── Main menu function ────────────────────────────────────────
bool run_menu(GameState &game_state) {
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Nim — Menu Principal",
                          sf::Style::Titlebar | sf::Style::Close);
  window.setFramerateLimit(60);

  sf::Font font;
  for (auto &p :
       {"/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf"}) {
    if (font.loadFromFile(p))
      break;
  }

  // ── State ────────────────────────────────────────────────
  int selectedDiff = 1; // 0=easy … 3=impossible
  int stickCount = 15;  // default
  const int STICK_MIN = 5;
  const int STICK_MAX = 50;

  // ── Decorative background sticks ─────────────────────────
  std::vector<FloatStick> bgSticks;
  {
    srand(42);
    for (int i = 0; i < 22; ++i) {
      FloatStick fs;
      fs.x = (float)(rand() % WIN_W);
      fs.y = (float)(rand() % WIN_H);
      fs.vx = ((rand() % 100) / 100.f - 0.5f) * 18.f;
      fs.vy = ((rand() % 100) / 100.f - 0.5f) * 12.f;
      fs.rot = (float)(rand() % 360);
      fs.vrot = ((rand() % 100) / 100.f - 0.5f) * 30.f;
      fs.phase = (float)(rand() % 628) / 100.f;
      bgSticks.push_back(fs);
    }
  }

  sf::Clock clock;
  float totalTime = 0.f;

  // ── Animation: title slide-in ─────────────────────────────
  float titleY = -80.f; // starts off-screen
  bool started = false;

  while (window.isOpen()) {
    float dt = clock.restart().asSeconds();
    totalTime += dt;

    sf::Vector2f mouse(sf::Mouse::getPosition(window));
    sf::Event ev;

    while (window.pollEvent(ev)) {
      if (ev.type == sf::Event::Closed) {
        window.close();
        return false;
      }

      if (ev.type == sf::Event::MouseButtonPressed &&
          ev.mouseButton.button == sf::Mouse::Left) {

        // ── Difficulty cards ──────────────────────────
        for (int i = 0; i < 4; ++i) {
          float cx = 120.f + i * 240.f;
          float cy = 340.f;
          if (isHovered(cx, cy, 200.f, 130.f, mouse))
            selectedDiff = i;
        }

        // ── Stick count buttons ───────────────────────
        // Minus
        if (isHovered(WIN_W / 2.f - 160.f, 580.f, 44.f, 44.f, mouse)) {
          if (stickCount > STICK_MIN)
            stickCount--;
        }
        // Plus
        if (isHovered(WIN_W / 2.f + 116.f, 580.f, 44.f, 44.f, mouse)) {
          if (stickCount < STICK_MAX)
            stickCount++;
        }

        // ── JOUER button ──────────────────────────────
        if (isHovered(WIN_W / 2.f - 120.f, WIN_H - 110.f, 240.f, 52.f, mouse)) {
          game_state.total_sticks = stickCount;
          game_state.max_pick = 3;
          game_state.player_turn = 1;
          game_state.ai_difficulty = DIFFICULTIES[selectedDiff].chances;
          window.close();
          return true;
        }

        // ── QUITTER button ────────────────────────────
        if (isHovered(WIN_W / 2.f - 60.f, WIN_H - 48.f, 120.f, 30.f, mouse)) {
          window.close();
          return false;
        }
      }
    }

    // ── Update background sticks ──────────────────────────
    for (auto &fs : bgSticks) {
      fs.x += fs.vx * dt;
      fs.y += fs.vy * dt;
      fs.rot += fs.vrot * dt;
      if (fs.x < -30.f)
        fs.x = WIN_W + 30.f;
      if (fs.x > WIN_W + 30.f)
        fs.x = -30.f;
      if (fs.y < -30.f)
        fs.y = WIN_H + 30.f;
      if (fs.y > WIN_H + 30.f)
        fs.y = -30.f;
    }

    // ── Animate title slide-in ────────────────────────────
    float targetY = 68.f;
    titleY += (targetY - titleY) * dt * 6.f;

    // ── Draw ─────────────────────────────────────────────
    window.clear(M_BG);

    // Subtle grid lines
    for (int i = 0; i < 23; ++i) {
      sf::RectangleShape line({(float)WIN_W, 1.f});
      line.setPosition(0.f, 40.f * i);
      line.setFillColor({255, 183, 77, 8});
      window.draw(line);
    }

    // Decorative background matchsticks (dim)
    for (auto &fs : bgSticks)
      drawFloatStick(window, fs, totalTime, 28.f);

    // ── Header panel ──────────────────────────────────────
    sf::RectangleShape header({(float)WIN_W, 110.f});
    header.setFillColor(M_PANEL);
    window.draw(header);

    // Amber accent line under header
    sf::RectangleShape accentLine({(float)WIN_W, 2.f});
    accentLine.setPosition(0.f, 110.f);
    accentLine.setFillColor(M_AMBER_DIM);
    window.draw(accentLine);

    // Title
    drawTextCentered(window, font, "N I M", 52, M_AMBER, WIN_W / 2.f, titleY);
    drawTextCentered(window, font, "JEU DES ALLUMETTES", 16, M_MUTED,
                     WIN_W / 2.f, titleY + 34.f);

    // ── Section: Difficulté ───────────────────────────────
    drawTextCentered(window, font, "DIFFICULTE", 14, M_MUTED, WIN_W / 2.f,
                     295.f);

    // Separator
    {
      float sepW = 340.f;
      sf::RectangleShape sep({sepW, 1.f});
      sep.setPosition(WIN_W / 2.f - sepW / 2.f, 308.f);
      sep.setFillColor({255, 183, 77, 40});
      window.draw(sep);
    }

    for (int i = 0; i < 4; ++i) {
      float cx = 120.f + i * 240.f;
      float cy = 340.f;
      float cw = 200.f, ch = 130.f;

      bool hov = isHovered(cx, cy, cw, ch, mouse);
      bool sel = (selectedDiff == i);
      auto &d = DIFFICULTIES[i];

      // Card background
      sf::RectangleShape card({cw, ch});
      card.setPosition(cx, cy);

      if (sel) {
        card.setFillColor({(sf::Uint8)(d.accent.r / 5),
                           (sf::Uint8)(d.accent.g / 5),
                           (sf::Uint8)(d.accent.b / 5)});
        card.setOutlineColor(d.accent);
        card.setOutlineThickness(2.5f);
      } else if (hov) {
        card.setFillColor({40, 36, 55});
        card.setOutlineColor({90, 80, 110});
        card.setOutlineThickness(1.5f);
      } else {
        card.setFillColor(M_PANEL);
        card.setOutlineColor({50, 45, 65});
        card.setOutlineThickness(1.f);
      }
      window.draw(card);

      // Colored top bar
      sf::RectangleShape topBar({cw - 4.f, 4.f});
      topBar.setPosition(cx + 2.f, cy + 2.f);
      topBar.setFillColor(sel ? d.accent : sf::Color{70, 65, 85});
      window.draw(topBar);

      // Label
      sf::Color labelCol = sel ? d.accent : (hov ? M_CREAM : M_MUTED);
      drawTextCentered(window, font, d.label, 16, labelCol, cx + cw / 2.f,
                       cy + 40.f);

      // Description
      sf::Color descCol =
          sel ? sf::Color{200, 195, 215} : sf::Color{90, 85, 100};
      drawTextCentered(window, font, d.desc, 11, descCol, cx + cw / 2.f,
                       cy + 75.f);

      // "Selected" indicator dot
      if (sel) {
        sf::CircleShape dot(5.f);
        dot.setOrigin(5.f, 5.f);
        dot.setPosition(cx + cw / 2.f, cy + ch - 18.f);
        dot.setFillColor(d.accent);
        window.draw(dot);
      }
    }

    // ── Section: Nombre d'allumettes ──────────────────────
    drawTextCentered(window, font, "NOMBRE D'ALLUMETTES", 14, M_MUTED,
                     WIN_W / 2.f, 520.f);
    {
      float sepW = 340.f;
      sf::RectangleShape sep({sepW, 1.f});
      sep.setPosition(WIN_W / 2.f - sepW / 2.f, 533.f);
      sep.setFillColor({255, 183, 77, 40});
      window.draw(sep);
    }

    // Minus button
    float btnY = 580.f;
    drawButton(
        window, font, "-", WIN_W / 2.f - 160.f, btnY, 44.f, 44.f, mouse,
        stickCount > STICK_MIN ? sf::Color{50, 45, 65} : sf::Color{30, 28, 40},
        sf::Color{80, 70, 100}, stickCount > STICK_MIN ? M_CREAM : M_MUTED, 22);

    // Count display box
    {
      sf::RectangleShape box({120.f, 44.f});
      box.setPosition(WIN_W / 2.f - 60.f, btnY);
      box.setFillColor({38, 34, 52});
      box.setOutlineColor(M_AMBER_DIM);
      box.setOutlineThickness(1.5f);
      window.draw(box);
      drawTextCentered(window, font, std::to_string(stickCount), 24, M_AMBER,
                       WIN_W / 2.f, btnY + 22.f);
    }

    // Plus button
    drawButton(
        window, font, "+", WIN_W / 2.f + 116.f, btnY, 44.f, 44.f, mouse,
        stickCount < STICK_MAX ? sf::Color{50, 45, 65} : sf::Color{30, 28, 40},
        sf::Color{80, 70, 100}, stickCount < STICK_MAX ? M_CREAM : M_MUTED, 22);

    // Range hint
    drawTextCentered(window, font,
                     "min " + std::to_string(STICK_MIN) + "  -  max " +
                         std::to_string(STICK_MAX),
                     11, M_MUTED, WIN_W / 2.f, btnY + 56.f);

    // Small stick preview row (visual feedback)
    {
      int preview = std::min(stickCount, 20);
      float gapP = std::min(32.f, (float)(WIN_W - 200) / preview);
      float totalPW = (preview - 1) * gapP;
      float startPX = WIN_W / 2.f - totalPW / 2.f;
      float previewY = btnY + 100.f;

      for (int i = 0; i < preview; ++i) {
        float px = startPX + i * gapP;
        float wobble = std::sin(totalTime * 1.8f + i * 0.4f) * 2.f;

        sf::RectangleShape body({6.f, 28.f});
        body.setOrigin(3.f, 14.f);
        body.setPosition(px, previewY + wobble);
        body.setFillColor({200, 175, 110, 180});
        window.draw(body);

        sf::CircleShape head(3.5f);
        head.setOrigin(3.5f, 3.5f);
        head.setPosition(px, previewY - 14.f + wobble);
        head.setFillColor({190, 60, 60, 200});
        window.draw(head);
      }

      if (stickCount > 20) {
        drawTextCentered(window, font,
                         "+ " + std::to_string(stickCount - 20) + " autres...",
                         11, M_MUTED, WIN_W / 2.f, previewY + 24.f);
      }
    }

    // ── JOUER button ──────────────────────────────────────
    auto &selD = DIFFICULTIES[selectedDiff];
    bool jouerhov =
        isHovered(WIN_W / 2.f - 120.f, WIN_H - 110.f, 240.f, 52.f, mouse);
    {
      sf::RectangleShape btn({240.f, 52.f});
      btn.setPosition(WIN_W / 2.f - 120.f, WIN_H - 110.f);
      btn.setFillColor(
          jouerhov ? sf::Color{(sf::Uint8)std::min(255, selD.accent.r + 30),
                               (sf::Uint8)std::min(255, selD.accent.g + 30),
                               (sf::Uint8)std::min(255, selD.accent.b + 30)}
                   : sf::Color{(sf::Uint8)(selD.accent.r * 3 / 4),
                               (sf::Uint8)(selD.accent.g * 3 / 4),
                               (sf::Uint8)(selD.accent.b * 3 / 4)});
      btn.setOutlineColor(selD.accent);
      btn.setOutlineThickness(2.f);
      window.draw(btn);
      drawTextCentered(window, font, "JOUER", 20, {245, 235, 210}, WIN_W / 2.f,
                       WIN_H - 110.f + 26.f);
    }

    // ── QUITTER link ──────────────────────────────────────
    bool qhov = isHovered(WIN_W / 2.f - 60.f, WIN_H - 48.f, 120.f, 30.f, mouse);
    drawTextCentered(window, font, "QUITTER", 13, qhov ? M_RED : M_MUTED,
                     WIN_W / 2.f, WIN_H - 33.f);

    window.display();
  }

  return false;
}