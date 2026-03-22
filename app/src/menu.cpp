// =============================================================
// NIM — Menu principal (stratégie + difficulté + nombre d'allumettes)
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

// ── Stratégies ────────────────────────────────────────────────
struct Strategy {
  std::string label;
  std::string desc;
  sf::Color   accent;
};

static const Strategy STRATEGIES[5] = {
    {"MIXTE",   "Probabiliste",    {90,  150, 230}},
    {"COPIE",   "Copie vos coups", {160, 110, 220}},
    {"MINIMAX", "Negamax borne",   {70,  190, 140}},
    {"MCTS",    "Monte Carlo",     {250, 155, 50}},
    {"RECUIT",  "Recuit simule",   {220, 70,  70}},
};

// ── Difficulté ───────────────────────────────────────────────
struct Difficulty {
  std::string label;
  std::string desc;
  int         chances;
  sf::Color   accent;
};

static const Difficulty DIFFICULTIES[4] = {
    {"FACILE",     "Niveau faible",   25,  {80,  180, 90}},
    {"MOYEN",      "Niveau moyen",    50,  {255, 200, 60}},
    {"DIFFICILE",  "Niveau eleve",    75,  {240, 130, 40}},
    {"IMPOSSIBLE", "Niveau maximal",  100, {220, 60,  60}},
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

static bool isHovered(float x, float y, float w, float h, sf::Vector2f mouse) {
  return sf::FloatRect{x, y, w, h}.contains(mouse);
}

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

static void drawFloatStick(sf::RenderWindow &win, const FloatStick &fs,
                           float t, float alpha) {
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
  float rad = fs.rot * 3.14159f / 180.f;
  head.setPosition(fs.x - 19.f * std::sin(rad),
                   fs.y + wobble - 19.f * std::cos(rad));
  head.setFillColor(headCol);
  win.draw(head);
}

// ── Helper: dessine une section de cartes générique ──────────
static void drawSectionSeparator(sf::RenderWindow &win, sf::Font &font,
                                 const std::string &title, float y) {
  drawTextCentered(win, font, title, 12, M_MUTED, WIN_W / 2.f, y);
  float sepW = 380.f;
  sf::RectangleShape sep({sepW, 1.f});
  sep.setPosition(WIN_W / 2.f - sepW / 2.f, y + 11.f);
  sep.setFillColor({255, 183, 77, 35});
  win.draw(sep);
}

// ── Main menu function ────────────────────────────────────────
bool run_menu(GameState &game_state) {
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Nim - Menu Principal",
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
  int selectedStrat = 0; // 0=MIXTE … 4=RECUIT
  int selectedDiff  = 1; // 0=FACILE … 3=IMPOSSIBLE
  int stickCount    = 20;
  const int STICK_MIN = 3;
  const int STICK_MAX = 50;

  // ── Decorative background sticks ─────────────────────────
  std::vector<FloatStick> bgSticks;
  {
    srand(42);
    for (int i = 0; i < 22; ++i) {
      FloatStick fs;
      fs.x    = (float)(rand() % WIN_W);
      fs.y    = (float)(rand() % WIN_H);
      fs.vx   = ((rand() % 100) / 100.f - 0.5f) * 18.f;
      fs.vy   = ((rand() % 100) / 100.f - 0.5f) * 12.f;
      fs.rot  = (float)(rand() % 360);
      fs.vrot = ((rand() % 100) / 100.f - 0.5f) * 30.f;
      fs.phase = (float)(rand() % 628) / 100.f;
      bgSticks.push_back(fs);
    }
  }

  sf::Clock clock;
  float totalTime = 0.f;
  float titleY = -80.f;

  // ── Layout constants ─────────────────────────────────────
  // Strategy cards row
  const float STRAT_LABEL_Y  = 122.f;
  const float STRAT_CARD_Y   = 136.f;
  const float STRAT_CARD_W   = 180.f;
  const float STRAT_CARD_H   = 100.f;
  const float STRAT_CARD_GAP = 15.f;
  const float STRAT_TOTAL_W  = 5 * STRAT_CARD_W + 4 * STRAT_CARD_GAP;
  const float STRAT_START_X  = (WIN_W - STRAT_TOTAL_W) / 2.f;

  // Difficulty cards row
  const float DIFF_LABEL_Y = 252.f;
  const float DIFF_CARD_Y  = 266.f;
  const float DIFF_CARD_W  = 200.f;
  const float DIFF_CARD_H  = 105.f;
  const float DIFF_CARD_GAP = 60.f; // gap between cards
  const float DIFF_START_X  = (WIN_W - (4 * DIFF_CARD_W + 3 * DIFF_CARD_GAP)) / 2.f;

  // Stick count controls
  const float SC_LABEL_Y = 385.f;
  const float SC_BTN_Y   = 400.f;

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

        // ── Strategy cards ────────────────────────────
        for (int i = 0; i < 5; ++i) {
          float cx = STRAT_START_X + i * (STRAT_CARD_W + STRAT_CARD_GAP);
          if (isHovered(cx, STRAT_CARD_Y, STRAT_CARD_W, STRAT_CARD_H, mouse))
            selectedStrat = i;
        }

        // ── Difficulty cards ──────────────────────────
        for (int i = 0; i < 4; ++i) {
          float cx = DIFF_START_X + i * (DIFF_CARD_W + DIFF_CARD_GAP);
          if (isHovered(cx, DIFF_CARD_Y, DIFF_CARD_W, DIFF_CARD_H, mouse))
            selectedDiff = i;
        }

        // ── Stick count −/+ ───────────────────────────
        if (isHovered(WIN_W / 2.f - 110.f, SC_BTN_Y, 44.f, 44.f, mouse)) {
          if (stickCount > STICK_MIN) stickCount--;
        }
        if (isHovered(WIN_W / 2.f + 66.f, SC_BTN_Y, 44.f, 44.f, mouse)) {
          if (stickCount < STICK_MAX) stickCount++;
        }

        // ── JOUER ─────────────────────────────────────
        if (isHovered(WIN_W / 2.f - 120.f, WIN_H - 110.f, 240.f, 52.f, mouse)) {
          game_state.total_sticks  = stickCount;
          game_state.max_pick      = 3;
          game_state.player_turn   = 1;
          game_state.ai_difficulty = DIFFICULTIES[selectedDiff].chances;
          game_state.ai_strategy   = (AIStrategyType)selectedStrat;
          window.close();
          return true;
        }

        // ── QUITTER ───────────────────────────────────
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
      if (fs.x < -30.f) fs.x = WIN_W + 30.f;
      if (fs.x > WIN_W + 30.f) fs.x = -30.f;
      if (fs.y < -30.f) fs.y = WIN_H + 30.f;
      if (fs.y > WIN_H + 30.f) fs.y = -30.f;
    }

    float targetY = 62.f;
    titleY += (targetY - titleY) * dt * 6.f;

    // ── Draw ─────────────────────────────────────────────
    window.clear(M_BG);

    for (int i = 0; i < 23; ++i) {
      sf::RectangleShape line({(float)WIN_W, 1.f});
      line.setPosition(0.f, 40.f * i);
      line.setFillColor({255, 183, 77, 8});
      window.draw(line);
    }

    for (auto &fs : bgSticks)
      drawFloatStick(window, fs, totalTime, 28.f);

    // ── Header ────────────────────────────────────────────
    sf::RectangleShape header({(float)WIN_W, 110.f});
    header.setFillColor(M_PANEL);
    window.draw(header);

    sf::RectangleShape accentLine({(float)WIN_W, 2.f});
    accentLine.setPosition(0.f, 110.f);
    accentLine.setFillColor(M_AMBER_DIM);
    window.draw(accentLine);

    drawTextCentered(window, font, "N I M", 48, M_AMBER, WIN_W / 2.f, titleY);
    drawTextCentered(window, font, "JEU DES ALLUMETTES", 14, M_MUTED,
                     WIN_W / 2.f, titleY + 30.f);

    // ── Section STRATEGIE ─────────────────────────────────
    drawSectionSeparator(window, font, "STRATEGIE DE L'IA", STRAT_LABEL_Y);

    for (int i = 0; i < 5; ++i) {
      float cx = STRAT_START_X + i * (STRAT_CARD_W + STRAT_CARD_GAP);
      float cy = STRAT_CARD_Y;
      bool hov = isHovered(cx, cy, STRAT_CARD_W, STRAT_CARD_H, mouse);
      bool sel = (selectedStrat == i);
      auto &s  = STRATEGIES[i];

      sf::RectangleShape card({STRAT_CARD_W, STRAT_CARD_H});
      card.setPosition(cx, cy);
      if (sel) {
        card.setFillColor({(sf::Uint8)(s.accent.r / 6),
                           (sf::Uint8)(s.accent.g / 6),
                           (sf::Uint8)(s.accent.b / 6)});
        card.setOutlineColor(s.accent);
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

      sf::RectangleShape topBar({STRAT_CARD_W - 4.f, 3.f});
      topBar.setPosition(cx + 2.f, cy + 2.f);
      topBar.setFillColor(sel ? s.accent : sf::Color{70, 65, 85});
      window.draw(topBar);

      sf::Color labelCol = sel ? s.accent : (hov ? M_CREAM : M_MUTED);
      drawTextCentered(window, font, s.label, 14, labelCol,
                       cx + STRAT_CARD_W / 2.f, cy + 34.f);
      sf::Color descCol = sel ? sf::Color{200, 195, 215} : sf::Color{80, 75, 95};
      drawTextCentered(window, font, s.desc, 10, descCol,
                       cx + STRAT_CARD_W / 2.f, cy + 62.f);

      if (sel) {
        sf::CircleShape dot(4.f);
        dot.setOrigin(4.f, 4.f);
        dot.setPosition(cx + STRAT_CARD_W / 2.f, cy + STRAT_CARD_H - 14.f);
        dot.setFillColor(s.accent);
        window.draw(dot);
      }
    }

    // ── Section DIFFICULTE ────────────────────────────────
    // Label de difficulté avec info stratégie-dépendante
    {
      std::string diffDetail;
      switch (selectedStrat) {
        case 0: diffDetail = "  (% de coups optimaux)";  break;
        case 1: diffDetail = "  (non applicable)";       break;
        case 2: diffDetail = "  (profondeur Negamax)";   break;
        case 3: diffDetail = "  (nb de simulations)";    break;
        case 4: diffDetail = "  (inverse temperature)";  break;
      }
      // Redraw the separator avec le détail
      drawTextCentered(window, font, "DIFFICULTE" + diffDetail, 12, M_MUTED,
                       WIN_W / 2.f, DIFF_LABEL_Y);
      float sepW = 500.f;
      sf::RectangleShape sep({sepW, 1.f});
      sep.setPosition(WIN_W / 2.f - sepW / 2.f, DIFF_LABEL_Y + 11.f);
      sep.setFillColor({255, 183, 77, 35});
      window.draw(sep);
    }

    for (int i = 0; i < 4; ++i) {
      float cx = DIFF_START_X + i * (DIFF_CARD_W + DIFF_CARD_GAP);
      float cy = DIFF_CARD_Y;
      bool hov = isHovered(cx, cy, DIFF_CARD_W, DIFF_CARD_H, mouse);
      bool sel = (selectedDiff == i);
      bool grayed = (selectedStrat == 1); // COPIE: difficulty unused
      auto &d = DIFFICULTIES[i];

      sf::Color cardAccent = grayed ? sf::Color{60, 55, 75} : d.accent;

      sf::RectangleShape card({DIFF_CARD_W, DIFF_CARD_H});
      card.setPosition(cx, cy);
      if (sel && !grayed) {
        card.setFillColor({(sf::Uint8)(d.accent.r / 5),
                           (sf::Uint8)(d.accent.g / 5),
                           (sf::Uint8)(d.accent.b / 5)});
        card.setOutlineColor(d.accent);
        card.setOutlineThickness(2.5f);
      } else if (hov && !grayed) {
        card.setFillColor({40, 36, 55});
        card.setOutlineColor({90, 80, 110});
        card.setOutlineThickness(1.5f);
      } else {
        card.setFillColor(M_PANEL);
        card.setOutlineColor(sf::Color{50, 45, (sf::Uint8)(grayed ? 55 : 65)});
        card.setOutlineThickness(1.f);
      }
      window.draw(card);

      sf::RectangleShape topBar({DIFF_CARD_W - 4.f, 4.f});
      topBar.setPosition(cx + 2.f, cy + 2.f);
      topBar.setFillColor((sel && !grayed) ? d.accent : sf::Color{70, 65, 85});
      window.draw(topBar);

      sf::Color labelCol = (sel && !grayed) ? d.accent
                         : (hov && !grayed) ? M_CREAM : M_MUTED;
      drawTextCentered(window, font, d.label, 14, labelCol,
                       cx + DIFF_CARD_W / 2.f, cy + 35.f);

      sf::Color descCol = (sel && !grayed) ? sf::Color{200, 195, 215}
                                           : sf::Color{80, 75, 95};
      drawTextCentered(window, font, d.desc, 10, descCol,
                       cx + DIFF_CARD_W / 2.f, cy + 68.f);

      if (sel && !grayed) {
        sf::CircleShape dot(5.f);
        dot.setOrigin(5.f, 5.f);
        dot.setPosition(cx + DIFF_CARD_W / 2.f, cy + DIFF_CARD_H - 14.f);
        dot.setFillColor(d.accent);
        window.draw(dot);
      }
    }

    // ── Section NOMBRE D'ALLUMETTES ───────────────────────
    drawSectionSeparator(window, font, "NOMBRE D'ALLUMETTES", SC_LABEL_Y);

    float btnY = SC_BTN_Y;

    drawButton(window, font, "-", WIN_W / 2.f - 110.f, btnY, 44.f, 44.f, mouse,
               stickCount > STICK_MIN ? sf::Color{50, 45, 65}
                                      : sf::Color{30, 28, 40},
               sf::Color{80, 70, 100},
               stickCount > STICK_MIN ? M_CREAM : M_MUTED, 22);

    {
      sf::RectangleShape box({120.f, 44.f});
      box.setPosition(WIN_W / 2.f - 60.f, btnY);
      box.setFillColor({38, 34, 52});
      box.setOutlineColor(M_AMBER_DIM);
      box.setOutlineThickness(1.5f);
      window.draw(box);
      drawTextCentered(window, font, std::to_string(stickCount), 22, M_AMBER,
                       WIN_W / 2.f, btnY + 22.f);
    }

    drawButton(window, font, "+", WIN_W / 2.f + 66.f, btnY, 44.f, 44.f, mouse,
               stickCount < STICK_MAX ? sf::Color{50, 45, 65}
                                      : sf::Color{30, 28, 40},
               sf::Color{80, 70, 100},
               stickCount < STICK_MAX ? M_CREAM : M_MUTED, 22);

    drawTextCentered(window, font,
                     "min " + std::to_string(STICK_MIN) + "  -  max " +
                         std::to_string(STICK_MAX),
                     10, M_MUTED, WIN_W / 2.f, btnY + 55.f);

    // Preview sticks
    {
      int   preview  = std::min(stickCount, 20);
      float gapP     = std::min(32.f, (float)(WIN_W - 200) / preview);
      float totalPW  = (preview - 1) * gapP;
      float startPX  = WIN_W / 2.f - totalPW / 2.f;
      float previewY = btnY + 80.f;

      for (int i = 0; i < preview; ++i) {
        float px     = startPX + i * gapP;
        float wobble = std::sin(totalTime * 1.8f + i * 0.4f) * 2.f;

        sf::RectangleShape body({6.f, 26.f});
        body.setOrigin(3.f, 13.f);
        body.setPosition(px, previewY + wobble);
        body.setFillColor({200, 175, 110, 180});
        window.draw(body);

        sf::CircleShape head(3.5f);
        head.setOrigin(3.5f, 3.5f);
        head.setPosition(px, previewY - 13.f + wobble);
        head.setFillColor({190, 60, 60, 200});
        window.draw(head);
      }
      if (stickCount > 20) {
        drawTextCentered(window, font,
                         "+ " + std::to_string(stickCount - 20) + " autres...",
                         10, M_MUTED, WIN_W / 2.f, previewY + 22.f);
      }
    }

    // ── JOUER button ──────────────────────────────────────
    {
      auto &selS  = STRATEGIES[selectedStrat];
      bool jouerhov =
          isHovered(WIN_W / 2.f - 120.f, WIN_H - 110.f, 240.f, 52.f, mouse);
      sf::RectangleShape btn({240.f, 52.f});
      btn.setPosition(WIN_W / 2.f - 120.f, WIN_H - 110.f);
      sf::Color base = selS.accent;
      btn.setFillColor(
          jouerhov
              ? sf::Color{(sf::Uint8)std::min(255, base.r + 30),
                          (sf::Uint8)std::min(255, base.g + 30),
                          (sf::Uint8)std::min(255, base.b + 30)}
              : sf::Color{(sf::Uint8)(base.r * 3 / 4),
                          (sf::Uint8)(base.g * 3 / 4),
                          (sf::Uint8)(base.b * 3 / 4)});
      btn.setOutlineColor(base);
      btn.setOutlineThickness(2.f);
      window.draw(btn);
      drawTextCentered(window, font, "JOUER", 20, M_CREAM, WIN_W / 2.f,
                       WIN_H - 110.f + 26.f);
    }

    // ── QUITTER link ──────────────────────────────────────
    bool qhov =
        isHovered(WIN_W / 2.f - 60.f, WIN_H - 48.f, 120.f, 30.f, mouse);
    drawTextCentered(window, font, "QUITTER", 13, qhov ? M_RED : M_MUTED,
                     WIN_W / 2.f, WIN_H - 33.f);

    window.display();
  }

  return false;
}
