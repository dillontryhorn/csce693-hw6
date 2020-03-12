
#include "Game.hpp"

#include <iostream>
#include "SDL2/SDL_image.h"
#include "sol/sol.hpp" //add sol header for reading in config.lua

#include "AssetManager.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include "components/TransformComponent.hpp"
#include "components/SpriteComponent.hpp"

SDL_Renderer* Game::renderer{};

EntityManager entity_mgr;
AssetManager* Game::asset_manager{new AssetManager()};

Game::Game(const char* title, int xpos, int ypos, int width, int height, bool fullscreen)
{
   Uint32 flags{};
   if (fullscreen) {
      flags = SDL_WINDOW_FULLSCREEN;
   }

   if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
      std::cout << "Subsystems initialized..." << std::endl;
      window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
      if (window) {
         std::cout << "Window created..." << std::endl;
      }
      renderer = SDL_CreateRenderer(window, -1, 0);
      if (renderer) {
         SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
         std::cout << "Renderer created..." << std::endl;
      }
   is_running = true;
   } else {
      is_running = false;
   }

   load_level(1);
}

Game::~Game()
{
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   SDL_Quit();
}

void Game::initialize() //calls initialize of entity manager
{
   entity_mgr.initialize();
}

void Game::handle_events()
{
   SDL_Event event;
   SDL_PollEvent(&event);
   switch (event.type) {
      case SDL_QUIT:
         is_running = false;
         break;
      default:
         break;
   }
}

void Game::update(const float dt)
{
   entity_mgr.update(dt);
}

void Game::render()
{
   SDL_RenderClear(renderer);
   entity_mgr.render();
   SDL_RenderPresent(renderer);
}

void Game::load_level(const int number)
{
   sol::state lua; //open a lua state
   lua.open_libraries(sol::lib::base, sol::lib::table);
   lua.script_file("config.lua"); //and go through config.lua file
   
   //Look through config.lua and create defined assets - complete
   sol::lua_table assets = lua["assets"];
   for(const auto& key_value_pair : assets) {
      sol::object k = key_value_pair.first;
      sol::object v = key_value_pair.second;
      asset_manager->add_texture(k.as<std::string>(), v.as<std::string>().c_str());
   }

   //Create entities based on tables in config.lua - not complete
   sol::lua_table entities = lua["entities"];
   for(const auto& key_value_pair : assets) {
      sol::object k = key_value_pair.first;
      sol::object v = key_value_pair.second;
      Entity& new_entity(entity_mgr.add_entity(k.as<std::string>()));

      //new_entity.add_component<TransformComponent>();
      //new_entity.add_component<SpriteComponent>();
   }

   // DELETE THIS CODE WHEN THE FOR LOOP IS FINISHED
   // Entity& tank_entity(entity_mgr.add_entity("tank"));
   // tank_entity.add_component<TransformComponent>(0,0,20,20,32,32,1);
   // tank_entity.add_component<SpriteComponent>("tank-image");
   // Entity& chopper_entity(entity_mgr.add_entity("chopper"));
   // chopper_entity.add_component<TransformComponent>(240, 106, 0, 0, 32, 32, 1);
   // chopper_entity.add_component<SpriteComponent>("chopper-image");

   entity_mgr.list_all_entities();
}
