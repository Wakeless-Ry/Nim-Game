// =============================================================
// NIM — Interface SFML + IA intégrée
// =============================================================

#include "../includes/Interface.hpp"
#include "../includes/Game.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <string>
#include <vector>

constexpr unsigned WIN_W = 1200;
constexpr unsigned WIN_H = 900;

const sf::Color BG{18, 16, 24};
const sf::Color PANEL{28, 25, 38};
const sf::Color AMBER{255, 183, 77};
const sf::Color AMBER_DIM{180, 120, 40};
const sf::Color CREAM{245, 235, 210};
const sf::Color MUTED{120, 110, 130};
const sf::Color RED_HEAD{220, 70, 70};
const sf::Color SEL_BODY{255, 210, 100};
const sf::Color SEL_HEAD{255, 140, 0};

struct Stick {
  sf::Vector2f pos;
  bool taken = false;
  bool hovered = false;
  bool selected = false;
  float wobble = 0.f;
};

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

// Return values: 1 = replay, 0 = back to menu, -1 = quit
static int showEndScreen(sf::RenderWindow &window, sf::Font &font,
                         bool playerWon) {
  const std::string title = playerWon ? "TU ES LE VAINQUEUR !" : "L'IA GAGNE !";
  const sf::Color titleCol = playerWon ? AMBER : RED_HEAD;

  while (window.isOpen()) {
    sf::Vector2f mouse(sf::Mouse::getPosition(window));
    sf::Event ev;
    while (window.pollEvent(ev)) {
      if (ev.type == sf::Event::Closed) {
        window.close();
        return -1;
      }
      if (ev.type == sf::Event::MouseButtonPressed &&
          ev.mouseButton.button == sf::Mouse::Left) {
        if (sf::FloatRect{WIN_W / 2.f - 130.f, WIN_H / 2.f + 60.f, 115.f, 44.f}
                .contains(mouse))
          return 1;
        if (sf::FloatRect{WIN_W / 2.f + 15.f, WIN_H / 2.f + 60.f, 115.f, 44.f}
                .contains(mouse))
          return 0;
      }
    }

    window.clear(BG);
    sf::RectangleShape overlay({(float)WIN_W, (float)WIN_H});
    overlay.setFillColor({10, 8, 16, 210});
    window.draw(overlay);

    drawTextCentered(window, font, title, 52, titleCol, WIN_W / 2.f,
                     WIN_H / 2.f - 50.f);
    drawTextCentered(window, font,
                     playerWon ? "Bravo !"
                               : "Dommage, peut-etre la prochaine fois",
                     18, MUTED, WIN_W / 2.f, WIN_H / 2.f + 10.f);

    drawButton(window, font, "REJOUER", WIN_W / 2.f - 130.f, WIN_H / 2.f + 60.f,
               115.f, 44.f, mouse, {40, 80, 50}, {55, 120, 70});
    drawButton(window, font, "MENU", WIN_W / 2.f + 15.f, WIN_H / 2.f + 60.f,
               115.f, 44.f, mouse, {50, 40, 70}, {75, 60, 105});

    window.display();
  }
  return -1;
}

// ── Calcul de la disposition adaptative ────────────────────────────────
// Renvoie : nombre de lignes, allumettes par ligne, gap horizontal, espacement
// vertical
struct LayoutInfo {
  int rows;
  int perRow;
  float gap;        // espacement horizontal entre centres
  float rowSpacing; // espacement vertical entre lignes
};

static LayoutInfo computeLayout(int count) {
  // Largeur utilisable (panel = WIN_W - 160px de marge)
  const float availW = WIN_W - 160.f; // 1040 px
  // Taille minimum entre deux allumettes pour qu'elles restent cliquables
  const float MIN_GAP = 16.f;
  // Espacement vertical fixe entre deux lignes
  const float ROW_SPACE = 76.f;

  // On cherche le nombre de lignes minimum pour que le gap >= MIN_GAP
  for (int rows = 1; rows <= 4; ++rows) {
    int perRow = (count + rows - 1) / rows; // ceil(count / rows)
    float gap = (perRow > 1) ? availW / (perRow - 1) : 0.f;
    if (gap >= MIN_GAP || rows == 4) {
      // Plafonner le gap : au-delà de 46px les allumettes sont trop espacées
      gap = std::min(gap, 46.f);
      return {rows, perRow, gap, ROW_SPACE};
    }
  }
  return {4, (count + 3) / 4, MIN_GAP, ROW_SPACE}; // fallback
}

// ─────────────────────────────────────────────────────────────────────────────

int run_interface(GameState &game_state) {
  sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Nim — IA vs Humain",
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

  std::vector<Stick> sticks(game_state.total_sticks);
  int selCount = 0;
  bool player_turn = true;
  bool showMaxError = false;
  bool showAIMessage = false;
  std::string aiPickedCount;

  // Centre vertical de la zone d'allumettes
  const float AREA_CENTER_Y = WIN_H / 2.f - 10.f;
  // Padding du fond autour de la zone d'allumettes
  const float PAD_H = 30.f; // padding vertical haut et bas dans le fond

  // ── Layout adaptatif ─────────────────────────────────────────────────────
  // La disposition est recalculée à chaque layoutSticks()
  LayoutInfo layout = computeLayout(game_state.total_sticks);

  auto layoutSticks = [&]() {
    int remaining = game_state.total_sticks;

    // Marquer tout comme pris par défaut, puis remplir
    for (auto &s : sticks)
      s.taken = true;

    if (remaining <= 0) {
      selCount = 0;
      showMaxError = false;
      return;
    }

    layout = computeLayout(remaining);

    // Hauteur totale occupée par toutes les lignes
    float totalH = (layout.rows - 1) * layout.rowSpacing;
    // Y de la première ligne
    float firstRowY = AREA_CENTER_Y - totalH / 2.f;

    // Largeur totale d'une ligne (basée sur perRow, même pour la dernière ligne
    // incomplète)
    float totalW = (layout.perRow > 1) ? (layout.perRow - 1) * layout.gap : 0.f;
    float startX = WIN_W / 2.f - totalW / 2.f;

    for (int idx = 0; idx < remaining; ++idx) {
      int row = idx / layout.perRow;
      int col = idx % layout.perRow;
      sticks[idx].taken = false;
      sticks[idx].pos = {startX + col * layout.gap,
                         firstRowY + row * layout.rowSpacing};
      sticks[idx].hovered = false;
      sticks[idx].selected = false;
      sticks[idx].wobble = 0.f;
    }
    selCount = 0;
    showMaxError = false;
  };

  layoutSticks();
  sf::Clock clock;

  while (window.isOpen()) {
    float dt = clock.restart().asSeconds();
    sf::Vector2f mouse(sf::Mouse::getPosition(window));
    sf::Event ev;

    while (window.pollEvent(ev)) {
      if (ev.type == sf::Event::Closed)
        window.close();

      if (ev.type == sf::Event::MouseButtonPressed &&
          ev.mouseButton.button == sf::Mouse::Left) {

        bool confirmHov =
            sf::FloatRect{WIN_W / 2.f - 115.f, WIN_H - 100.f, 105.f, 40.f}
                .contains(mouse);
        bool cancelHov =
            sf::FloatRect{WIN_W / 2.f + 10.f, WIN_H - 100.f, 105.f, 40.f}
                .contains(mouse);

        if (confirmHov && selCount > 0 && player_turn) {
          if (selCount > game_state.max_pick) {
            showMaxError = true;
            continue;
          }

          int picked = 0;
          for (auto &s : sticks) {
            if (s.selected) {
              s.taken = true;
              picked++;
              s.selected = false;
            }
          }
          player_picks(&game_state, picked);

          if (game_state.total_sticks <= 0)
            return showEndScreen(window, font, true);

          {
            int before = game_state.total_sticks;
            ai_picks(&game_state);
            aiPickedCount = std::to_string(before - game_state.total_sticks);
            showAIMessage = true;

            if (game_state.total_sticks <= 0)
              return showEndScreen(window, font, false);
          }

          layoutSticks();
          selCount = 0;

        } else if (cancelHov) {
          for (auto &s : sticks)
            s.selected = false;
          selCount = 0;
          showMaxError = false;
        } else {
          // Clic sur une allumette
          for (auto &s : sticks) {
            if (s.taken)
              continue;
            float dx = mouse.x - s.pos.x;
            float dy = mouse.y - s.pos.y;
            if (std::abs(dx) < 14.f && std::abs(dy) < 34.f &&
                selCount < game_state.max_pick) {
              s.selected = !s.selected;
              selCount += s.selected ? 1 : -1;
              break;
            }
          }
        }
      }
    }

    // Mise à jour hover + wobble
    for (auto &s : sticks) {
      if (s.taken)
        continue;
      float dx = mouse.x - s.pos.x;
      float dy = mouse.y - s.pos.y;
      s.hovered = (std::abs(dx) < 14.f && std::abs(dy) < 34.f);
      s.wobble += dt * (s.hovered ? 5.f : 1.5f);
    }

    // ── Draw ────────────────────────────────────────────────────────────
    window.clear(BG);

    for (int i = 0; i < 13; ++i) {
      sf::RectangleShape line({(float)WIN_W, 1.f});
      line.setPosition(0.f, 40.f * i);
      line.setFillColor({255, 183, 77, 10});
      window.draw(line);
    }

    // En-tête
    sf::RectangleShape header({(float)WIN_W, 62.f});
    header.setFillColor(PANEL);
    window.draw(header);
    drawTextCentered(window, font,
                     player_turn ? "N I M - VOTRE TOUR" : "N I M - IA JOUE", 30,
                     AMBER, WIN_W / 2.f, 31.f);

    std::string badge = std::to_string(game_state.total_sticks) + " restantes";
    drawTextCentered(window, font, badge, 16, MUTED, WIN_W / 2.f, 95.f);

    // Fond de la zone d'allumettes — s'adapte au nombre de lignes
    {
      float totalH = (layout.rows - 1) * layout.rowSpacing;
      float bgH = totalH + 2.f * (PAD_H + 30.f); // demi-allumette ~30px
      float bgY = AREA_CENTER_Y - totalH / 2.f - PAD_H - 30.f;

      sf::RectangleShape rowBg({(float)WIN_W - 160.f, bgH});
      rowBg.setPosition(80.f, bgY);
      rowBg.setFillColor(PANEL);
      rowBg.setOutlineThickness(1.5f);
      window.draw(rowBg);
    }

    // Allumettes
    for (auto &s : sticks)
      drawStick(window, s);

    // Messages d'erreur / IA
    if (showMaxError) {
      float totalH = (layout.rows - 1) * layout.rowSpacing;
      float bottomY = AREA_CENTER_Y + totalH / 2.f + PAD_H + 30.f;
      std::string msg =
          "Max " + std::to_string(game_state.max_pick) + " allumettes!";
      drawTextCentered(window, font, msg, 24, RED_HEAD, WIN_W / 2.f,
                       bottomY + 14.f);
    }

    if (showAIMessage) {
      std::string msg = "L'IA prend " + aiPickedCount + " allumette(s)!";
      drawTextCentered(window, font, msg, 28, MUTED, WIN_W / 2.f, WIN_H / 4.f);
    }

    // Boutons CONFIRMER / ANNULER
    sf::Color confirmFill =
        (selCount > 0 && selCount <= game_state.max_pick && player_turn)
            ? sf::Color{40, 100, 60}
            : sf::Color{30, 50, 35};

    drawButton(window, font, "CONFIRMER", WIN_W / 2.f - 115.f, WIN_H - 100.f,
               105.f, 40.f, mouse, confirmFill,
               selCount > 0 && selCount <= game_state.max_pick
                   ? sf::Color{55, 140, 80}
                   : confirmFill);

    drawButton(window, font, "ANNULER", WIN_W / 2.f + 10.f, WIN_H - 100.f,
               105.f, 40.f, mouse, {90, 40, 40}, {130, 55, 55});

    if (selCount > 0) {
      std::string hint = "Selection: " + std::to_string(selCount);
      drawTextCentered(window, font, hint, 15, SEL_BODY, WIN_W / 2.f,
                       WIN_H - 130.f);
    }

    window.display();
  }
  return 0;
}