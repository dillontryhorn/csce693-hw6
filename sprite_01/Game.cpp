
#include "Game.hpp"

#include <iostream>
#include <map>
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
   
   //Look through config.lua and create defined assets
   sol::lua_table assets = lua["assets"];
   for(const auto& key_value_pair : assets) {
      sol::object k = key_value_pair.first;
      sol::object v = key_value_pair.second;
      asset_manager->add_texture(k.as<std::string>(), v.as<std::string>().c_str());
   }

   //Create entities based on tables in config.lua
   sol::lua_table entities = lua["entities"];
   for(const auto& key_value_pair : entities) { //higher level, type of world object to be made
      sol::object k = key_value_pair.first;
      sol::object v = key_value_pair.second;
      Entity& new_entity(entity_mgr.add_entity(k.as<std::string>())); //create a new entity based on type defined in lua file

      //Now we need to create and modify the components
      sol::lua_table components = entities[k.as<std::string>()];
      for(const auto& innerKVP : components) { //create components defined in config.lua
         sol::object key = innerKVP.first;
         sol::object val = innerKVP.second;

         sol::lua_table members = components[key.as<std::string>()]; //create new lua table to parse
         std::map<std::string, std::string> varmap; //map is used because the args usually come in different orders every time
         for(const auto& params : members) {
            varmap[params.first.as<std::string>()] = params.second.as<std::string>(); //create a map to hold the key and values of component attributes
         }

         if(key.as<std::string>() == "transform"){ //create transform with value from config.lua
            new_entity.add_component<TransformComponent>(std::stoi(varmap.find("position_x")->second), 
                                                         std::stoi(varmap.find("position_y")->second),
                                                         std::stoi(varmap.find("velocity_x")->second),
                                                         std::stoi(varmap.find("velocity_y")->second),
                                                         std::stoi(varmap.find("width")->second),
                                                         std::stoi(varmap.find("height")->second),
                                                         std::stoi(varmap.find("scale")->second));
         }
         else if(key.as<std::string>() == "sprite"){ //create sprite from config.lua
            new_entity.add_component<SpriteComponent>(varmap.find("texture_id")->second);
         }
         else{
            std::cout << "unknown component type\n"; //error
         }
      }
   }
   entity_mgr.list_all_entities();
}
