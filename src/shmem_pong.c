/*****************************************************
 * Copyright Grégory Mounié 2016                     *
 *           Frédéric Pétrot 2016                    *
 *                                                   *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>


const int TAILLE_X = 800;
const int TAILLE_Y = 600;

/* parametres de l'affichage de la balle */
static int x;
static int y;
static int delta_x;
static int delta_y;
static int taille;
static int couleur;

#define handle_error(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_SDL_error(msg) \
  do { fprintf(stderr, "SDL :" msg ", ligne %d \n", __LINE__); exit(EXIT_FAILURE); } while (0)

static void put_pixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
    int bpp = surface->format->BytesPerPixel;
    uint8_t *p = (uint8_t *) surface->pixels + y * surface->pitch + x * bpp;
    *(uint32_t *) p = pixel;
}

static uint32_t get_pixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    uint8_t *p = (uint8_t *) surface->pixels + y * surface->pitch + x * bpp;
    return *(uint32_t *) p;
}

static void draw_ball(SDL_Surface *canvas)
{
    /* fond noir sur l'ancienne position */
    int noir = 0;
    for (int k = 0; k < taille; k++)
        for (int l = 0; l < taille; l++)
            put_pixel(canvas, x + k, y + l, noir);


    bool collisionXDroite = false;
    bool collisionXGauche = false;
    bool collisionYHaut = false;
    bool collisionYBas = false;
    for(int i = 0; i < taille; i++)
    {
        if(x + taille + 1 > TAILLE_X)
        {
            continue;
        }
        if(get_pixel(canvas, x + taille + 1, y + i) != 0)
        {
            collisionXDroite = true;
            if(delta_x > 0)
            {
                delta_x *= -1;
            }
            break;
        }
        if(x - 1 < 0)
        {
            continue;
        }
        if(get_pixel(canvas, x - 1, y + i) != 0)
        {
            collisionXGauche = true;
            if(delta_x < 0)
            {
                delta_x *= -1;
            }
            break;
        }
        if(y - 1 < 0)
        {
            continue;
        }
        if(get_pixel(canvas, x + i, y - 1) != 0)
        {
            collisionYHaut = true;
            if(delta_y < 0)
            {
                delta_y *= -1;
            }
            break;
        }
        if(y + taille + 1 > TAILLE_Y)
        {
            continue;
        }
        if(get_pixel(canvas, x + i, y + taille + 1) != 0)
        {
            collisionYBas = true;
            if(delta_y > 0)
            {
                delta_y *= -1;
            }
            break;
        }
    }


    /* modification de position */
    // Si on a une collision on se décalle un peu plus
    x += delta_x;
    y += delta_y;

    /* inversion des directions et ajustement */
    if (x + taille > TAILLE_X) {
        x = TAILLE_X - taille;
        delta_x *= -1;
    }

    if (x < 0) {
        x = 0;
        delta_x *= -1;
    }

    if (y + taille > TAILLE_Y) {
        y = TAILLE_Y - taille;
        delta_y *= -1;
    }

    if (y < 0) {
        y = 0;
        delta_y *= -1;
    }


    /* dessin de la balle */
    for (int k = 0; k < taille; k++)
        for (int l = 0; l < taille; l++)
            put_pixel(canvas, x + k, y + l, couleur);
}


int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    SDL_Surface *canvas = NULL;
    SDL_Window *ecran = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
        handle_SDL_error("Init");

    ecran = SDL_CreateWindow("Ensipong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, TAILLE_X, TAILLE_Y, 0);
    if (ecran == NULL)
        handle_SDL_error("SetVideoMode");

    /*
       On initialise le générateur de nombres aléatoires en utilisant
       le pid du processus courant comme graine. Ainsi, chaque nouvelle
       exécution du programme donnera des nombres différents.
    */
    srandom(getpid());

    /*
       On initialise les caractéristiques de la balle : position initiale,
       taille, couleur. Ces valeurs sont tirées aléatoirement.
    */
    x = random() % TAILLE_X; 
    y = random() % TAILLE_Y;
    delta_x = random() % 5 + 1; // si positif, la balle va vers la droite ; si négatif, elle va vers la gauche
    delta_y = random() % 5 + 1; // si positif, la balle va vers le bas ; si négatif, elle va vers le haut
    // On commence toujours par aller vers la droite et vers le bas.
    taille = random() % 20 + 10;
    couleur =
        (random() % 256 << 16) | (random() % 256 << 8) | (random() % 255);

  /********************
   * Le tampon a changer pour mettre en place le couplage mémoire
   * entre les différents processus.
   *********************/

   // Partie modifiée
    int taille = TAILLE_X * TAILLE_Y * 4;
    int val_shm = shm_open("/shared_space", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    ftruncate(val_shm, taille);
    void* tampon = mmap(NULL, taille, PROT_READ | PROT_WRITE, MAP_SHARED, val_shm, 0);

    if (tampon == NULL)
        handle_error("malloc");

    /* On associe ce tampon à la zone de dessin. */
    canvas =
        SDL_CreateRGBSurfaceFrom(tampon, TAILLE_X, TAILLE_Y, 32, TAILLE_X * 4,
                                 0x0, 0x0, 0x0, 0x0);
    if (canvas == NULL)
        handle_SDL_error();

    renderer = SDL_CreateRenderer(ecran, -1, SDL_RENDERER_SOFTWARE);

    assert(canvas->pixels == tampon);

    if (SDL_BlitSurface(canvas, NULL, SDL_GetWindowSurface(ecran), NULL) == -1)
        handle_SDL_error("BlitSurface");

    SDL_RenderPresent(renderer);

    /* Boucle infinie d'affichage de l'animation de la balle. */
    while (1) {
        /* On attend 20ms entre chaque affichage. */
        SDL_Delay(20);

        /* On dessine la nouvelle position de la balle. */
        draw_ball(canvas);

        if (SDL_BlitSurface(canvas, NULL, SDL_GetWindowSurface(ecran), NULL) == -1)
            handle_SDL_error("BlitSurface");

        SDL_RenderPresent(renderer);

        /*
           Si l'utilisateur clique sur la croix pour fermer la fenêtre,
           on s'arrête proprement.
         */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                goto fin;
        }
    }

  fin:
    /* On libère la mémoire avant de s'arrêter. */
    SDL_FreeSurface(canvas);
    close(val_shm);
    munmap(tampon, taille);
    shm_unlink("/shared_space");
    SDL_Quit();

    return 0;
}
