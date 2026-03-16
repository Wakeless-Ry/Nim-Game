#ifndef GAME_H
#define GAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AI.h"

typedef struct {
  int total_sticks;
  int max_pick;
  int player_turn;
  int ai_difficulty;
  graphe_t ai_graphe;

} GameState;

#ifdef __cplusplus
void init_game(GameState *state);
int player_picks(GameState *state, int count);
int ai_picks(GameState *state);
bool is_game_over(const GameState *state);
}
#else
// Déclarations C pures
void init_game(GameState *state);
int player_picks(GameState *state, int count);
int ai_picks(GameState *state);
int is_game_over(const GameState *state);
#endif

#endif
