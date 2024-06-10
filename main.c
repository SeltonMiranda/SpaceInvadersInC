#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

void must_init(bool test, const char *description)
{
        if(test) return;

        printf("couldn't initialize %s\n", description);
        exit(1);
}

#define HEIGHT 480
#define WIDTH 720

#define KEY_SEEN 1
#define KEY_RELEASED 2

void keyboard_init(unsigned char key[])
{
        for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
                key[i] = 0;
}

void keyboard_update(ALLEGRO_EVENT *e, unsigned char key[])
{
        switch (e->type)
        {
                case ALLEGRO_EVENT_TIMER:
                        for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
                                key[i] &= KEY_SEEN;
                        break;

                case ALLEGRO_EVENT_KEY_DOWN:
                        key[e->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
                        break;

                case ALLEGRO_EVENT_KEY_UP:
                        key[e->keyboard.keycode] &= KEY_RELEASED;
                        break;
        }
}

#define TOTAL_ENEMIES 6
typedef struct {
        ALLEGRO_BITMAP *_sheet;
        ALLEGRO_BITMAP *spaceShip;
        ALLEGRO_BITMAP *squidEnemy[2];
        ALLEGRO_BITMAP *bugEnemy[2];
        ALLEGRO_BITMAP *antEnemy[2];
} Sprites;

void destroy_sprites(Sprites *s)
{
        al_destroy_bitmap(s->spaceShip);

        al_destroy_bitmap(s->squidEnemy[0]);
        al_destroy_bitmap(s->squidEnemy[1]);

        al_destroy_bitmap(s->bugEnemy[0]);
        al_destroy_bitmap(s->bugEnemy[1]);

        al_destroy_bitmap(s->antEnemy[0]);
        al_destroy_bitmap(s->antEnemy[1]);

        al_destroy_bitmap(s->_sheet);
        free(s);
}

#define SPEED 2
#define PLAYER_MAX_X (WIDTH - 32)
#define PLAYER_MAX_Y (HEIGHT - 32)

typedef struct {
        int x, y;
} Player;

void update_player(Player *player, unsigned char key[])
{
        if (key[ALLEGRO_KEY_LEFT])
                player->x -= SPEED;
        if (key[ALLEGRO_KEY_RIGHT])
                player->x += SPEED;
        if (player->x < 0)
                player->x = 0;
        if (player->y < 0)
                player->y = 0;

        if (player->x > PLAYER_MAX_X)
                player->x = PLAYER_MAX_X;

}

void draw_player(ALLEGRO_BITMAP *sprite, Player *player)
{
        al_draw_bitmap(sprite, player->x, player->y, 0);
}

#define NUM_ROWS 7
#define NUM_COLS 7
#define ENEMY_WIDTH 32
#define ENEMY_HEIGHT 32
#define SPACING 10

typedef enum enemy_type {
        SQUID = 0,
        BUG,
        ANT,
} Enemy_type;

typedef struct {
        int x, y;
        int dx;
        int frame;
        bool alive;
        Enemy_type type;
} Enemies;

Enemies **init_enemies()
{
        Enemies **e = malloc(sizeof(Enemies *) * NUM_ROWS);
        e[0] = malloc(sizeof(Enemies) * NUM_ROWS * NUM_COLS);
        for (int i = 1; i < NUM_ROWS; i++)
                e[i] = e[0] + i * NUM_COLS;

        for (int i = 0; i < NUM_ROWS; i++) {
                for (int j = 0; j < NUM_COLS; j++) {
                        e[i][j].x = i * (ENEMY_WIDTH + SPACING);
                        e[i][j].y = j * (ENEMY_HEIGHT + SPACING) + SPACING;
                        e[i][j].frame = 0;
                        e[i][j].dx = SPEED;
                        e[i][j].alive = true;

                        if (i % 3 == 0) 
                                e[j][i].type = SQUID;
                        else if (i % 3 == 1)
                                e[j][i].type = BUG;
                        else if (i % 3 == 2)
                                e[j][i].type = ANT;
                }
        }

        return e;
}

void update_enemies(Enemies **e)
{
        bool hittedEdge = false;

        for (int i = 0; i < NUM_ROWS; i++) {
                for (int j = 0; j < NUM_COLS; j++) {
                        if ( e[i][j].alive ) {
                                if (e[i][j].x < 0 || e[i][j].x + ENEMY_WIDTH > WIDTH) {
                                        hittedEdge = true;
                                        break;
                                }
                        }
                }

                if (hittedEdge)
                        break;
        }

        if (hittedEdge) {
                for (int i = 0; i < NUM_ROWS; i++) {
                        for (int j = 0; j < NUM_COLS; j++) {
                                e[i][j].dx = -e[i][j].dx;
                        }
                }
        }


        for (int i = 0; i < NUM_ROWS; i++) {
                for (int j = 0; j < NUM_COLS; j++) {
                        if ( e[i][j].alive ) {
                                e[i][j].x += e[i][j].dx;                       
                                e[i][j].frame++;
                        }
                }
        }
}

void draw_enemies(Sprites *s, Enemies **e)
{
        for (int i = 0; i < NUM_ROWS; i++) {
                for (int j = 0; j < NUM_COLS; j++) {
                        if (e[i][j].alive) {
                                int frame_display = (e[i][j].frame / 30) % 2;
                                switch (e[i][j].type) {
                                        case SQUID:
                                                al_draw_bitmap(s->squidEnemy[frame_display], e[i][j].x, e[i][j].y, 0);
                                                break;
                                        case BUG:
                                                al_draw_bitmap(s->bugEnemy[frame_display], e[i][j].x, e[i][j].y, 0);
                                                break;
                                        case ANT:
                                                al_draw_bitmap(s->antEnemy[frame_display], e[i][j].x, e[i][j].y, 0);
                                                break;
                                }
                        }
                }
        }
}

void destroy_enemies(Enemies **e)
{
        free(e[0]);
        free(e);
}

int main()
{
        must_init(al_init(), "allegro");
        must_init(al_install_keyboard(), "keyboard");

        ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
        must_init(timer, "timer");

        ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
        must_init(queue, "queue");

        ALLEGRO_DISPLAY* disp = al_create_display(WIDTH, HEIGHT);
        must_init(disp, "display");

        ALLEGRO_FONT* font = al_create_builtin_font();
        must_init(font, "font");

        must_init(al_init_image_addon(), "image addon");

        // SPRITES 
        Sprites *sprites = malloc(sizeof(Sprites));
        sprites->_sheet = al_load_bitmap("invaders_sheet.png");

        // SPACESHIP SPRITE
        sprites->spaceShip = al_create_sub_bitmap(sprites->_sheet, 5*32, 32, 32, 32);

        // ENEMIES SPRITES
        sprites->squidEnemy[0] = al_create_sub_bitmap(sprites->_sheet, 0, 0, 32, 32);
        sprites->squidEnemy[1] = al_create_sub_bitmap(sprites->_sheet, 32, 0, 32, 32);

        sprites->bugEnemy[0] = al_create_sub_bitmap(sprites->_sheet, 2 * 32, 0, 32, 32);
        sprites->bugEnemy[1] = al_create_sub_bitmap(sprites->_sheet, 3 * 32, 0, 32, 32);

        sprites->antEnemy[0] = al_create_sub_bitmap(sprites->_sheet, 4 * 32, 0, 32, 32);
        sprites->antEnemy[1] = al_create_sub_bitmap(sprites->_sheet, 5 * 32, 0, 32, 32);

        // INIT PLAYER
        Player *player = malloc(sizeof(Player));
        player->x = 100;
        player->y = HEIGHT - 2 * 32;

        // INIT ENEMIES
        Enemies **enemies = init_enemies();

        al_register_event_source(queue, al_get_keyboard_event_source());
        al_register_event_source(queue, al_get_display_event_source(disp));
        al_register_event_source(queue, al_get_timer_event_source(timer));

        // INIT KEYBOARD
        unsigned char key[ALLEGRO_KEY_MAX];
        keyboard_init(key);

        bool done = false;
        bool redraw = true;
        ALLEGRO_EVENT event;

        al_start_timer(timer);
        while(1)
        {
                al_wait_for_event(queue, &event);

                switch(event.type)
                {
                        case ALLEGRO_EVENT_TIMER:
                                // game logic goes here.
                                update_enemies(enemies);
                                update_player(player, key);
                                redraw = true;
                                if (key[ALLEGRO_KEY_ESCAPE])
                                        done = true;
                                break;

                        case ALLEGRO_EVENT_DISPLAY_CLOSE:
                                done = true;
                                break;
                }

                if(done)
                        break;

                keyboard_update(&event, key);

                if(redraw && al_is_event_queue_empty(queue))
                {
                        al_clear_to_color(al_map_rgb(0, 0, 0));

                        draw_enemies(sprites, enemies);
                        draw_player(sprites->spaceShip, player);

                        al_flip_display();

                        redraw = false;
                }
        }

        destroy_enemies(enemies);
        destroy_sprites(sprites);
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);

        return 0;
}
