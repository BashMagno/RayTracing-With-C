//Maths based on https://en.wikipedia.org/wiki/Ray_tracing_(graphics)

#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <stdlib.h>

const int WIDTH = 1200;
const int HEIGHT = 800;
const Uint32 COLOR_WHITE = 0xffffffff;
const Uint32 COLOR_BLACK = 0x00000000;
const Uint32 COLOR_RED = 0xb1524b;
const Uint32 COLOR_GRAY = 0xefefefef;
const Uint32 COLOR_YELLOW = 0xFFFF00;
const int MAX_RAY_AMOUNT = 1000000;

/*
    Structs
*/
struct Circle 
{
    double x,y,r;
};

struct Ray 
{
    double  xStart, //empieza x
            yStart; //empieza y
};

/*
    FillMethods
*/
void fillCircle( SDL_Surface* surface, struct Circle circle, Uint32 colorIn)
{
    double radius_squared = pow( circle.r, 2 );

    for( double x = circle.x - circle.r; x <= circle.x+circle.r; x++ )
    {
        for( double y = circle.y-circle.r; y <= circle.y+circle.r; y++  )
        {
            double distance_squared = pow( x-circle.x, 2 ) + pow( y-circle.y, 2 );
            if( distance_squared < radius_squared )
            { 
                SDL_Rect pixel = ( SDL_Rect ){x,y,1,1};
                SDL_FillRect(surface, &pixel, colorIn);
            }

        }
    }
}

void fillRay(SDL_Surface* surface, struct Ray rays[], Uint32 color, int rays_amount, struct Circle circle)
{
    // Longitud máxima: diagonal de la pantalla
    int length = sqrt(WIDTH * WIDTH + HEIGHT * HEIGHT);
    int thickness = 4;

    for (int i = 0; i < rays_amount; i++)
    {
        struct Ray ray = rays[i];
        double angle = (2 * M_PI / rays_amount) * i;
        double x_draw = ray.xStart;
        double y_draw = ray.yStart;

        // Dirección del rayo
        double dx = cos(angle);
        double dy = sin(angle);

        // Parámetros para intersección con el círculo
        double ox = ray.xStart - circle.x;
        double oy = ray.yStart - circle.y;

        double a = dx * dx + dy * dy;
        double b = 2 * (ox * dx + oy * dy);
        double c = ox * ox + oy * oy - circle.r * circle.r;

        double discriminant = b * b - 4 * a * c;
        double t_collision = length; // Colisión mínima en longitud

        if (discriminant >= 0)
        {
            double sqrt_disc = sqrt(discriminant);
            double t1 = (-b - sqrt_disc) / (2 * a);
            double t2 = (-b + sqrt_disc) / (2 * a);

            // Se elige la colisión más cercana en la dirección del rayo
            if (t1 > 0 && t1 < t_collision) t_collision = t1;
            if (t2 > 0 && t2 < t_collision) t_collision = t2;
        }

        // Dibujar el rayo hasta el punto de colisión
        for (int step = 0; step < t_collision && step < length; step++)
        {
            x_draw = ray.xStart + dx * step;
            y_draw = ray.yStart + dy * step;

            SDL_Rect pixel = {x_draw - thickness / 2, y_draw - thickness / 2, thickness, thickness};
            SDL_FillRect(surface, &pixel, color);

            if (x_draw < 0 || x_draw > WIDTH || y_draw < 0 || y_draw > HEIGHT)
            {
                break;
            }
        }

        // Si el rayo colisiona, continuar la sombra desde el punto de colisión hacia afuera
        if (t_collision < length)
        {
            int shadow_thickness = 1;
            for (int step = t_collision + 1; step < length; step++) // Empieza después del punto de colisión
            {
                x_draw = ray.xStart + dx * step;
                y_draw = ray.yStart + dy * step;

                // Solo dibujar sombra si ya ha salido del círculo
                double dist_to_center = sqrt((x_draw - circle.x) * (x_draw - circle.x) +
                                             (y_draw - circle.y) * (y_draw - circle.y));

                if (dist_to_center > circle.r) 
                {
                    SDL_Rect shadow_pixel = {x_draw - shadow_thickness / 2, y_draw - shadow_thickness / 2, shadow_thickness, shadow_thickness};
                    SDL_FillRect(surface, &shadow_pixel, COLOR_BLACK);
                }

                if (x_draw < 0 || x_draw > WIDTH || y_draw < 0 || y_draw > HEIGHT)
                {
                    break;
                }
            }
        }
    }
}



/*
    Render Methods
*/
void render_rays(struct Circle circle, struct Ray rays[], int rays_amount)
{
    for (int i = 0; i < rays_amount; i++)
    {
        double angle = ( (double)i / rays_amount ) * 2 * M_PI;
        
        // Circunferencia circulo emisor de rayos
        double x_start = circle.x + circle.r * cos(angle);
        double y_start = circle.y + circle.r * sin(angle);

        struct Ray ray = {x_start, y_start};
        rays[i] = ray;

        printf("DEBUG: ANGULOS - > %f | X: %f, Y: %f\n", angle, x_start, y_start);
    }
}


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Raytracing",
         SDL_WINDOWPOS_CENTERED, 
         SDL_WINDOWPOS_CENTERED, 
         WIDTH, HEIGHT,
         0);
    SDL_Surface* surface = SDL_GetWindowSurface(window);  

    int rays_amount = 300;
    struct Circle circle1 = {((2 * WIDTH) / 3), HEIGHT / 2, 100}; // Circulo afectado
    struct Circle circle = {200, HEIGHT / 2, 60}; // Circulo emisor

    // Asignación dinámica de memoria para los rayos
    struct Ray* rays = (struct Ray*)malloc(rays_amount * sizeof(struct Ray));
    if (!rays) {
        printf("Error: No se pudo asignar memoria para los rayos.\n");
        return 1;
    }

    render_rays(circle, rays, rays_amount);
    short int running = 1;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
            if (event.type == SDL_MOUSEMOTION && event.motion.state != 0)
            {
                double new_x = event.motion.x;
                double new_y = event.motion.y;

                double dx = new_x - circle1.x;
                double dy = new_y - circle1.y;
                double distance = sqrt(dx * dx + dy * dy);

                if (distance >= (circle.r + circle1.r))
                {
                    circle.x = new_x;
                    circle.y = new_y;
                }
                else
                {
                    double angle = atan2(dy, dx);
                    circle.x = circle1.x + (circle.r + circle1.r) * cos(angle);
                    circle.y = circle1.y + (circle.r + circle1.r) * sin(angle);
                }

                render_rays(circle, rays, rays_amount);
            }

            // Control de cantidad de rayos con teclas ↑ ↓
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_UP && rays_amount < MAX_RAY_AMOUNT)
                {
                    rays_amount += 10; // Aumenta de 10 en 10

                    // Reasignar memoria para los rayos
                    struct Ray* new_rays = (struct Ray*)realloc(rays, rays_amount * sizeof(struct Ray));
                    if (!new_rays)
                    {
                        printf("Error: No se pudo reasignar memoria para los rayos.\n");
                        break;
                    }
                    rays = new_rays;

                    render_rays(circle, rays, rays_amount);
                    printf("Rayos renderizando: %d\n", rays_amount);
                }
                if (event.key.keysym.sym == SDLK_DOWN && rays_amount > 10)
                {
                    rays_amount -= 10; // Disminuye de 10 en 10

                    struct Ray* new_rays = (struct Ray*)realloc(rays, rays_amount * sizeof(struct Ray));
                    if (!new_rays)
                    {
                        printf("Error: No se pudo reasignar memoria para los rayos.\n");
                        break;
                    }
                    rays = new_rays;

                    render_rays(circle, rays, rays_amount);
                    printf("Rayos renderizando: %d\n", rays_amount);
                }
            }
        }

        // Fondo
        SDL_FillRect(surface, NULL, COLOR_BLACK);

        // Dibujar círculos
        fillCircle(surface, circle1, COLOR_RED);
        fillCircle(surface, circle, COLOR_WHITE);

        // Dibujar rayos
        fillRay(surface, rays, COLOR_YELLOW, rays_amount, circle1);

        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }

    // Liberar memoria al salir
    free(rays);
    return 0;
}
