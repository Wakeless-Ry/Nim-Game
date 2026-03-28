#include "../includes/Game.h"
#include "../includes/Interface.hpp"
#include "../includes/menu.hpp"
#include <cstdio>

int main() {
  GameState game = {-1, 15, 3, 1, 40, AI_STRATEGY_MIXED, {0, NULL}};

  while (true) {
    // Show menu — fills game with chosen stick count, difficulty, etc.
    if (!run_menu(game))
      break; // user clicked QUITTER

    // Remember the chosen stick count so REJOUER can restore it
    int chosen_sticks = game.total_sticks;

    int result;
    do {
      // Reset stick count for each new round
      game.total_sticks = chosen_sticks;

      // Build AI graph for this round
      detruire_graphe(&game.ai_graphe);
      init_game(&game);

      // Run the game
      // Returns: 1 = REJOUER, 0 = back to MENU, -1 = window closed
      result = run_interface(game);

    } while (result == 1);

    if (result == -1)
      break; // window was closed → exit
             // result == 0 → loop back to menu
  }

  detruire_graphe(&game.ai_graphe);
  printf("Jeu terminé!\n");
  return 0;
}
