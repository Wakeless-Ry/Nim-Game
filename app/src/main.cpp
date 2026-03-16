#include "../includes/Game.h"
#include "../includes/Interface.hpp"
#include "../includes/menu.hpp"

int main() {
  // Default values (menu will override them)
  GameState game = {15, 3, 1, 40, {0, NULL}};

  // Show the main menu; user picks difficulty + stick count
  if (!run_menu(game)) {
    printf("Partie annulée.\n");
    return 0;
  }

  // Initialize the AI graph with the chosen settings
  init_game(&game);

  // Run the game
  run_interface(game);

  detruire_graphe(&game.ai_graphe);
  printf("Jeu terminé!\n");
  return 0;
}