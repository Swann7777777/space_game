#include <SDL3/SDL.h>
#include <iostream>
#include <vector>
#include <random>


class player_class {
public:
	float x = 0;
	float y = 0;
	float xv = 0;
	float yv = 0;
	float speed = 10;
	int size = 50;
};

class planet {
public:
	int x;
	int y;
	int size;
	int mass;
	int r;
	int g;
	int b;
};


class camera {
public:
	float x;
	float y;
	int height;
};


int main() {

	player_class player;

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<> pos_dist(-10000, 10000);
	std::uniform_int_distribution<> size_dist(20, 200);
	std::uniform_int_distribution<> mass_dist(10, 100);
	std::uniform_int_distribution<> color_dist(0, 255);



	int w = 0;
	int h = 0;

	std::vector<planet> planets;
	for (int i = 0; i < 100; i++) {
		planets.push_back({ pos_dist(gen), pos_dist(gen), size_dist(gen), mass_dist(gen), color_dist(gen), color_dist(gen), color_dist(gen)});
	}

	SDL_Init(SDL_INIT_VIDEO);

	SDL_DisplayID* displays;
	displays = SDL_GetDisplays(NULL);
	const SDL_DisplayMode* dm;
	dm = SDL_GetDesktopDisplayMode(displays[0]);

	w = dm->w;
	h = dm->h;


	SDL_Window* window = SDL_CreateWindow("Space game", w, h, SDL_WINDOW_FULLSCREEN);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

	SDL_Event event;

	float zoom = 1;

	bool run = true;

	Uint64 old_ticks = 0;

	while (run) {
		
		
		Uint64 ticks = SDL_GetTicks();

		float dt = (ticks - old_ticks) / 1000.0f;		
		old_ticks = ticks;
		
		
		
		
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				run = false;
			}
		}


		const bool* state = SDL_GetKeyboardState(NULL);

		if (state[SDL_SCANCODE_ESCAPE]) {
			run = false;
		}
		if (state[SDL_SCANCODE_UP]) {
			player.yv += player.speed / zoom;
		}
		if (state[SDL_SCANCODE_DOWN]) {
			player.yv -= player.speed / zoom;
		}
		if (state[SDL_SCANCODE_RIGHT]) {
			player.xv += player.speed / zoom;
		}
		if (state[SDL_SCANCODE_LEFT]) {
			player.xv -= player.speed / zoom;
		}
		if (state[SDL_SCANCODE_W]) {
			zoom /= pow(1.1f, dt);
		}
		if (state[SDL_SCANCODE_S]) {
			zoom *= pow(1.3f, dt);
		}

		std::cout << zoom << "\n";

		player.x += player.xv * dt;
		player.y += player.yv * dt;
		player.xv *= pow(0.7f, dt);
		player.yv *= pow(0.7f, dt);



		SDL_FRect player_rect = { (w / 2) - ((player.size / zoom) / 2), (h / 2) - ((player.size / zoom) / 2), player.size / zoom, player.size / zoom};


		SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255);
		SDL_RenderClear(renderer);





		for (int i = 0; i < planets.size(); i++) {
			SDL_FRect planet_rect = { (w / 2) - ((planets[i].size / zoom) / 2) + (planets[i].x / zoom) - player.x, ((h / 2) - (planets[i].size / zoom) / 2) - (planets[i].y / zoom) + player.y, planets[i].size / zoom, planets[i].size / zoom};
			SDL_SetRenderDrawColor(renderer, planets[i].r, planets[i].g, planets[i].b, 255);
			SDL_RenderFillRect(renderer, &planet_rect);
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

		SDL_RenderFillRect(renderer, &player_rect);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}