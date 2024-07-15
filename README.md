# DungGine

`DungGine` is a terminal based dungeon engine for RPGs and Rogue-likes and that uses [`Termin8or`](https://github.com/razterizer/Termin8or) for rendering.
This is a header only library.
The engine works very well together with the `GameEngine` class of lib [`Termin8or`](https://github.com/razterizer/Termin8or).

This library is very new and currently only provides two classes: `BSPTree` that is responsible for creating the rooms, corridors and doors in the "dungeon", and then there is the class `DungGine` that is the dungeon game engine itself.

## Headers

* `BSPTree.h`
  - `BSPTree(int min_room_length)` : The constructor.
  - `generate(int world_size_rows, int world_size_cols,
                  Orientation first_split_orientation)` : Generates the BSP regions recursively.
  - `pad_rooms(int min_rnd_wall_padding = 1, int max_rnd_wall_padding = 4)` : Pads the regions into rooms.
  - `create_corridors(int min_corridor_half_width = 1)` : Non-recursive method of creating corridors on leaf-level.
  - `create_doors(int max_num_locked_doors, bool allow_passageways)` : Creates doors between rooms and corridors. You need to first have called `generate()`, `pad_rooms()` and `create_corridors()` before calling this function.
  - `draw_regions(SpriteHandler<NR, NC>& sh, int r0 = 0, int c0 = 0, const styles::Style& border_style = { Color::Black, Color::Yellow })` : Draws the regions.
  - `draw_rooms(SpriteHandler<NR, NC>& sh, int r0 = 0, int c0 = 0, const styles::Style& room_style = { Color::White, Color::DarkRed })` : Draws the rooms.
  - `draw_corridors(SpriteHandler<NR, NC>& sh, int r0 = 0, int c0 = 0, const styles::Style& corridor_outline_style = { Color::Green, Color::DarkGreen }, const styles::Style& corridor_fill_style = { Color::Black, Color::Green })` : Draws the non-recursive corridors.
  - `print_tree()` : Debug printing of the tree.
  - `fetch_leaves()` : Fetches the leaves of the BSP tree where the rooms are stored.
  - `get_room_corridor_map()` : Function that retrieves the room and corridor relationship data structure.
  - `get_world_size()` : Gets the world size.
  - `fetch_doors()` : Gets a vector of pointers to all doors.
* `DungGine.h`
  - `DungGine(const std::string& exe_folder, bool use_fow, DungGineTextureParams texture_params = {})` : The constructor.
  - `load_dungeon(BSPTree* bsp_tree)` : Loads a generated BSP tree.
  - `style_dungeon()` : Performs automated styling of rooms in the dungeon / realm.
  - `set_player_character(char ch)` : Sets the character of the playable character (pun intended).
  -  `set_player_style(const Style& style)` : Sets the style (fg/bg color) of the playable character.
  - `place_player(const RC& screen_size, std::optional<RC> world_pos = std::nullopt)` : Places the player near the middle of the realm in one of the corridors and centers the screen around the player.
  - `configure_sun(float sun_t_offs, float minutes_per_day, Latitude latitude = Latitude::High, Season season = Season::Spring)` : Configures the speed of the solar day and the starting direction of the sun. Used for shadow movements for rooms over ground.
  - `configure_sun(float minutes_per_day, Latitude latitude = Latitude::High, Season season = Season::Spring)` : Same as above but randomizes the initial direction of the sun.
  - `place_keys()` : Places the keys in rooms, randomly all over the world.
  - `place_lamps(int num_lamps)` : Places `num_lamps` lamps in rooms, randomly all over the world.
  - `set_screen_scrolling_mode(ScreenScrollingMode mode, float t_page = 0.2f)` : Sets the screen scrolling mode to either `AlwaysInCentre`, `PageWise` or `WhenOutsideScreen`. `t_page` is used with `PageWise` mode.
  - `update(double real_time_s, const keyboard::KeyPressData& kpd)` : Updating the state of the dungeon engine. Manages things such as the change of direction of the sun for the shadows of rooms that are not under the ground and key-presses for control of the playable character.
  - `draw(SpriteHandler<NR, NC>& sh, double real_time_s)` : Draws the rooms of the dungeon / realm (will include drawing of corridors in the near(?) future).

## Examples

```cpp
dung::BSPTree::draw_regions()
```

<img width="574" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/d3dba22a-f41d-482b-9a8f-0490e6f24835">

***

```cpp
dung::BSPTree::draw_rooms()
```

<img width="560" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/b4b672a8-9ce5-4d74-8d71-c23b37ac1042">


```cpp
dung::BSPTree::draw_rooms()
dung::BSPTree::draw_corridors()
```

<img width="573" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/fd23bf8e-17b2-4055-84d6-d6bdd87538ef">

***

```cpp
dung::BSPTree bsp_tree { 4 }; // argument: `min_room_length = 4`.
bsp_tree.generate(29, 79, dung::Orientation::Vertical); // arguments: world_size_rows, world_size_cols,
                  first_split_orientation.
bsp_tree.pad_rooms(4); // arguments: min_rnd_wall_padding = 4, [max_rnd_wall_padding = 4].
bsp_tree.create_corridors(1); // argument: min_corridor_half_width = 1, (1 means it will be three chars wide).

Text t;
SpriteHandler<NR, NC> sh;
Color bg_color = Color::Default;

dung::DungGine dungeon_engine;
dungeon_engine.load_dungeon(&bsp_tree);
dungeon_engine.style_dungeon();
dungeon_engine.draw(sh);
sh.print_screen_buffer(t, bg_color);
```

<img width="560" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/7417d29d-1d2a-47b6-b926-31a9ae0177b2">

***

```cpp
// Declarations
dung::BSPTree bsp_tree { 4 }; // argument: `min_room_length = 4`.
dung::DungGineTextureParams texture_params;
std::unique_ptr<dung::DungGine> dungeon_engine;
Text t;
SpriteHandler<NR, NC> sh;
Color bg_color = Color::Black;

// Initializations
bsp_tree.generate(200, 400, dung::Orientation::Vertical); // arguments: world_size_rows, world_size_cols,
                  first_split_orientation.
bsp_tree.pad_rooms(4); // arguments: min_rnd_wall_padding = 4, [max_rnd_wall_padding = 4].
bsp_tree.create_corridors(1); // argument: min_corridor_half_width = 1, (1 means it will be three chars wide).
bsp_tree.create_doors(100, true); // We also create doors here. Arguments: max_num_locked_doors, allow_passageways.

dungeon_engine = std::make_unique<dung::DungGine>(get_exe_folder(), true); // arguments: exe_folder, use_fow, texture_params.
dungeon_engine.load_dungeon(&bsp_tree);
dungeon_engine.style_dungeon();
if (!dungeon_engine.place_player(sh.size()))
  std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
dungeon_engine.configure_sun(20.f);
dungeon_engine->place_keys();
dungeon_engine->place_lamps(20);
dungeon_engine.set_screen_scrolling_mode(ScreenScrollingMode::WhenOutsideScreen);

// In game loop:
sh.clear();
dungeon_engine->update(get_real_time_s(), kpd); // arg0 : time from game start, arg1 : keyboard::KeyPressData object.
dungeon_engine->draw(sh, get_real_time_s());
sh.print_screen_buffer(t, bg_color);
```

<img width="566" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/b24d58d7-6ad5-4063-881d-650dd7bc905a">

Note the playable character marked as a "@" in the centre of the screen.
To move the character in a game loop, use function `update()` to allow keystrokes to be registered. Then control the character by pressing the ASWD keys. Press space-bar to open and close doors that you are located next to.

***

```cpp
// Declarations
dung::BSPTree bsp_tree { 4 }; // argument: `min_room_length = 4`.
dung::DungGineTextureParams texture_params;
std::unique_ptr<dung::DungGine> dungeon_engine;
Text t;
SpriteHandler<NR, NC> sh;
Color bg_color = Color::Black;

// Initializations
bsp_tree.generate(200, 400, dung::Orientation::Vertical); // arguments: world_size_rows, world_size_cols,
                  first_split_orientation.
bsp_tree.pad_rooms(4); // arguments: min_rnd_wall_padding = 4, [max_rnd_wall_padding = 4].
bsp_tree.create_corridors(1); // argument: min_corridor_half_width = 1, (1 means it will be three chars wide).
bsp_tree.create_doors(100, true); // We also create doors here. Arguments: max_num_locked_doors, allow_passageways.

texture_params.dt_anim_s = 0.5;
auto f_tex_path = [](const auto& filename)
{
return folder::join_path({ "textures", filename });
};
texture_params.texture_file_names_surface_level_fill.emplace_back(f_tex_path("texture_sl_fill_0.tex"));
texture_params.texture_file_names_surface_level_fill.emplace_back(f_tex_path("texture_sl_fill_1.tex"));
texture_params.texture_file_names_surface_level_shadow.emplace_back(f_tex_path("texture_sl_shadow_0.tex"));
texture_params.texture_file_names_surface_level_shadow.emplace_back(f_tex_path("texture_sl_shadow_1.tex"));

dungeon_engine = std::make_unique<dung::DungGine>(get_exe_folder(), true, texture_params); // arguments: exe_folder, use_fow, texture_params.
dungeon_engine.load_dungeon(&bsp_tree);
dungeon_engine.style_dungeon();
if (!dungeon_engine.place_player(sh.size()))
  std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
dungeon_engine.configure_sun(20.f);
dungeon_engine->place_keys();
dungeon_engine->place_lamps(20);
dungeon_engine.set_screen_scrolling_mode(ScreenScrollingMode::WhenOutsideScreen);

// In game loop:
sh.clear();
dungeon_engine->update(get_real_time_s(), kpd); // arg0 : time from game start, arg1 : keyboard::KeyPressData object.
dungeon_engine->draw(sh, get_real_time_s());
sh.print_screen_buffer(t, bg_color);
```
