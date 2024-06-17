# DungGine

`DungGine` is a terminal based dungeon engine for RPGs and Rogue-likes and that uses [`Termin8or`](https://github.com/razterizer/Termin8or) for rendering.
This is a header only library.
The engine works very well together with the `GameEngine` class of lib [`Termin8or`](https://github.com/razterizer/Termin8or).

This library is very new and currently only provides a class `BSPTree` that is responsible for creating the rooms in the "dungeon".

## Headers

* `BSPTree.h`
  - `generate()` : Generates the BSP regions recursively.
  - `pad_rooms()` : Pads the regions into rooms.
  - `ceate_corridors()` : Non-recursive method of creating corridors on leaf-level.
  - `draw_regions()` : Draws the regions.
  - `draw_rooms()` : Draws the rooms.
  - `draw_corridors()` : Draws the non-recursive corridors.
  - `print_tree()` : Debug printing of the tree.
  - `fetch_leaves()` : Fetches the leaves of the BSP tree where the rooms are stored.
  - `get_room_corridor_map()` : Function that retrieves the room and corridor relationship data structure.
* `DungGine.h`
  - `load_dungeon(BSPTree*)` : Loads a generated BSP tree.
  - `style_dungeon()` : Performs automated styling of rooms in the dungeon / realm.
  - `set_player_character()` : Sets the character of the playable character (pun intended).
  - `place_player()` : Places the player near the middle of the realm in one of the corridors and centers the screen around the player.
  - `configure_sun(Direction sun_dir, float minutes_per_day)` : Configures the speed of the solar day and the starting direction of the sun. Used for shadow movements for rooms over ground.
  - `configure_sun(float minutes_per_day)` : Same as above but randomizes the initial direction of the sun.
  - `set_screen_scrolling_mode()` : Sets the screen scrolling mode to either `AlwaysInCentre`, `PageWise` or `WhenOutsideScreen`.
  - `update()` : Updating the state of the dungeon engine. Manages things such as the change of direction of the sun for the shadows of rooms that are not under the ground and key-presses for control of the playable character.
  - `draw()` : Draws the rooms of the dungeon / realm (will include drawing of corridors in the near(?) future).

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
Text::Color bg_color = Text::Color::Default;

dung::DungGine dungeon_engine;
dungeon_engine.load_dungeon(&bsp_tree);
dungeon_engine.style_dungeon();
dungeon_engine.draw(sh);
sh.print_screen_buffer(t, bg_color);
```

<img width="560" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/7417d29d-1d2a-47b6-b926-31a9ae0177b2">

***

```cpp
dung::BSPTree bsp_tree { 4 }; // argument: `min_room_length = 4`.
bsp_tree.generate(200, 400, dung::Orientation::Vertical); // arguments: world_size_rows, world_size_cols,
                  first_split_orientation.
bsp_tree.pad_rooms(4); // arguments: min_rnd_wall_padding = 4, [max_rnd_wall_padding = 4].
bsp_tree.create_corridors(1); // argument: min_corridor_half_width = 1, (1 means it will be three chars wide).
bsp_tree.create_doors(); // We also create doors here.

Text t;
SpriteHandler<NR, NC> sh;
Text::Color bg_color = Text::Color::Black;

dung::DungGine dungeon_engine;
dungeon_engine.load_dungeon(&bsp_tree);
dungeon_engine.style_dungeon();
if (!dungeon_engine.place_player(sh.size()))
  std::cerr << "ERROR : Unable to place the playable character!" << std::endl;
dungeon_engine.configure_sun(20.f);
dungeon_engine.set_screen_scrolling_mode(ScreenScrollingMode::WhenOutsideScreen);

// In game loop:
sh.clear();
dungeon_engine.update(get_sim_time_s(), kpd); // arg0 : time from game start, arg1 : keyboard::KeyPressData object.
dungeon_engine.draw(sh);
sh.print_screen_buffer(t, bg_color);
```

<img width="566" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/b24d58d7-6ad5-4063-881d-650dd7bc905a">

Note the playable character marked as a "@" in the centre of the screen.
To move the character in a game loop, use function `update()` to allow keystrokes to be registered. Then control the character by pressing the ASWD keys. Press space-bar to open and close doors that you are located next to.
