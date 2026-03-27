// =============================================================
// NIM — Menu principal (stratégie + difficulté + nombre d'allumettes)
// =============================================================

#include "../includes/menu.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
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
  sf::Color accent;
};

static const Strategy STRATEGIES[5] = {
    {"Graphe optimale",
     "avec degre d'erreurs\n \t\t introduit",
     {90, 150, 230}},
    {"COPIE", "Copie vos coups", {160, 110, 220}},
    {"MINIMAX", "Negamax borne", {70, 190, 140}},
    {"MCTS", "Monte Carlo", {250, 155, 50}},
    {"RECUIT", "Recuit simule", {220, 70, 70}},
};

// ── Difficulté ───────────────────────────────────────────────
struct Difficulty {
  std::string label;
  std::string desc;
  int chances;
  sf::Color accent;
};

// Descriptions reécrites : chaque libellé apporte une info utile
// sans répéter le nom du niveau (ex. "FACILE - Niveau facile")
static const Difficulty DIFFICULTIES[4] = {
    {"FACILE", "Beaucoup d'erreurs", 25, {80, 180, 90}},
    {"MOYEN", "Quelques erreurs", 50, {255, 200, 60}},
    {"DIFFICILE", "Peu d'erreurs", 75, {240, 130, 40}},
    {"IMPOSSIBLE", "Aucune erreur", 100, {220, 60, 60}},
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
  float rad = fs.rot * 3.14159f / 180.f;
  head.setPosition(fs.x - 19.f * std::sin(rad),
                   fs.y + wobble - 19.f * std::cos(rad));
  head.setFillColor(headCol);
  win.draw(head);
}

// ── Section separator : titre + ligne fine centrée ───────────
static void drawSectionSeparator(sf::RenderWindow &win, sf::Font &font,
                                 const std::string &title, float y,
                                 float lineW = 380.f) {
  // Ligne pleine largeur très discrète (séparation visuelle majeure)
  sf::RectangleShape fullLine({(float)WIN_W - 80.f, 1.f});
  fullLine.setPosition(40.f, y - 14.f);
  fullLine.setFillColor({255, 183, 77, 128});
  win.draw(fullLine);

  // Titre de section
  drawTextCentered(win, font, title, 12, M_MUTED, WIN_W / 2.f, y);

  // Ligne courte sous le titre
  sf::RectangleShape sep({lineW, 1.f});
  sep.setPosition(WIN_W / 2.f - lineW / 2.f, y + 11.f);
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
  int selectedStrat  = 0;  // 0=Graphe … 4=RECUIT
  int difficultyPct  = 50; // difficulté réelle 0–100, appliquée directement
  int stickCount = 20;
  const int STICK_MIN = 3;
  const int STICK_MAX = 50;

  // Slider difficulté
  bool sliderDragging = false;

  // Hold-click pour les boutons +/-
  bool  plusHeld    = false;
  bool  minusHeld   = false;
  float holdTimer   = 0.f;
  float repeatTimer = 0.f;
  const float HOLD_DELAY  = 0.35f;  // secondes avant que la répétition démarre
  const float HOLD_REPEAT = 0.08f;  // intervalle entre deux incréments (~12/s)

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
  float titleY = -80.f;

  // ── Layout constants ─────────────────────────────────────
  // Dimensions des cartes (modifier ici pour changer la taille)
  const float STRAT_CARD_W   = 180.f;
  const float STRAT_CARD_H   = 125.f;
  const float STRAT_CARD_GAP = 15.f;
  const float STRAT_TOTAL_W  = 5 * STRAT_CARD_W + 4 * STRAT_CARD_GAP;
  const float STRAT_START_X  = (WIN_W - STRAT_TOTAL_W) / 2.f;

  const float DIFF_SLIDER_H  = 64.f;   // hauteur totale de la zone slider difficulté

  // Cascade : modifier ces valeurs redistribue tout le menu automatiquement
  const float HEADER_H      = 110.f;
  const float LABEL_PADDING = 16.f;  // espace entre titre de section et ses cartes
  const float SEC_GAP       = 44.f;  // espace entre bas d'une section et titre de la suivante

  const float STRAT_LABEL_Y = HEADER_H + 20.f;
  const float STRAT_CARD_Y  = STRAT_LABEL_Y + LABEL_PADDING;
  const float STRAT_BOTTOM  = STRAT_CARD_Y + STRAT_CARD_H;

  const float DIFF_LABEL_Y  = STRAT_BOTTOM + SEC_GAP;
  const float DIFF_CARD_Y   = DIFF_LABEL_Y + LABEL_PADDING;
  const float DIFF_BOTTOM   = DIFF_CARD_Y + DIFF_SLIDER_H;

  const float SC_LABEL_Y    = DIFF_BOTTOM + SEC_GAP;
  const float SC_BTN_Y      = SC_LABEL_Y + LABEL_PADDING;
  const float SC_PREVIEW_Y  = SC_BTN_Y + 70.f;
  const float SC_BOTTOM     = SC_PREVIEW_Y + 42.f;

  const float RULES_Y       = SC_BOTTOM + SEC_GAP;

  // Slider geometry (difficulté)
  const float SL_W       = 560.f;
  const float SL_X       = WIN_W / 2.f - SL_W / 2.f;  // 320
  const float SL_TRACK_Y = DIFF_CARD_Y + 30.f;         // centre vertical de la piste
  const float SL_TRACK_H = 6.f;
  const float SL_THUMB_R = 14.f;

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

        // ── Slider difficulté ─────────────────────────
        {
          float thumbX = SL_X + (difficultyPct / 100.f) * SL_W;
          bool onThumb = std::abs(mouse.x - thumbX) <= SL_THUMB_R + 4.f &&
                         std::abs(mouse.y - SL_TRACK_Y) <= SL_THUMB_R + 4.f;
          bool onTrack = mouse.y >= SL_TRACK_Y - SL_THUMB_R - 4.f &&
                         mouse.y <= SL_TRACK_Y + SL_THUMB_R + 4.f &&
                         mouse.x >= SL_X && mouse.x <= SL_X + SL_W;
          if (onThumb) {
            sliderDragging = true;
          } else if (onTrack) {
            float ratio = (mouse.x - SL_X) / SL_W;
            difficultyPct = std::clamp((int)std::round(ratio * 100.f), 0, 100);
          }
        }

        // ── Stick count −/+ (premier clic immédiat) ───
        if (isHovered(WIN_W / 2.f - 110.f, SC_BTN_Y, 44.f, 44.f, mouse)) {
          if (stickCount > STICK_MIN) stickCount--;
          minusHeld = true; holdTimer = 0.f; repeatTimer = 0.f;
        }
        if (isHovered(WIN_W / 2.f + 66.f, SC_BTN_Y, 44.f, 44.f, mouse)) {
          if (stickCount < STICK_MAX) stickCount++;
          plusHeld = true; holdTimer = 0.f; repeatTimer = 0.f;
        }

        // ── JOUER ─────────────────────────────────────
        if (isHovered(WIN_W / 2.f - 120.f, WIN_H - 110.f, 240.f, 52.f, mouse)) {
          game_state.total_sticks = stickCount;
          game_state.max_pick = 3;
          game_state.player_turn = 1;
          game_state.ai_difficulty = difficultyPct;
          game_state.ai_strategy = (AIStrategyType)selectedStrat;
          window.close();
          return true;
        }

        // ── QUITTER ───────────────────────────────────
        if (isHovered(WIN_W / 2.f - 60.f, WIN_H - 48.f, 120.f, 30.f, mouse)) {
          window.close();
          return false;
        }
      }

      if (ev.type == sf::Event::MouseButtonReleased &&
          ev.mouseButton.button == sf::Mouse::Left) {
        sliderDragging = false;
        plusHeld = minusHeld = false;
      }
    }

    // ── Slider drag (suivi continu de la souris bouton maintenu) ──────
    if (sliderDragging) {
      if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        float ratio = std::clamp((mouse.x - SL_X) / SL_W, 0.f, 1.f);
        difficultyPct = std::clamp((int)std::round(ratio * 100.f), 0, 100);
      } else {
        sliderDragging = false;
      }
    }

    // ── Hold-click +/- ────────────────────────────────────────────────
    if (plusHeld || minusHeld) {
      holdTimer += dt;
      if (holdTimer >= HOLD_DELAY) {
        repeatTimer += dt;
        if (repeatTimer >= HOLD_REPEAT) {
          repeatTimer = 0.f;
          if (plusHeld  && stickCount < STICK_MAX) stickCount++;
          if (minusHeld && stickCount > STICK_MIN) stickCount--;
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

    float targetY = 50.f;
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
                     WIN_W / 2.f, titleY + 40.f);

    // ── Section STRATEGIE ─────────────────────────────────
    drawSectionSeparator(window, font, "STRATEGIE DE L'IA", STRAT_LABEL_Y);

    for (int i = 0; i < 5; ++i) {
      float cx = STRAT_START_X + i * (STRAT_CARD_W + STRAT_CARD_GAP);
      float cy = STRAT_CARD_Y;
      bool hov = isHovered(cx, cy, STRAT_CARD_W, STRAT_CARD_H, mouse);
      bool sel = (selectedStrat == i);
      auto &s = STRATEGIES[i];

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
      sf::Color descCol =
          sel ? sf::Color{200, 195, 215} : sf::Color{80, 75, 95};
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
    // Label avec info stratégie-dépendante
    {
      std::string diffDetail;
      switch (selectedStrat) {
      case 0:
        diffDetail = "  (% de coups optimaux)";
        break;
      case 1:
        diffDetail = "  (non applicable)";
        break;
      case 2:
        diffDetail = "  (profondeur Negamax)";
        break;
      case 3:
        diffDetail = "  (nb de simulations)";
        break;
      case 4:
        diffDetail = "  (inverse temperature)";
        break;
      }
      drawSectionSeparator(window, font, "DIFFICULTE" + diffDetail,
                           DIFF_LABEL_Y, 500.f);
    }

    // ── Slider difficulté 0–100 ───────────────────────────────
    {
      bool grayed = (selectedStrat == 1);
      float t     = difficultyPct / 100.f;

      // Couleur dégradée vert→rouge selon le pourcentage
      sf::Color accent = grayed ? sf::Color{60, 55, 75} : sf::Color{
          (sf::Uint8)(80  + (int)(t * (220 - 80 ))),
          (sf::Uint8)(180 - (int)(t * (180 - 60 ))),
          (sf::Uint8)(90  - (int)(t * (90  - 60 )))
      };

      float thumbX = SL_X + t * SL_W;

      // Valeur en pourcentage (affiché en grand, centré)
      std::string pctStr = grayed ? "N/A" : std::to_string(difficultyPct) + " %";
      drawTextCentered(window, font, pctStr, 18,
                       grayed ? M_MUTED : accent, WIN_W / 2.f, DIFF_CARD_Y);

      // Piste de fond
      sf::RectangleShape track({SL_W, SL_TRACK_H});
      track.setPosition(SL_X, SL_TRACK_Y - SL_TRACK_H / 2.f);
      track.setFillColor(grayed ? sf::Color{40, 37, 52} : sf::Color{50, 45, 65});
      window.draw(track);

      // Partie remplie (gauche → thumb)
      if (!grayed && thumbX > SL_X) {
        sf::RectangleShape fill({thumbX - SL_X, SL_TRACK_H});
        fill.setPosition(SL_X, SL_TRACK_Y - SL_TRACK_H / 2.f);
        fill.setFillColor(accent);
        window.draw(fill);
      }

      // Ticks de référence à 0, 25, 50, 75, 100
      const int tickVals[] = {0, 25, 50, 75, 100};
      for (int v : tickVals) {
        float tx   = SL_X + (v / 100.f) * SL_W;
        bool  near = (!grayed && std::abs(difficultyPct - v) < 5);

        sf::RectangleShape tick({2.f, 10.f});
        tick.setOrigin(1.f, 5.f);
        tick.setPosition(tx, SL_TRACK_Y);
        tick.setFillColor(near ? accent : sf::Color{70, 65, 85});
        window.draw(tick);

        drawTextCentered(window, font, std::to_string(v), 12,
                         near ? accent : sf::Color{90, 85, 105},
                         tx, SL_TRACK_Y + 24.f);
      }

      // Curseur draggable
      if (!grayed) {
        bool hovThumb = std::abs(mouse.x - thumbX) <= SL_THUMB_R + 4.f &&
                        std::abs(mouse.y - SL_TRACK_Y) <= SL_THUMB_R + 4.f;
        sf::CircleShape thumb(SL_THUMB_R);
        thumb.setOrigin(SL_THUMB_R, SL_THUMB_R);
        thumb.setPosition(thumbX, SL_TRACK_Y);
        thumb.setFillColor(sliderDragging || hovThumb
            ? sf::Color{(sf::Uint8)std::min(255, (int)accent.r + 40),
                        (sf::Uint8)std::min(255, (int)accent.g + 40),
                        (sf::Uint8)std::min(255, (int)accent.b + 40)}
            : accent);
        thumb.setOutlineColor(M_CREAM);
        thumb.setOutlineThickness(2.f);
        window.draw(thumb);
      }

      // Description contextuelle selon la plage
      const char *desc = difficultyPct < 25 ? "Beaucoup d'erreurs"
                       : difficultyPct < 50 ? "Quelques erreurs"
                       : difficultyPct < 75 ? "Peu d'erreurs"
                                            : "Aucune erreur";
      drawTextCentered(window, font,
                       grayed ? "Difficulte non applicable" : desc,
                       12, grayed ? M_MUTED : sf::Color{180, 175, 195},
                       WIN_W / 2.f, SL_TRACK_Y + 40.f);
    }

    // ── Section NOMBRE D'ALLUMETTES ───────────────────────
    drawSectionSeparator(window, font, "NOMBRE D'ALLUMETTES", SC_LABEL_Y);

    float btnY = SC_BTN_Y;

    drawButton(
        window, font, "-", WIN_W / 2.f - 110.f, btnY, 44.f, 44.f, mouse,
        stickCount > STICK_MIN ? sf::Color{50, 45, 65} : sf::Color{30, 28, 40},
        sf::Color{80, 70, 100}, stickCount > STICK_MIN ? M_CREAM : M_MUTED, 22);

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

    drawButton(
        window, font, "+", WIN_W / 2.f + 66.f, btnY, 44.f, 44.f, mouse,
        stickCount < STICK_MAX ? sf::Color{50, 45, 65} : sf::Color{30, 28, 40},
        sf::Color{80, 70, 100}, stickCount < STICK_MAX ? M_CREAM : M_MUTED, 22);

    drawTextCentered(window, font,
                     "min " + std::to_string(STICK_MIN) + "  -  max " +
                         std::to_string(STICK_MAX),
                     10, M_MUTED, WIN_W / 2.f, btnY + 55.f);

    // Preview sticks
    {
      int preview = std::min(stickCount, 20);
      float gapP = std::min(32.f, (float)(WIN_W - 200) / preview);
      float totalPW = (preview - 1) * gapP;
      float startPX = WIN_W / 2.f - totalPW / 2.f;
      float previewY = SC_PREVIEW_Y + 18.f;

      for (int i = 0; i < preview; ++i) {
        float px = startPX + i * gapP;
        float wobble = std::sin(totalTime * 1.8f + i * 0.4f) * 2.f;

        sf::RectangleShape body({8.f, 45.f});
        body.setOrigin(3.f, 13.f);
        body.setPosition(px, previewY + wobble);
        body.setFillColor({200, 175, 110, 180});
        window.draw(body);

        sf::CircleShape head(5.5f);
        head.setOrigin(4.f, 4.f);
        head.setPosition(px, previewY - 13.f + wobble);
        head.setFillColor({190, 60, 60, 255});
        window.draw(head);
      }
      if (stickCount > 20) {
        drawTextCentered(window, font,
                         "+ " + std::to_string(stickCount - 20) + " autres...",
                         10, M_MUTED, WIN_W / 2.f, previewY + 22.f);
      }
    }

    // ── Section RÈGLES DU JEU ─────────────────────────────
    drawSectionSeparator(window, font, "REGLES DU JEU", RULES_Y, 420.f);

    {
      const float BOX_W = 640.f;
      const float BOX_H = 150.f;
      const float BOX_X = WIN_W / 2.f - BOX_W / 2.f;
      const float BOX_Y = RULES_Y + 18.f;

      sf::RectangleShape rulesBox({BOX_W, BOX_H});
      rulesBox.setPosition(BOX_X, BOX_Y);
      rulesBox.setFillColor({28, 25, 40, 220});
      rulesBox.setOutlineColor({255, 183, 77, 45});
      rulesBox.setOutlineThickness(1.f);
      window.draw(rulesBox);

      // Ligne décorative gauche en couleur ambre
      sf::RectangleShape leftAccent({3.f, BOX_H - 16.f});
      leftAccent.setPosition(BOX_X + 8.f, BOX_Y + 8.f);
      leftAccent.setFillColor({255, 183, 77, 120});
      window.draw(leftAccent);

      drawTextCentered(window, font, "A chaque tour, prenez 1 a 3 allumettes.",
                       19, M_CREAM, WIN_W / 2.f, BOX_Y + 20.f);
      drawTextCentered(
          window, font,
          "Celui qui prend la DERNIERE allumette gagne la partie !", 19,
          M_AMBER, WIN_W / 2.f, BOX_Y + (BOX_H/2.f));
      drawTextCentered(window, font,
                       "Choisissez votre strategie et defiez l'IA !",19,
                       M_MUTED, WIN_W / 2.f, BOX_Y + BOX_H - 20.f);
    }

    // ── JOUER button ──────────────────────────────────────
    {
      auto &selS = STRATEGIES[selectedStrat];
      bool jouerhov =
          isHovered(WIN_W / 2.f - 120.f, WIN_H - 110.f, 240.f, 52.f, mouse);
      sf::RectangleShape btn({240.f, 52.f});
      btn.setPosition(WIN_W / 2.f - 120.f, WIN_H - 110.f);
      sf::Color base = selS.accent;
      btn.setFillColor(jouerhov
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
    bool qhov = isHovered(WIN_W / 2.f - 60.f, WIN_H - 48.f, 120.f, 30.f, mouse);
    drawTextCentered(window, font, "QUITTER", 13, qhov ? M_RED : M_MUTED,
                     WIN_W / 2.f, WIN_H - 33.f);

    window.display();
  }

  return false;
}
