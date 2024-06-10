#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
        ALLEGRO_BITMAP *shipShots[2];
        ALLEGRO_BITMAP *enemyShot[1];
        ALLEGRO_BITMAP *explosionEnemy[0];
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


// EFFECTS

#define FX_N 128
typedef struct FX {
        int x, y;
        int frame;
        bool exploded;
        bool used;
} FX;

FX *init_fx()
{
        FX *fx = malloc(sizeof(FX) * FX_N);
        for (int i = 0; i < FX_N; i++) {
                fx[i].used = false;
        }

        return fx;
}

// PLAYER

#define SPEED 1
#define PLAYER_MAX_X (WIDTH - 32)
#define PLAYER_MAX_Y (HEIGHT - 32)

typedef struct {
        int x, y;
        int shot_timer;
} Player;

void draw_player(ALLEGRO_BITMAP *sprite, Player *player)
{
        al_draw_bitmap(sprite, player->x, player->y, 0);
}

// ENEMIES

#define NUM_ENEMIES 30
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
        int shot_timer;
        Enemy_type type;
} Enemies;

Enemies *init_enemies()
{
        Enemies *e = malloc(sizeof(Enemies) * NUM_ENEMIES);
        for (int i = 0; i < NUM_ENEMIES; i++) {
                int row = i / 10;
                int col = i % 10;
                e[i].x = col *  (ENEMY_WIDTH + SPACING);
                e[i].y = row *  (ENEMY_HEIGHT + SPACING) + 40;
                e[i].frame = 0;
                e[i].shot_timer = rand() % 1000 + 200;
                e[i].dx = SPEED;
                e[i].alive = true;

                if (row % 3 == 0) 
                        e[i].type = SQUID;
                else if (row % 3 == 1)
                        e[i].type = BUG;
                else if (row % 3 == 2)
                        e[i].type = ANT;
        }

        return e;
}

void draw_enemies(Sprites *s, Enemies *e)
{
        for (int i = 0; i < NUM_ENEMIES; i++) {
                if (e[i].alive) {
                        int frame_display = (e[i].frame / 30) % 2;
                        switch (e[i].type) {
                                case SQUID:
                                        al_draw_bitmap(s->squidEnemy[frame_display], e[i].x, e[i].y, 0);
                                        break;
                                case BUG:
                                        al_draw_bitmap(s->bugEnemy[frame_display], e[i].x, e[i].y, 0);
                                        break;
                                case ANT:
                                        al_draw_bitmap(s->antEnemy[frame_display], e[i].x, e[i].y, 0);
                                        break;
                        }
                }
        }        
}
void destroy_enemies(Enemies *e)
{
        free(e);
}

// AUDIO

typedef struct {
        ALLEGRO_SAMPLE *sample_shot;
        ALLEGRO_SAMPLE *sample_explode;
} Audio;

void audio_init(Audio *audio)
{
        al_install_audio();
        al_init_acodec_addon();
        al_reserve_samples(64);

        audio->sample_shot = al_load_sample("audio_shot.flac");
        audio->sample_explode = al_load_sample("audio_explode1.flac");

}



void fx_add(FX *fx, bool explode, int x, int y, Audio *audio)
{
        if (explode)
                al_play_sample(audio->sample_explode, 0.75, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);

        for (int i = 0; i < FX_N; i++) {
                if (fx[i].used)
                        continue;
                
                fx[i].x = x;
                fx[i].y = y;
                fx[i].frame = 0;
                fx[i].exploded = explode;
                fx[i].used = true;
                return;
        }
}

void update_fx(FX *fx)
{
        for (int i = 0; i < FX_N; i++) {
                if (!fx[i].used)
                        continue;

                fx[i].frame++;

                if (fx[i].exploded)
                        fx[i].used = false; 
        }
}

void draw_fx(Sprites *s, FX *fx)
{
        for (int i = 0; i < FX_N; i++) {
                if (!fx[i].used)
                        continue;

                int frame_display = (fx[i].frame / 2) % 1 ;
                if (fx[i].exploded)
                        al_draw_bitmap(s->)
        }
}

void destroy_audio(Audio *audio)
{
        al_destroy_sample(audio->sample_shot);
        al_destroy_sample(audio->sample_explode);
        free(audio);
        audio = NULL;
}

// SHOTS

typedef struct {
        int x, y, dy;
        int frame;
        bool ship;
        bool used;
} Shots;

#define SHOTS_N 256
#define SHOTS_H 32

void shots_init(Shots *shots)
{
        for (int i = 0; i < SHOTS_N; i++) {
                shots[i].used = false;
        }
}

void shoot(bool ship, int x, int y, Shots *shots)
{
        for (int i = 0; i < SHOTS_N; i++) {
                if (shots[i].used) 
                        continue;

                shots[i].x = x;
                shots[i].y = y;

                if (ship) {
                        shots[i].dy = 5;
                        shots[i].ship = true;
                } else {
                        shots[i].dy = 1;
                        shots[i].ship = false;
                }

                shots[i].used = true;
                shots[i].frame = 0;
                break;

        }
}

void update_shots(Shots *shots)
{
        for (int i = 0; i < SHOTS_N; i++) {
                if (!shots[i].used)
                        continue;

                if (shots[i].ship) {
                        shots[i].y -= shots[i].dy;
                        if (shots[i].y < -SHOTS_H) {
                                shots[i].used = false;
                                continue;
                        }
                } else {
                        shots[i].y += shots[i].dy;
                        if (shots[i].y > HEIGHT) {
                                shots[i].used = false;
                                continue;
                        }
                }
                shots[i].frame++;
        }
}


#define PLAYER_SPEED 5
void update_player(Player *player, unsigned char key[], Shots *shots)
{
        if (key[ALLEGRO_KEY_LEFT])
                player->x -= PLAYER_SPEED;
        if (key[ALLEGRO_KEY_RIGHT])
                player->x += PLAYER_SPEED;
        if (player->x < 0)
                player->x = 0;
        if (player->y < 0)
                player->y = 0;

        if (player->x > PLAYER_MAX_X)
                player->x = PLAYER_MAX_X;

        if (player->shot_timer)
                player->shot_timer--;
        else if (key[ALLEGRO_KEY_SPACE]) {
                if (player->shot_timer == 0)
                        shoot(true, player->x, player->y, shots);
                player->shot_timer = 10; 
        }
}

void update_enemies(Enemies *e, Shots *shots)
{
        bool hittedEdge = false;

        for (int i = 0; i < NUM_ENEMIES; i++) {
                if (e[i].alive) {
                        if (e[i].x < 0 || e[i].x + ENEMY_WIDTH > WIDTH) {
                                hittedEdge = true;
                                break;
                        }
                }
        }

        if (hittedEdge) {
                for (int i = 0; i < NUM_ENEMIES; i++) {
                        e[i].dx = -e[i].dx;
                }
        }


        for (int i = 0; i < NUM_ENEMIES; i++) {
                if ( e[i].alive ) {
                        e[i].x += e[i].dx;                       
                        e[i].frame++;

                        if (e[i].shot_timer > 0)
                                e[i].shot_timer--;

                        if (e[i].shot_timer == 0) {
                                if (e[i].shot_timer == 0) {
                                        int shot_x = e[i].x;
                                        int shot_y = e[i].y;
                                        shoot(false, shot_x, shot_y, shots);
                                        e[i].shot_timer = rand() % 10000 + 200;
                                }
                        }
                }
        }
}

void draw_shots(Sprites *s, Shots *shots)
{
        for (int i = 0; i < SHOTS_N; i++) {
                if (!shots[i].used)
                        continue;

                int frame_display = (shots[i].frame / 2) % 2;
                if (shots[i].ship) {
                        al_draw_bitmap(s->shipShots[frame_display], shots[i].x, shots[i].y, 0);
                } else {
                        al_draw_bitmap(s->enemyShot[0], shots[i].x, shots[i].y, 0);
                }
        }
}

int main()
{
        srand(time(0));
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

        // AUDIO INIT
        Audio *audio = malloc(sizeof(Audio));
        audio_init(audio);

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

        spirtes->explosionEnemy[0] = al_create_sub_bitmap(sprites->_sheet, 4 * 32, 32, 32, 32);

        // SHOTS SPRITES
        sprites->shipShots[0] = al_create_sub_bitmap(sprites->_sheet, 2 * 32, 32, 32, 32);
        sprites->shipShots[1] = al_create_sub_bitmap(sprites->_sheet, 3 * 32, 32, 32, 32);
        sprites->enemyShot[0] = al_create_sub_bitmap(sprites->_sheet, 1 * 32, 32, 32, 32);

        // INIT PLAYER
        Player *player = malloc(sizeof(Player));
        player->x = 100;
        player->y = HEIGHT - 2 * 32;

        // INIT ENEMIES
        Enemies *enemies = init_enemies();

        // INIT SHOTS
        Shots *shots = malloc(sizeof(Shots) * SHOTS_N);
        shots_init(shots);

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
                                update_enemies(enemies, shots);
                                update_player(player, key, shots);
                                update_shots(shots);
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

                        draw_shots(sprites, shots);
                        draw_enemies(sprites, enemies);
                        draw_player(sprites->spaceShip, player);

                        al_flip_display();

                        redraw = false;
                }
        }

        free(shots);
        destroy_audio(audio);
        destroy_enemies(enemies);
        destroy_sprites(sprites);
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_timer(timer);
        al_destroy_event_queue(queue);

        return 0;
}
