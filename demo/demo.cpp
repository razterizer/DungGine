//
//  demo.cpp
//  DungGine
//
//  Created by Rasmus Anthin on 2024-08-05.
//

#include <Termin8or/GameEngine.h>
#include <DungGine/BSPTree.h>
#include <DungGine/DungGine.h>
#include <DungGine/DungGineListener.h>

#include <iostream>

enum class TestType { DungeonSimple, DungeonRuntime };
static TestType test_type = TestType::DungeonRuntime;


class Game : public GameEngine<>, public dung::DungGineListener
{
public:
  Game(int argc, char** argv, const GameEngineParams& params)
    : GameEngine(argv[0], params)
  {
    GameEngine::set_real_fps(15);
    GameEngine::set_sim_delay_us(10'000);
    GameEngine::set_anim_rate(0, 3); // swim animation
    GameEngine::set_anim_rate(1, 4); // fight animation
  }
  
  virtual ~Game() override
  {
    dungeon_engine->remove_listener(this);
  }
  
  virtual void on_pc_death() override
  {
    // Play sound.
  }
  
  virtual void on_npc_death() override
  {
    // Play sound.
  }
  
  virtual void on_fight_begin(dung::NPC* UNUSED(npc)) override
  {
    // Play sound.
  }
  
  virtual void on_fight_end(dung::NPC* UNUSED(npc)) override
  {
    // Stop sound.
  }
  
  virtual void generate_data() override
  {
    // /////////
    if (test_type == TestType::DungeonSimple)
    {
      std::cout << "curr rnd seed = " << curr_rnd_seed << std::endl;
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
      sh.print_screen_buffer(bg_color);
      
      //sh.clear();
      //bsp_tree.draw_rooms(sh);
      //bsp_tree.draw_regions(sh, 0, 0);
      //sh.print_screen_buffer(t, bg_color);
      
      sh.clear();
      bsp_tree.draw_corridors(sh);
      bsp_tree.draw_regions(sh, 0, 0);
      bsp_tree.draw_rooms(sh);
      sh.print_screen_buffer(bg_color);
      
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
      dungeon_engine->update(get_frame_count(), get_real_fps(),
                             get_real_time_s(), get_sim_time_s(), get_sim_dt_s(),
                             fire_smoke_dt_factor,
                             kpdp, &game_over);
      dungeon_engine->draw(sh, get_real_time_s(), get_sim_time_s(),
                           get_anim_count(0), get_anim_count(1));
      sh.print_screen_buffer(Color::Black);
#endif
    }
    else if (test_type == TestType::DungeonRuntime)
    {
      std::cout << "curr rnd seed = " << curr_rnd_seed << std::endl;
      //rnd::srand(1626475275); // Terrain offset bug.
      //rnd::srand(3074848586); // Torch fire-smoke test.
      //rnd::srand(4133950669); // Drown animation test.
      //rnd::srand(1905630639); // Dev days demo 1.
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
      dungeon_engine->configure_sun_rand(10.f, 3*60.f, dung::Latitude::Equator, dung::Longitude::F, true);
      dungeon_engine->style_dungeon();
      if (!dungeon_engine->place_player(sh.size()))
        std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
      dungeon_engine->place_keys(true);
      dungeon_engine->place_lamps(30, 15, 5, true);
      dungeon_engine->place_weapons(150, true);
      dungeon_engine->place_potions(100, true);
      dungeon_engine->place_armour(150, true);
      dungeon_engine->place_npcs(100, true);
      
      dungeon_engine->add_listener(this);
    }
  }
  
private:
  virtual void update() override
  {
    if (test_type == TestType::DungeonRuntime)
    {
      bool game_over = false;
      dungeon_engine->update(get_frame_count(), get_real_fps(),
                             get_real_time_s(), get_sim_time_s(), get_sim_dt_s(),
                             fire_smoke_dt_factor,
                             kpdp, &game_over);
      if (game_over)
        set_state_game_over();
      
      if (framed_mode)
        draw_frame(sh, Color::White);
      
      dungeon_engine->draw(sh, get_real_time_s(), get_sim_time_s(),
                           get_anim_count(0), get_anim_count(1),
                           ui::VerticalAlignment::CENTER, ui::HorizontalAlignment::CENTER,
                           4, 0, framed_mode, use_gore);
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
  
  bool framed_mode = false;
  bool use_gore = true;
  
  float fire_smoke_dt_factor = 0.5f;
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
  
  for (int i = 1; i < argc; ++i)
  {
    if (strcmp(argv[i], "--help") == 0)
    {
      std::cout << "demo --help | [--log_mode (record | replay)] [--suppress_tty_output] [--suppress_tty_input]" << std::endl;
      return EXIT_SUCCESS;
    }
    
    if (strcmp(argv[i],  "--suppress_tty_output") == 0)
      params.suppress_tty_output = true;
    else if (strcmp(argv[i], "--suppress_tty_input") == 0)
      params.suppress_tty_input = true;
    else if (i + 1 < argc && strcmp(argv[i], "--log_mode") == 0)
    {
      if (strcmp(argv[i + 1], "record") == 0)
        params.log_mode = LogMode::Record;
      else if (strcmp(argv[i + 1], "replay") == 0)
        params.log_mode = LogMode::Replay;
      params.xcode_log_filepath = "../../../../../../../../Documents/xcode/lib/DungGine/demo";
    }
  }

  Game game(argc, argv, params);

  game.init();
  game.generate_data();
  if (test_type != TestType::DungeonSimple)
    game.run();

  return EXIT_SUCCESS;
}
