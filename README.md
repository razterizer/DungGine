# DungGine

![GitHub License](https://img.shields.io/github/license/razterizer/DungGine?color=blue)
![Static Badge](https://img.shields.io/badge/linkage-header_only-yellow)
![Static Badge](https://img.shields.io/badge/C%2B%2B-20-yellow)

[![build ubuntu](https://github.com/razterizer/DungGine/actions/workflows/build-ubuntu.yml/badge.svg)](https://github.com/razterizer/DungGine/actions/workflows/build-ubuntu.yml)
[![build macos](https://github.com/razterizer/DungGine/actions/workflows/build-macos.yml/badge.svg)](https://github.com/razterizer/DungGine/actions/workflows/build-macos.yml)
[![build ubuntu](https://github.com/razterizer/DungGine/actions/workflows/build-windows.yml/badge.svg)](https://github.com/razterizer/DungGine/actions/workflows/build-windows.yml)
![Valgrind Status](https://raw.githubusercontent.com/razterizer/DungGine/badges/valgrind-badge.svg)

![Top Languages](https://img.shields.io/github/languages/top/razterizer/DungGine)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/DungGine)
![C++ LOC](https://raw.githubusercontent.com/razterizer/DungGine/badges/loc-badge.svg)
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
![378201385-434aa258-ea76-4143-9ddc-44f63ed0f294-trimmed](https://github.com/user-attachments/assets/ef7b2efb-ee0a-4530-a278-a71f4008e991)

`DungGine` is a terminal based dungeon engine for RPGs and dungeon-crawlers (e.g. Rogue-likes) and that uses [`Termin8or`](https://github.com/razterizer/Termin8or) for rendering.
This is a header only library.
The engine works very well together with the `GameEngine` class of lib [`Termin8or`](https://github.com/razterizer/Termin8or).

There are three main classes for this dungeon generator: `BSPTree` that is responsible for creating the rooms, corridors and doors for each floor in the "dungeon", `Dungeon` which holds the BSP trees (one for each floor) and thus the whole map, and then there is the class `DungGine` that is the dungeon game engine itself.
See the next section for a summary over these three classes.

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
* save game feature
* logging recording and replay (logging is a feature of [`Termin8or`](https://github.com/razterizer/Termin8or) and works well together with the save game feature)

and stuff like that.
The corridors are all straight (for now). Other than that, I think the BSP generation is pretty standard.
Items you can pick up are keys, torches, lanterns, magic lamps (isotropic light), potions (some contain poison, so gotta watch out), weapons and armour.

## Keys

* `WASD keys` or `arrow keys` to control the direction of the PC or browse inventory objects.
* `Space` to pick up items, open doors or equip items in the inventory UI.
* `-` to open and close the inventory.
* `f` or `F` to fight friendlies (hostile NPCs will always start the fight). When running away from them, they will quickly forget and forgive.
* `d` or `D` to drop a hilited item when in the inventory.
* `c` or `C` to consume a potion.
* `i` or `I` to identify things the PC can see.
* `g` to save game (need to setup the save game feature properly. See below). File extension: `*.dsg`.
* `G` to load saved game (need to setup the save game feature properly. See below). File extension: `*.dsg`.
* `z` to save a screenshot. File extension: `*.txt`.
* `+` NPC debug view. It shows NPC states and displays arrows to them to make them easier to find.
* `?` Misc debug view. It shows what terrain you're standing on, what floor you're on, and in the corner of every room you'll see a `0` (surface-level) or `1` (underground). Also when in debug view, you will be able to see the `TextBoxDebug` debug variable window if there are any variables registered to it.

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

## Gateways

There are two kinds of gateways if you will: Doors and Staircases.

### Doors

Doors can be either a passageway or a door that you can open and close. A door door can be locked.
The following characters are used for different door states:

* `^` : Passageway. You can always pass through these from a corridor to a room or vice versa.
* `L` : An open door. Just pass through.
* `D` : A closed door. You need to open with the space key when next to the door while the PC is directed at it.
* `G` : A locked door. You need to find the key first, then select the key and then press space when next to the door while the PC is directed at it. A used key will automatically vanish by the PC performing a vanishing spell for you.

### Staircases

Staircases go either one level up or down. They look like a gray `B` on a black background. To walk up/down the stairs, you first need to stand on the staircase symbol and then press space. The messaging system will inform you where you walked and if you are on the top floor.

There is currently no visual distiction between upwards or downwards going staircases. Perhaps a future feature.

## Headers

* `BSPTree.h`
  - `BSPTree(int min_room_length)` : The constructor.
  - `reset()` : Resets the whole tree with all of its nodes and auxiliary data structures. Need to call this before regenerating with `generate()`.
  - `generate(int world_size_rows, int world_size_cols,
                  Orientation first_split_orientation)` : Generates the BSP regions recursively.
  - `get_bounding_box()` gets the bounding box of the whole tree that was set by `generate()`.
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
  - `serialize(std::vector<std::string>& lines)` : Used by the save-game feature to store info about the current state.
  - `deserialize(std::vector<std::string>::iterator it_line_begin, std::vector<std::string>::iterator it_line_end)` : Used by the save-game feature to restore info about the current state.
* `Dungeon.h`
  - `DungeonFloorParams` : A param struct that contain parameters describing how a certain floor (`BSPTree`) should be generated.
  - `Dungeon(bool first_level_is_over_ground, int starting_floor = -1)` : If argument `starting_floor = -1` means that the PC starts from the bottom-most floor. If argument `first_level_is_over_ground = true` then the first floor/bps-tree will be styled such that all of its rooms are marked as surface-level and all subsequent floors will be styled such that all of their rooms are marked as underground, if `false`, then all rooms of all levels will be randomly marked as underground / surface-level (old/legacy behaviour).
  - `generate(const std::vector<DungeonFloorParams>& floor_params)` : Generates the BSP trees (floors) for the whole dungeon map given the `floor_params` where each element in the vector corresponds to one floor.
  - `reset()` : Resets the datastructures. Call this prior to `generate()` in order to regenerate all the data.
  - `get_world_size()` : Returns the total world size of all of the floors stored in this object.
  - `get_trees()` : Retrieves the BSP trees AKA floors in this `Dungeon` object.
  - `get_init_floor()` : Gets the starting floor index.
  - `get_tree(int floor)` : Gets the BSP tree corresponding to the supplied floor index.
  - `get_rooms(BSPTree* bsp_tree)` : Retrieves the cached rooms of the supplied BSP tree.
  - `create_staircases(int prob_in_room = 10)` : Creates staircases between adjacent pairs of floors. `prob_in_room` means the inverse probability of creating a staircase in a pair of rooms that overlap. A value of 10 means probability 1/10 or about once every ten times.
  - `num_floors()` : Returns the number of floors in this object.
  - `is_first_floor_is_surface_level()` : Retrieves the first argument of the constructor. Just worded a bit differently.
  - `fetch_staircases(int floor)` : Fetches the staircases of a certain floor as a vector of raw-ptrs instead of a vector of unique-ptrs.
  - `serialize(std::vector<std::string>& lines)` : Used by the save-game feature to store info about the current state.
  - `deserialize(std::vector<std::string>::iterator it_line_begin, std::vector<std::string>::iterator it_line_end)` : Used by the save-game feature to restore info about the current state.
* `DungGine.h`
  - `DungGine(bool use_fow, bool sorted_inventory_items, DungGineTextureParams texture_params = {})` : The constructor. If `use_fow = true` then the whole dungeon map will be covered in black until you gradually uncover area by area. If `sorted_inventory_items = true` then an inventory subgroup will be automatically sorted every time an item is added to it. It adjusts any states and indices related to this subgroup in order to retain the correct hilite and selection status after sorting.
  - `load_dungeon(Dungeon& dungeon)` : Loads a dungeon consisting of generated BSP trees, one BSP tree corresponds to a floor.
  - `style_dungeon(WallShadingType wall_shading_surface_level, WallShadingType wall_shading_underground)` : Performs automated styling of rooms in the dungeon / realm.
  - `set_player_character(char ch)` : Sets the character of the playable character (pun intended).
  - `set_player_style(const Style& style)` : Sets the style (fg/bg color) of the playable character.
  - `place_player(const RC& screen_size, std::optional<RC> world_pos = std::nullopt)` : Places the player near the middle of the realm in one of the corridors and centers the screen around the player.
  -  `configure_save_game(std::optional<std::string> dunggine_lib_repo_path)` : Allows you to choose between version checking (using git commit hash on last commit of DungGine.git) and no version checking. If path is `nullopt` then version checking is disabled.
  - `configure_sun(float sun_day_t_offs = 0.f, float minutes_per_day = 20.f, Season start_season = Season::Spring, float minutes_per_year = 120.f, Latitude latitude = Latitude::NorthernHemisphere, Longitude longitude = Longitude::F, bool use_per_room_lat_long_for_sun_dir = true)` : Configures the speed of the solar day, speed of the solar year, the starting direction of the sun and the starting season. Used for shadow movements for rooms over ground.
      When `use_per_room_lat_long_for_sun_dir` is `true` then use `latitude = Latitude::Equator` and `longitude = Longitude::F` to start with. Other values will shift the map over the globe so to speak, but with these starting settings the rooms at the top of the map will be the at the north pole and the rooms at the bottom of the map will be at the south pole. When `use_per_room_lat_long_for_sun_dir` is `false` then the specified latitude and longitude will be used globally across the whole map and the the function default args is a good starting point.
  - `configure_sun_rand(float minutes_per_day = 20.f, float minutes_per_year = 120.f, Latitude latitude = Latitude::NorthernHemisphere, Longitude longitude = Longitude::F, bool use_per_room_lat_long_for_sun_dir = true)` : Same as above but randomizes the initial direction of the sun.
  - `place_keys(bool only_place_on_dry_land, bool assure_contrasting_fg_colors, bool only_place_on_same_floor)` : Places the keys in rooms, randomly all over the world.
  - `place_lamps(int num_torches_per_floor, int num_lanterns_per_floor, int num_magic_lamps_per_floor, bool only_place_on_dry_land, bool assure_contrasting_fg_colors)` : Places lamps in rooms, randomly all over the world.
  - `place_weapons(int num_daggers_per_floor, int num_swords_per_floor, int num_flails_per_floor, int num_morningstars_per_floor, int num_slings_per_floor, int num_bows_per_floor, int num_crossbows_per_floor, bool only_place_on_dry_land, bool assure_contrasting_fg_colors)` : Places weapons in rooms, randomly all over the world.
  - `place_potions(int num_health_potions_per_floor, int num_poison_potions_per_floor, bool only_place_on_dry_land, bool assure_contrasting_fg_colors)` : Places potions in rooms, randomly all over the world.
  - `place_armour(int num_shields_per_floor, int num_gambesons_per_floor, int num_cmhs_per_floor, int num_pbas_per_floor, int num_padded_coifs_per_floor, int num_cmcs_per_floor, int num_helmets_per_floor, bool only_place_on_dry_land, bool assure_contrasting_fg_colors)` : Places armour parts in rooms, randomly all over the world. Explanation: `cmh` = chain-maille hauberk, `pba` = plated body armour, `cmc` = chain-maille coif.
  - `place_npcs(int num_npcs_per_floor, bool only_place_on_dry_land)` : Places `num_npcs` NPCs in rooms, randomly all over the world.
  - `set_screen_scrolling_mode(ScreenScrollingMode mode, float t_page = 0.2f)` : Sets the screen scrolling mode to either `AlwaysInCentre`, `PageWise` or `WhenOutsideScreen`. `t_page` is used with `PageWise` mode.
  - `update(int frame_ctr, float fps, double real_time_s, float sim_time_s, float sim_dt_s, float fire_smoke_dt_factor, float projectile_speed_factor, int melee_attack_dice, int ranged_attack_dice, const keyboard::KeyPressDataPair& kpdp, bool* game_over)` : Updating the state of the dungeon engine. Manages things such as the change of direction of the sun for the shadows of rooms that are not under the ground and key-presses for control of the playable character.
  - `draw(ScreenHandler<NR, NC>& sh, double real_time_s, float sim_time_s, int anim_ctr_swim, int anim_ctr_fight, int melee_blood_prob_visible, int melee_blood_prob_invisible, ui::VerticalAlignment mb_v_align = ui::VerticalAlignment::CENTER, ui::HorizontalAlignment mb_h_align = ui::HorizontalAlignment::CENTER, int mb_v_align_offs = 0, int mb_h_align_offs = 0, bool framed_mode = false, bool gore = false)` : Draws the whole dungeon world with NPCs and the PC along with items strewn all over the place. melee_blood_prob_visible and melee_blood_prob_invisible are the 1 in prob probabilities for generating a blood splat during melee fight depending on whether the NPC is visible or not. melee_blood_prob_invisible should therefore be higher than melee_blood_prob_visible, although doesn't have to be. Use mb_v_align and mb_h_align to place the messagebox along with mb_v_align_offs, mb_h_align_offs and framed_mode. If `gore = true` then PC and NPCs will leave tracks of blood during fights. 
  - `save_game_post_build(const std::string& savegame_filename, unsigned int curr_rnd_seed, double real_time_s)` : Called when pressing the `g` key.
  - `load_game_pre_build(const std::string& savegame_filename, unsigned int* curr_rnd_seed, double real_time_s)` : Called when pressing the `G` key. Called internally before rebuilding the scene via the `on_scene_rebuild_request()` event to funnel the random seed from the save-game file to `GameEngine` or whatever system you are using to run the `DungGine` in. The random seed from the save-file needs to be set before regenerating the scene.
  - `load_game_post_build(const std::string& savegame_filename, double real_time_s)` : Called when pressing the `G` key. Called internally after the scene has been rebuilt via the `on_scene_rebuild_request()` event. This function deserializes all the states from the save-file on top of the rebuilt scene and its data structures.

## Texturing

Texturing done using the editor [`TextUR`](https://github.com/razterizer/TextUR).
Use the `TextUR` command line argument `-c` to convert a normal/fill texture to a shadow texture.
`DungGine` supports texture animations.

## Save Game

To be able to use the save game feature, you need to implement the following DungGineListener events:

* `virtual void on_scene_rebuild_request()`
* `virtual void on_save_game_request(std::string& filepath, unsigned int& curr_rnd_seed)`
* `virtual void on_load_game_request_pre(std::string& filepath)`
* `virtual void on_load_game_request_post(unsigned int rnd_seed)`

Refer to the demo for an example on how to use these in a `GameEngine` application.

The save game feature works very well together with the logging record/playback feature of [`Termin8or`](https://github.com/razterizer/Termin8or) and you can even make a logging recording of you loading a saved game and then replay when you loaded that save game.

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
std::vector<dung::DungeonFloorParams> dungeon_floor_params
auto& floor = dungeon_floor_params.emplace_back();
floor.min_room_length = 4;
floor.world_size = { 29, 79 };
floor.first_split_orientation = dung::Orientation::Vertical;
floor.room_padding_min = 4;
floor.room_padding_max = 4;
floor.min_corridor_half_width = 1; // min_corridor_half_width = 1, (1 means it will be three chars wide).
floor.max_num_locked_doors = 0;
floor.allow_passageways = true;
dung::Dungeon dungeon { false, -1 };
dungeon.generate(dungeon_floor_params);

ScreenHandler<NR, NC> sh;
Color bg_color = Color::Default;

dung::DungGine dungeon_engine { "bin/", false, false };
dungeon_engine.load_dungeon(dungeon);
dungeon_engine.style_dungeon(dung::WallShadingType::BG_Rand, dung::WallShadingType::BG_Rand);
dungeon_engine.draw(sh, get_real_time_s(), get_sim_time_s(), 0, 0, 30, 40);
sh.print_screen_buffer(bg_color);
```

<img width="560" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/7417d29d-1d2a-47b6-b926-31a9ae0177b2">

***

```cpp
// Declarations
std::vector<dung::DungeonFloorParams> dungeon_floor_params;
dung::Dungeon dungeon { false, -1 };
dung::DungGineTextureParams texture_params;
std::unique_ptr<dung::DungGine> dungeon_engine;
ScreenHandler<NR, NC> sh;
Color bg_color = Color::Black;
int anim_ctr = 0;
const float fire_smoke_dt_factor = 0.5f;

// Initializations
auto& floor = dungeon_floor_params.emplace_back();
floor.min_room_length = 4;
floor.world_size = { 200, 400 };
floor.first_split_orientation = dung::Orientation::Vertical;
floor.room_padding_min = 4;
floor.room_padding_max = 4;
floor.min_corridor_half_width = 1; // min_corridor_half_width = 1, (1 means it will be three chars wide).
floor.max_num_locked_doors = 100;
floor.allow_passageways = true;
dungeon.generate(dungeon_floor_params);

dungeon_engine = std::make_unique<dung::DungGine>(true, false); // arguments: exe_folder, use_fow, sorted_inventory_items, texture_params.
dungeon_engine.load_dungeon(dungeon);
dungeon_engine.configure_sun_rand(20.f, 120.f, dung::Latitude::NorthernHemisphere, dung::Longitude::FW, false); // 20 minutes per day and 120 minutes per year. Global shadow.
dungeon_engine.style_dungeon(dung::WallShadingType::BG_Rand, dung::WallShadingType::BG_Rand);
if (!dungeon_engine.place_player(sh.size()))
  std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
dungeon_engine->place_keys(true, true, true);
dungeon_engine->place_lamps(20, 5, 3, true, true);
dungeon_engine->place_weapons(16, 13, 15, 13, 16, 14, 13, true, true);
dungeon_engine->place_potions(10, 90, true, true);
dungeon_engine.set_screen_scrolling_mode(ScreenScrollingMode::WhenOutsideScreen);

// In game loop:
sh.clear();
bool game_over = false;
dungeon_engine->update(get_frame_count(), get_real_fps(), 
  get_real_time_s(), get_sim_time_s(), get_sim_dt_s(),
  fire_smoke_dt_factor, projectile_speed_factor,
  20, 40,
  kpdp, &game_over); // arg0 : time from game start, arg3 : keyboard::KeyPressData object, arg4 : retrieves game over state.
if (game_over)
  set_state_game_over();
dungeon_engine->draw(sh, get_real_time_s(), get_sim_time_s(),
  get_anim_count(0), get_anim_count(1),
  30, 40);
sh.print_screen_buffer(bg_color);
anim_ctr++;
```

<img width="566" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/b24d58d7-6ad5-4063-881d-650dd7bc905a">

Note the playable character marked as a "@" in the centre of the screen.
To move the character in a game loop, use function `update()` to allow keystrokes to be registered. Then control the character by pressing the ASWD keys. Press space-bar to open and close doors that you are located next to.

***

```cpp
// Declarations
std::vector<dung::DungeonFloorParams> dungeon_floor_params;
dung::Dungeon dungeon { true, -1 }; // First floor is surface level, PC placed at the lowest underground floor.
dung::DungGineTextureParams texture_params;
std::unique_ptr<dung::DungGine> dungeon_engine;
ScreenHandler<NR, NC> sh;
Color bg_color = Color::Black;
int anim_ctr = 0;
const float fire_smoke_dt_factor = 0.5f;

// Initializations
auto& floor_surface_level = dungeon_floor_params.emplace_back();
floor_surface_level.min_room_length = 4;
floor_surface_level.world_size = { 200, 400 };
floor_surface_level.first_split_orientation = dung::Orientation::Vertical;
floor_surface_level.room_padding_min = 4;
floor_surface_level.room_padding_max = 4;
floor_surface_level.min_corridor_half_width = 1; // min_corridor_half_width = 1, (1 means it will be three chars wide).
floor_surface_level.max_num_locked_doors = 100;
floor_surface_level.allow_passageways = true;
auto& floor_underground = dungeon_floor_params.emplace_back();
floor_underground.min_room_length = 4;
floor_underground.world_size = { 200, 400 };
floor_underground.first_split_orientation = dung::Orientation::Vertical;
floor_underground.room_padding_min = 4;
floor_underground.room_padding_max = 4;
floor_underground.min_corridor_half_width = 1; // min_corridor_half_width = 1, (1 means it will be three chars wide).
floor_underground.max_num_locked_doors = 100;
floor_underground.allow_passageways = true;
dungeon.generate(dungeon_floor_params);

texture_params.dt_anim_s = 0.5;
auto f_tex_path = [](const auto& filename)
{
  return folder::join_path({ "textures", filename });
};
texture_params.texture_file_names_surface_level_fill.emplace_back(f_tex_path("texture_sl_fill_0.tex"));
texture_params.texture_file_names_surface_level_fill.emplace_back(f_tex_path("texture_sl_fill_1.tex"));
texture_params.texture_file_names_surface_level_shadow.emplace_back(f_tex_path("texture_sl_shadow_0.tex"));
texture_params.texture_file_names_surface_level_shadow.emplace_back(f_tex_path("texture_sl_shadow_1.tex"));

dungeon_engine = std::make_unique<dung::DungGine>(true, true, texture_params); // arguments: exe_folder, use_fow, sorted_inventory_items, texture_params.
dungeon_engine.load_dungeon(dungeon);
dungeon_engine.configure_sun_rand(20.f, 120.f, dung::Latitude::Equator, dung::Longitude::F, true); // 20 minutes per day and 120 minutes per year. Localized shadows across the map starting at Equator & Front.
dungeon_engine.style_dungeon(dung::WallShadingType::BG_Light, dung::WallShadingType::BG_Dark);
if (!dungeon_engine.place_player(sh.size()))
  std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
dungeon_engine->place_keys(true, true, false);
dungeon_engine->place_lamps(20, 15, 5, true, true);
dungeon_engine->place_weapons(30, 25, 17, 13, 30, 20, 15, true, true);
dungeon_engine->place_potions(20 80, true, true);
dungeon_engine->place_armour(25, 30, 20, 10, 40, 15, 10, true, true);
dungeon_engine->place_npcs(100, false);
dungeon_engine.set_screen_scrolling_mode(ScreenScrollingMode::WhenOutsideScreen);

// In game loop:
sh.clear();
bool game_over = false;
dungeon_engine->update(get_frame_count(), get_real_fps(),
  get_real_time_s(), get_sim_time_s(), get_sim_dt_s(),
  fire_smoke_dt_factor, projectile_speed_factor,
  20, 40,
  kpdp, &game_over); // arg0 : time from game start, arg3 : keyboard::KeyPressData object, arg4 : retrieves game over state.
if (game_over)
  set_state_game_over();
dungeon_engine->draw(sh, get_real_time_s(), get_sim_time_s(),
  get_anim_count(0), get_anim_count(1),
  30, 40,
  ui::VerticalAlignment::BOTTOM, ui::HorizontalAlignment::CENTER,
  -5, 0, false, true);
sh.print_screen_buffer(bg_color);
anim_ctr++;
```
