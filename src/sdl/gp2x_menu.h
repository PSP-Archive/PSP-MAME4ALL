#ifndef GP2X_MENU_H
#define GP2X_MENU_H

void game_list_init(void);
void game_list_view(int *pos);
void game_list_select (int index, char *game);
void select_game(char *game);
void gp2x_intro_screen(void);

#endif
