#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <enet/enet.h>
#include <SDL3/SDL.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

class player_class {
public:
    float x = 0;
    float y = 0;
    float xv = 0;
    float yv = 0;
    float speed = 10;
    int size = 100;
};

class planet_class {
public:
    int x;
    int y;
    int size;
    int index;
};


bool initialize() {

    if (enet_initialize() < 0) {
        std::cerr << "Enet could not initialize !" << std::endl;
        return false;
    }
    atexit(enet_deinitialize);





    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL could not initialize !" << std::endl;
        return -1;
    }


    if (!TTF_Init()) {
        std::cerr << "TTF could not initialize !" << std::endl;
        SDL_Quit();
        return false;
    }

    return true;
}



bool cleanup(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font, SDL_Surface* textSurface, SDL_Texture* textTexture) {


	if (font) {
		TTF_CloseFont(font);
	}

	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}

	if (window) {
		SDL_DestroyWindow(window);
	}

    if (textSurface) {
        SDL_DestroySurface(textSurface);
    }

    if (textTexture) {
        SDL_DestroyTexture(textTexture);
    }

	TTF_Quit();
	SDL_Quit();

	return true;
}








int main() {

	if (!initialize()) {
		std::cerr << "Initialization failed!" << std::endl;
		return -1;
	}



    player_class player;

	std::vector<planet_class> planets;




    ENetHost* client;

    client = enet_host_create(NULL, 1, 1, 0, 0);

    if (!client) {
        std::cerr << "Enet could not create a client host." << std::endl;
        return -1;
    }

    ENetAddress address;
    ENetEvent enet_event;
    ENetPeer* peer;


    enet_address_set_host(&address, "88.163.97.247");
    address.port = 16383;

    peer = enet_host_connect(client, &address, 1, 0);

    if (!peer) {
        std::cerr << "Enet could not establish connection." << std::endl;
        return -1;
    }


    if (enet_host_service(client, &enet_event, 5000) > 0 && enet_event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connection succeeded !" << std::endl;
    }
    else {
        enet_peer_reset(peer);
        std::cout << "Connection failed !" << std::endl;
        return -1;
    }

    bool wait = true;

    while (wait) {
		while (enet_host_service(client, &enet_event, 1000) > 0) {
			switch (enet_event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
				std::cout << "Successfully received a packet !" << std::endl;


				size_t planets_count = enet_event.packet->dataLength / sizeof(planet_class);

				planets.resize(planets_count);

				std::memcpy(planets.data(), enet_event.packet->data, enet_event.packet->dataLength);

				wait = false;

				break;
			}
		}
    }










    SDL_DisplayID* displays;
    int display_count;
    displays = SDL_GetDisplays(&display_count);
    if (!displays) {
        std::cerr << "Failed to get displays: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    const SDL_DisplayMode* dm;
    dm = SDL_GetDesktopDisplayMode(displays[0]);
    if (!dm) {
        std::cerr << "Failed to get display mode: " << SDL_GetError() << std::endl;
        SDL_free(displays);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    int w = dm->w;
    int h = dm->h;
    SDL_free(displays);

    SDL_Window* window = SDL_CreateWindow("Space game", w, h, SDL_WINDOW_FULLSCREEN);
    if (!window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }


    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");



    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Load font
    TTF_Font* font = TTF_OpenFont("VCR_OSD_MONO.ttf", 24.0f);
    if (!font) {
        std::cerr << "Failed to load font: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }




    std::vector<SDL_Surface*> planet_images;

    // Load planet images
    for (int i = 1; i <= 5; ++i) {
        std::string path = "assets/planets/" + std::to_string(i) + ".png";
        SDL_Surface* image = IMG_Load(path.c_str());
        if (!image) {
            std::cerr << "Failed to load " << path << std::endl;
            return -1;
        }
        planet_images.push_back(image);
    }










    std::string fps_string = "FPS: --";
    SDL_Color textColor = { 200, 200, 200, 255 };
    SDL_Surface* textSurface = nullptr;
    SDL_Texture* textTexture = nullptr;
    SDL_FRect textRect = { 10, 10, 0, 0 };








    SDL_Event event;
    float zoom = 1;
    bool run = true;
    Uint64 old_ticks = SDL_GetTicks();

    float time = 0.0f;
    int frameCount = 0;
    int currentFPS = 0;








    while (run) {



        Uint64 ticks = SDL_GetTicks();
        float dt = (ticks - old_ticks) / 1000.0f;
        old_ticks = ticks;

        // FPS calculation
        time += dt;
        frameCount++;





        if (time >= 1.0f) { // Update FPS every second
            currentFPS = frameCount;
            fps_string = "FPS: " + std::to_string(currentFPS);
            frameCount = 0;

            // Update text texture
            if (textSurface) {
                SDL_DestroySurface(textSurface);
                textSurface = nullptr;
            }
            if (textTexture) {
                SDL_DestroyTexture(textTexture);
                textTexture = nullptr;
            }

            // Create new text surface and texture
            textSurface = TTF_RenderText_Blended(font, fps_string.c_str(), fps_string.length(), textColor);
            if (!textSurface) {
                std::cerr << "Failed to render text surface: " << SDL_GetError() << std::endl;
                continue;
            }

            textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (!textTexture) {
                std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
                SDL_DestroySurface(textSurface);
                textSurface = nullptr;
                continue;
            }

            textRect.w = textSurface->w;
            textRect.h = textSurface->h;
        }













        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                run = false;
            }
        }



        while (enet_host_service(client, &enet_event, 1) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                std::cout << "Successfully received a packet !" << std::endl;
                break;
            }
        }









        // Keyboard input
        const bool* state = SDL_GetKeyboardState(NULL);

        if (state[SDL_SCANCODE_ESCAPE]) {
            run = false;
        }
        if (state[SDL_SCANCODE_UP]) {
            player.yv += player.speed;
        }
        if (state[SDL_SCANCODE_DOWN]) {
            player.yv -= player.speed;
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            player.xv += player.speed;
        }
        if (state[SDL_SCANCODE_LEFT]) {
            player.xv -= player.speed;
        }
        if (state[SDL_SCANCODE_W]) {
            zoom *= 1.0f + (0.5f * dt);
        }
        if (state[SDL_SCANCODE_S]) {
            zoom *= 1.0f - (0.3f * dt);
        }








        // Gravitational effects
        for (int i = 0; i < planets.size(); i++) {
            float dx = planets[i].x - player.x;
            float dy = planets[i].y - player.y;
            float distanceSquared = dx * dx + dy * dy;

            // Avoid division by zero or extremely small distances
            if (distanceSquared < 1.0f) {
                distanceSquared = 1.0f;
            }

            // Scale the gravitational force to make it more noticeable
            float gravitationalConstant = 10000.0f; // Adjust this value as needed
            float force = (gravitationalConstant * planets[i].size) / distanceSquared;
            float distance = sqrt(distanceSquared);

            // Normalize the direction and apply the force
            player.xv += force * (dx / distance);
            player.yv += force * (dy / distance);
        }







        // Collision detection
        for (int i = 0; i < planets.size(); i++) {
            if (sqrt(pow(planets[i].x - player.x, 2) + pow(planets[i].y - player.y, 2)) < planets[i].size / 2.0f) {
                run = false;
            }
        }






        // Update player position and velocity
        player.x += player.xv * dt;
        player.y += player.yv * dt;
        player.xv *= pow(0.9f, dt);
        player.yv *= pow(0.9f, dt);


        if (time >= 1.0f) {

            char buffer[sizeof(float) * 2];

            memcpy(buffer, &player.x, sizeof(float));
			memcpy(buffer + sizeof(float), &player.y, sizeof(float));

            ENetPacket* packet = enet_packet_create(buffer, sizeof(buffer), ENET_PACKET_FLAG_RELIABLE);

            if (enet_peer_send(peer, 0, packet) != 0) {
                std::cerr << "Failed to send packet!" << std::endl;
                enet_packet_destroy(packet);
            }

            time = 0.0f;

        }









        // Render background
        SDL_SetRenderDrawColor(renderer, 0, 0, 50, 255);
        SDL_RenderClear(renderer);

        // Render planets
        for (int i = 0; i < planets.size(); i++) {

            //Draw the planets images :
			SDL_Surface* planet_image = planet_images[planets[i].index];
			SDL_Texture* planet_texture = SDL_CreateTextureFromSurface(renderer, planet_image);

			if (!planet_texture) {
				std::cerr << "Failed to create texture from surface: " << SDL_GetError() << std::endl;
				return -1;
			}

            
            SDL_FRect planet_rect = {
                (w / 2) + ((planets[i].x - player.x) / zoom) - ((planets[i].size / zoom) / 2),
                (h / 2) - ((planets[i].y - player.y) / zoom) - ((planets[i].size / zoom) / 2),
                planets[i].size / zoom,
                planets[i].size / zoom };


			SDL_RenderTexture(renderer, planet_texture, NULL, &planet_rect);
        }






        // Render player
        SDL_FRect player_rect = {
            (w / 2) - ((player.size / zoom) / 2),
            (h / 2) - ((player.size / zoom) / 2),
            player.size / zoom, player.size / zoom };

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &player_rect);




        // Render FPS text
        if (textTexture) {
            SDL_RenderTexture(renderer, textTexture, NULL, &textRect);
        }







        // Present renderer
        SDL_RenderPresent(renderer);
    }









    enet_peer_disconnect(peer, 0);

    while (enet_host_service(client, &enet_event, 3000) > 0) {
        switch (enet_event.type) {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy(enet_event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            std::cout << "Disconnection succeeded !" << std::endl;
            break;
        }
    }
    







    if (!cleanup(window, renderer, font, textSurface, textTexture)) {
		std::cerr << "Cleanup failed!" << std::endl;
		return -1;
    }









    return 0;
}