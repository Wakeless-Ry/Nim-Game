#include "../includes/Game.h"
#include "../includes/Interface.hpp"

int main() {
  GameState game = {20, 3, 1, 50, {0, NULL}};
  init_game(&game);

  run_interface(game);

  detruire_graphe(&game.ai_graphe);
  printf("Jeu terminé!\n");
  return 0;
}
