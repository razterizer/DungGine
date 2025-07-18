# DungGine

![GitHub License](https://img.shields.io/github/license/razterizer/DungGine?color=blue)
![Static Badge](https://img.shields.io/badge/linkage-header_only-yellow)
![Static Badge](https://img.shields.io/badge/C%2B%2B-20-yellow)

[![build ubuntu](https://github.com/razterizer/DungGine/actions/workflows/build-ubuntu.yml/badge.svg)](https://github.com/razterizer/DungGine/actions/workflows/build-ubuntu.yml)
[![build macos](https://github.com/razterizer/DungGine/actions/workflows/build-macos.yml/badge.svg)](https://github.com/razterizer/DungGine/actions/workflows/build-macos.yml)
[![build ubuntu](https://github.com/razterizer/DungGine/actions/workflows/build-windows.yml/badge.svg)](https://github.com/razterizer/DungGine/actions/workflows/build-windows.yml)

![Top Languages](https://img.shields.io/github/languages/top/razterizer/DungGine)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/DungGine)
![](https://tokei.rs/b1/github/razterizer/DungGine)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/DungGine)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/DungGine?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/DungGine?color=blue)

<img width="568" alt="image" src="https://github.com/user-attachments/assets/c3075838-7f15-4679-862e-ce47780c9b39">

<!-- ![dunggine_demo-comp](https://github.com/user-attachments/assets/b476eeb8-4d47-46f8-a5c1-403906555174)

(Gif animation produced using [`terminalizer`](https://github.com/faressoft/terminalizer) and [`gifsicle`](https://github.com/kohler/gifsicle).)
-->

### Ubuntu WSL
![ubuntu_QPClLlbOCK](https://github.com/user-attachments/assets/464e2a23-dd11-499e-9537-c422bd1a4217)

### Windows
![demo vs_YSrQHcSQqu](https://github.com/user-attachments/assets/434aa258-ea76-4143-9ddc-44f63ed0f294)

`DungGine` is a terminal based dungeon engine for RPGs and dungeon-crawlers (e.g. Rogue-likes) and that uses [`Termin8or`](https://github.com/razterizer/Termin8or) for rendering.
This is a header only library.
The engine works very well together with the `GameEngine` class of lib [`Termin8or`](https://github.com/razterizer/Termin8or).

There are two main classes for this dungeon generator: `BSPTree` that is responsible for creating the rooms, corridors and doors in the "dungeon", and then there is the class `DungGine` that is the dungeon game engine itself.
See the next section for a summary over these two classes.

## Features

There is no skill progression yet and underground rooms are currently mixed with surface-level rooms, so there are some things to improve on in the future. But it does feature:

* an inventory system
* items to pick up
* combat-system (THAC0-based)
* some gore
* light-field
* fog-of-war
* season-based and location-based wall shadows for surface level rooms
* particle physics-based fire/smoke
* material based textures. So that when you design a texture in TextUR, DungGine treats different parts of the terrain differently, such as different viscosity and friction etc
* different scrolling-modes for tracking the PC

and stuff like that.
The corridors are all straight (for now). Other than that, I think the BSP generation is pretty standard.
Items you can pick up are keys, torches, lanterns, magic lamps (isotropic light), potions (some contain poison, so gotta watch out), weapons and armour.

## Keys

* `WASD keys` or `arrow keys` to control the direction of the PC or browse inventory objects.
* `Space` to pick up items, open doors or equip items in the inventory UI.
* `-` to open and close the inventory.
* `F` to fight friendlies (hostile NPCs will always start the fight). When running away from them, they will quickly forget and forgive.
* `D` to drop a hilited item when in the inventory.
* `C` to consume a potion.
* `I` to identify things the PC can see.

## NPCs

There are a bunch of different NPCs and that have different behaviours. The following are supported:

* human
* elf
* half elf
* gnome
* halfling
* dwarf
* half orc
* ogre
* hobgoblin
* goblin
* orc
* troll
* monster
* lich
* lich king
* basilisk
* bear
* kobold
* skeleton
* giant
* huge spider
* wolf
* wyvern
* griffin
* ghoul
* dragon

They all look different visually and have different walking styles etc.
Some can swim, some can fly, and some can only walk.

## Headers

* `BSPTree.h`
  - `BSPTree(int min_room_length)` : The constructor.
  - `generate(int world_size_rows, int world_size_cols,
                  Orientation first_split_orientation)` : Generates the BSP regions recursively.
  - `pad_rooms(int min_rnd_wall_padding = 1, int max_rnd_wall_padding = 4)` : Pads the regions into rooms.
  - `create_corridors(int min_corridor_half_width = 1)` : Non-recursive method of creating corridors on leaf-level.
  - `create_doors(int max_num_locked_doors, bool allow_passageways)` : Creates doors between rooms and corridors. You need to first have called `generate()`, `pad_rooms()` and `create_corridors()` before calling this function.
  - `draw_regions(ScreenHandler<NR, NC>& sh, int r0 = 0, int c0 = 0, const styles::Style& border_style = { Color::Black, Color::Yellow })` : Draws the regions.
  - `draw_rooms(ScreenHandler<NR, NC>& sh, int r0 = 0, int c0 = 0, const styles::Style& room_style = { Color::White, Color::DarkRed })` : Draws the rooms.
  - `draw_corridors(ScreenHandler<NR, NC>& sh, int r0 = 0, int c0 = 0, const styles::Style& corridor_outline_style = { Color::Green, Color::DarkGreen }, const styles::Style& corridor_fill_style = { Color::Black, Color::Green })` : Draws the non-recursive corridors.
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
  - `configure_sun(float sun_day_t_offs = 0.f, float minutes_per_day = 20.f, Season start_season = Season::Spring, float minutes_per_year = 120.f, Latitude latitude = Latitude::NorthernHemisphere, Longitude longitude = Longitude::F, bool use_per_room_lat_long_for_sun_dir = true)` : Configures the speed of the solar day, speed of the solar year, the starting direction of the sun and the starting season. Used for shadow movements for rooms over ground.
      When `use_per_room_lat_long_for_sun_dir` is `true` then use `latitude = Latitude::Equator` and `longitude = Longitude::F` to start with. Other values will shift the map over the globe so to speak, but with these starting settings the rooms at the top of the map will be the at the north pole and the rooms at the bottom of the map will be at the south pole. When `use_per_room_lat_long_for_sun_dir` is `false` then the specified latitude and longitude will be used globally across the whole map and the the function default args is a good starting point.
  - `configure_sun_rand(float minutes_per_day = 20.f, float minutes_per_year = 120.f, Latitude latitude = Latitude::NorthernHemisphere, Longitude longitude = Longitude::F, bool use_per_room_lat_long_for_sun_dir = true)` : Same as above but randomizes the initial direction of the sun.
  - `place_keys(bool only_place_on_dry_land)` : Places the keys in rooms, randomly all over the world.
  - `place_lamps(int num_torches, int num_lanterns, int num_magic_lamps, bool only_place_on_dry_land)` : Places `num_torches` torches, `num_lanterns` lanterns and `num_magic_lamps` magic lamps in rooms, randomly all over the world.
  - `place_weapons(int num_weapons, bool only_place_on_dry_land)` : Places `num_weapons` weapons in rooms, randomly all over the world.
  - `place_potions(int num_potions, bool only_place_on_dry_land)` : Places `num_potions` potions in rooms, randomly all over the world.
  - `place_armour(int num_armour, bool only_place_on_dry_land)` : Places `num_armour` armour parts in rooms, randomly all over the world.
  - `place_npcs(int num_npcs, bool only_place_on_dry_land)` : Places `num_npcs` NPCs in rooms, randomly all over the world.
  - `set_screen_scrolling_mode(ScreenScrollingMode mode, float t_page = 0.2f)` : Sets the screen scrolling mode to either `AlwaysInCentre`, `PageWise` or `WhenOutsideScreen`. `t_page` is used with `PageWise` mode.
  - `update(int frame_ctr, float fps, double real_time_s, float sim_time_s, float sim_dt_s, float fire_smoke_dt_factor, const keyboard::KeyPressDataPair& kpdp, bool* game_over)` : Updating the state of the dungeon engine. Manages things such as the change of direction of the sun for the shadows of rooms that are not under the ground and key-presses for control of the playable character.
  - `draw(ScreenHandler<NR, NC>& sh, double real_time_s, float sim_time_s, int anim_ctr_swim, int anim_ctr_fight, ui::VerticalAlignment mb_v_align = ui::VerticalAlignment::CENTER, ui::HorizontalAlignment mb_h_align = ui::HorizontalAlignment::CENTER, int mb_v_align_offs = 0, int mb_h_align_offs = 0, bool framed_mode = false, bool gore = false)` : Draws the whole dungeon world with NPCs and the PC along with items strewn all over the place. Use mb_v_align and mb_h_align to place the messagebox along with mb_v_align_offs, mb_h_align_offs and framed_mode. If `gore = true` then PC and NPCs will leave tracks of blood during fights.

## Texturing

Texturing done using the editor [`TextUR`](https://github.com/razterizer/TextUR).
Use the `TextUR` command line argument `-c` to convert a normal/fill texture to a shadow texture.
`DungGine` supports texture animations.

## Demo - Build and Run

When you clone this repo. The repo workspace/checkout dir should preferrably be located in a superfolder named `lib` in order for other libraries and programs to know where to look for it.

There are two options on dealing with repo dependencies:

### Repo Dependencies Option 1

This method will ensure that you are running the latest stable versions of the dependencies that work with `DungGine`.

The script `fetch-dependencies.py` used for this was created by [Thibaut Buchert](https://github.com/thibautbuchert).
`fetch-dependencies.py` is used in the following scripts below:

After a successful build, the scripts will then prompt you with the question if you want to run the demo.

When the script has been successfully run for the first time, you can then go to sub-folder `demo` and use the `build_demo.sh` / `build_demo.bat` script instead, and after you have built, just run the `run_demo.sh` or `run_demo.bat` script.

#### Windows

Run the following script:
```sh
setup_and_build_demo.bat
```

#### MacOS / Linux

Run the following script:
```sh
setup_and_build_demo.sh
```

### Repo Dependencies Option 2

In this method we basically outline the things done in the `setup_and_build_demo`-scripts in Option 1.

This method is more suitable for development as we're not necessarily working with "locked" dependencies.

You need the following header-only libraries:
* https://github.com/razterizer/Core
* https://github.com/razterizer/Termin8or

Make sure the folder structure looks like this:
```
<my_source_code_dir>/lib/Core/                   ; Core repo workspace/checkout goes here.
<my_source_code_dir>/lib/Termin8or/              ; Termin8or repo workspace/checkout goes here.
<my_source_code_dir>DungGine/                    ; DungGine repo workspace/checkout goes here.
```

These repos are not guaranteed to all the time work with the latest version of `DungGine`. If you want the more stable aproach then look at Option 1 instead.

### Windows

Then just open `<my_source_code_dir>/DungGine/demo/demo.vs.sln` and build and run. That's it!

You can also build it by going to `<my_source_code_dir>/DungGine/demo/` and build with `build_demo.bat`.
Then you run by typing `run_demo.bat`.

### MacOS / Linux

Goto `<my_source_code_dir>/DungGine/demo/` and build with `./build_demo.sh`.

Then run by typing `run_demo.sh` or simply `./bin/demo`.

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

ScreenHandler<NR, NC> sh;
Color bg_color = Color::Default;

dung::DungGine dungeon_engine;
dungeon_engine.load_dungeon(&bsp_tree);
dungeon_engine.style_dungeon();
dungeon_engine.draw(sh, get_real_time_s(), get_sim_time_s(), 0, 0);
sh.print_screen_buffer(bg_color);
```

<img width="560" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/7417d29d-1d2a-47b6-b926-31a9ae0177b2">

***

```cpp
// Declarations
dung::BSPTree bsp_tree { 4 }; // argument: `min_room_length = 4`.
dung::DungGineTextureParams texture_params;
std::unique_ptr<dung::DungGine> dungeon_engine;
ScreenHandler<NR, NC> sh;
Color bg_color = Color::Black;
int anim_ctr = 0;
const float fire_smoke_dt_factor = 0.5f;

// Initializations
bsp_tree.generate(200, 400, dung::Orientation::Vertical); // arguments: world_size_rows, world_size_cols,
                  first_split_orientation.
bsp_tree.pad_rooms(4); // arguments: min_rnd_wall_padding = 4, [max_rnd_wall_padding = 4].
bsp_tree.create_corridors(1); // argument: min_corridor_half_width = 1, (1 means it will be three chars wide).
bsp_tree.create_doors(100, true); // We also create doors here. Arguments: max_num_locked_doors, allow_passageways.

dungeon_engine = std::make_unique<dung::DungGine>(get_exe_folder(), true); // arguments: exe_folder, use_fow, texture_params.
dungeon_engine.load_dungeon(&bsp_tree);
dungeon_engine.configure_sun_rand(20.f, 120.f, dung::Latitude::NorthernHemisphere, dung::Longitude::FW, false); // 20 minutes per day and 120 minutes per year. Global shadow.
dungeon_engine.style_dungeon();
if (!dungeon_engine.place_player(sh.size()))
  std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
dungeon_engine->place_keys(true);
dungeon_engine->place_lamps(20, 5, 3, true);
dungeon_engine->place_weapons(100, true);
dungeon_engine->place_potions(100, true);
dungeon_engine.set_screen_scrolling_mode(ScreenScrollingMode::WhenOutsideScreen);

// In game loop:
sh.clear();
bool game_over = false;
dungeon_engine->update(get_frame_count(), get_real_fps(), 
  get_real_time_s(), get_sim_time_s(), get_sim_dt_s(),
  fire_smoke_dt_factor,
  kpdp, &game_over); // arg0 : time from game start, arg3 : keyboard::KeyPressData object, arg4 : retrieves game over state.
if (game_over)
  set_state_game_over();
dungeon_engine->draw(sh, get_real_time_s(), get_sim_time_s(),
  get_anim_count(0), get_anim_count(1));
sh.print_screen_buffer(bg_color);
anim_ctr++;
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
ScreenHandler<NR, NC> sh;
Color bg_color = Color::Black;
int anim_ctr = 0;
const float fire_smoke_dt_factor = 0.5f;

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
dungeon_engine.configure_sun_rand(20.f, 120.f, dung::Latitude::Equator, dung::Longitude::F, true); // 20 minutes per day and 120 minutes per year. Localized shadows across the map starting at Equator & Front.
dungeon_engine.style_dungeon();
if (!dungeon_engine.place_player(sh.size()))
  std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
dungeon_engine->place_keys(true);
dungeon_engine->place_lamps(20, 15, 5, true);
dungeon_engine->place_weapons(150, true);
dungeon_engine->place_potions(100, true);
dungeon_engine->place_armour(150, true);
dungeon_engine->place_npcs(100, false);
dungeon_engine.set_screen_scrolling_mode(ScreenScrollingMode::WhenOutsideScreen);

// In game loop:
sh.clear();
bool game_over = false;
dungeon_engine->update(get_frame_count(), get_real_fps(),
  get_real_time_s(), get_sim_time_s(), get_sim_dt_s(),
  fire_smoke_dt_factor,
  kpdp, &game_over); // arg0 : time from game start, arg3 : keyboard::KeyPressData object, arg4 : retrieves game over state.
if (game_over)
  set_state_game_over();
dungeon_engine->draw(sh, get_real_time_s(), get_sim_time_s(),
  get_anim_count(0), get_anim_count(1),
  ui::VerticalAlignment::BOTTOM, ui::HorizontalAlignment::CENTER,
  -5, 0, false, true);
sh.print_screen_buffer(bg_color);
anim_ctr++;
```
