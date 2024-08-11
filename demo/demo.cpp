//
//  demo.cpp
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-05.
//

#include <Termin8or/GameEngine.h>
#include <DungGine/BSPTree.h>
#include <DungGine/DungGine.h>

#include <iostream>

enum class TestType { DungeonSimple, DungeonRuntime };
static TestType test_type = TestType::DungeonRuntime;


class Game : public GameEngine<>
{
public:
  Game(int argc, char** argv, const GameEngineParams& params)
    : GameEngine(argv[0], params)
  {}
  
  virtual void generate_data() override
  {
    // /////////
    if (test_type == TestType::DungeonSimple)
    {
      //rnd::srand(0x1337f00d); // Use srand_time() after map generation.
      
      bsp_tree.generate(29, 79, dung::Orientation::Vertical);
      bsp_tree.pad_rooms(4);
      bsp_tree.create_corridors(1);
      bsp_tree.create_doors(0, true);
      
      bsp_tree.print_tree();
      
      clear_screen();
      return_cursor();
      
      sh.clear();
      bsp_tree.draw_rooms(sh);
      bsp_tree.draw_corridors(sh);
      sh.print_screen_buffer(t, bg_color);
      
      //sh.clear();
      //bsp_tree.draw_rooms(sh);
      //bsp_tree.draw_regions(sh, 0, 0);
      //sh.print_screen_buffer(t, bg_color);
      
      sh.clear();
      bsp_tree.draw_corridors(sh);
      bsp_tree.draw_regions(sh, 0, 0);
      bsp_tree.draw_rooms(sh);
      sh.print_screen_buffer(t, bg_color);
      
      //sh.clear();
      //bsp_tree.draw_regions(sh, 0, 0);
      //sh.print_screen_buffer(t, bg_color);

#if 1
      dungeon_engine = std::make_unique<dung::DungGine>(get_exe_folder(), false);
      dungeon_engine->load_dungeon(&bsp_tree);
      dungeon_engine->style_dungeon();
      if (!dungeon_engine->place_player(sh.size()))
        std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
      dungeon_engine->configure_sun(20.f);
      dungeon_engine->place_keys(true);
      
      sh.clear();
      bool game_over = false;
      dungeon_engine->update(get_real_time_s(), get_sim_dt_s(), kpd, &game_over);
      dungeon_engine->draw(sh, get_real_time_s(), anim_ctr);
      sh.print_screen_buffer(t, Color::Black);
#endif
    }
    else if (test_type == TestType::DungeonRuntime)
    {
      //rnd::srand(0x1337f00d + 5); // Use srand_time() after map generation.
      
      bsp_tree.generate(200, 400, dung::Orientation::Vertical);
      bsp_tree.pad_rooms(4);
      bsp_tree.create_corridors(1);
      bsp_tree.create_doors(50, true);
      
      texture_params.dt_anim_s = 0.5;
      auto f_tex_path = [](const auto& filename)
      {
        return folder::join_path({ "textures", filename });
      };
      texture_params.texture_file_names_surface_level_fill.emplace_back(f_tex_path("texture_sl_fill_0.tex"));
      texture_params.texture_file_names_surface_level_fill.emplace_back(f_tex_path("texture_sl_fill_1.tex"));
      texture_params.texture_file_names_surface_level_shadow.emplace_back(f_tex_path("texture_sl_shadow_0.tex"));
      texture_params.texture_file_names_surface_level_shadow.emplace_back(f_tex_path("texture_sl_shadow_1.tex"));
    
      dungeon_engine = std::make_unique<dung::DungGine>(get_exe_folder(), true, texture_params);
      dungeon_engine->load_dungeon(&bsp_tree);
      //dungeon_engine->configure_sun(0.75f, 1e6f, dung::Season::Summer, 1e6f, dung::Latitude::NorthernHemisphere, dung::Longitude::F, false);
      dungeon_engine->configure_sun_rand(2.f, 1e6f, dung::Latitude::Equator, dung::Longitude::F, true);
      dungeon_engine->style_dungeon();
      if (!dungeon_engine->place_player(sh.size()))
        std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
      dungeon_engine->place_keys(true);
      dungeon_engine->place_lamps(20);
      dungeon_engine->place_weapons(150);
      dungeon_engine->place_potions(150);
      dungeon_engine->place_armour(150);
      dungeon_engine->place_npcs(100);
    }
  }
  
private:
  virtual void update() override
  {
    if (test_type == TestType::DungeonRuntime)
    {
      bool game_over = false;
      dungeon_engine->update(get_real_time_s(), get_sim_dt_s(), kpd, &game_over);
      if (game_over)
        set_state_game_over();
      
      dungeon_engine->draw(sh, get_real_time_s(), anim_ctr);
    }
  }
  
  virtual void draw_title() override
  {
  }
  
  virtual void draw_instructions() override
  {
  }
  
  dung::BSPTree bsp_tree { 4 };
  
  dung::DungGineTextureParams texture_params;
  std::unique_ptr<dung::DungGine> dungeon_engine;
};

int main(int argc, char** argv)
{
  GameEngineParams params;
  params.enable_title_screen = false;
  params.enable_instructions_screen = false;
  params.enable_quit_confirm_screen = true;
  params.enable_hiscores = false;
  params.screen_bg_color_default = Color::Black;
  params.screen_bg_color_title = Color::DarkYellow;
  params.screen_bg_color_instructions = Color::Black;

  Game game(argc, argv, params);

  game.init();
  game.generate_data();
  if (test_type != TestType::DungeonSimple)
    game.run();

  return EXIT_SUCCESS;
}
