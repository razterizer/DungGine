# DungGine

`DungGine` is a terminal based dungeon engine for RPGs and Rogue-likes and that uses [`Termin8or`](https://github.com/razterizer/Termin8or) for rendering.
This is a header only library.
The engine works very well together with the `GameEngine` class of lib [`Termin8or`](https://github.com/razterizer/Termin8or).

This library is very new and currently only provides a class `BSPTree` that is responsible for creating the rooms in the "dungeon".

## Headers

* `BSPTree.h`
  - `generate()` : Generates the BSP regions recursively.
  - `pad_rooms()` : Pads the regions into rooms.
  - `create_corridors_recursive()` : Creates bottom-up recursive corridors. **Warning: Not working properly yet**.
  - `ceate_corridors_flat()` : Non-recursive method of creating corridors on leaf-level. Much more robust than `ceate_corridors()` but is very greedy at the moment.
  - `draw_regions()` : Draws the regions.
  - `draw_rooms()` : Draws the rooms.
  - `draw_corridors_recursive()` : Draws the recursive corridors.
  - `draw_corridors_flat()` : Draws the non-recursive corridors.
  - `print_tree()` : Debug printing of the tree.
* `DungGine.h`
  - `load_dungeon(BSPTree*)` : Loads a generated BSP tree.
  - `style_dungeon()` : Performs automated styling of rooms in the dungeon / realm.
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

***

```cpp
dung::BSPTree::draw_rooms()
dung::BSPTree::draw_corridors_recursive()
```

**Warning: WIP. Not working properly yet.**

<img width="570" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/1f9223ce-94cb-46dc-88e0-c3c3446cdebf">

***

```cpp
dung::BSPTree::draw_rooms()
dung::BSPTree::draw_corridors_flat()
```

<img width="573" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/fd23bf8e-17b2-4055-84d6-d6bdd87538ef">

***

```cpp
dung::BSPTree bsp_tree { 4 }; // argument: `min_room_length = 4`.
bsp_tree.generate(29, 79, dung::Orientation::Vertical); // arguments: world_size_rows, world_size_cols,
                  first_split_orientation.
bsp_tree.pad_rooms(4); // arguments: min_rnd_wall_padding = 4, [max_rnd_wall_padding = 4].
bsp_tree.create_corridors_flat(1); // argument: min_corridor_half_width = 1, (1 means it will be three chars wide).

Text t;
SpriteHandler<NR, NC> sh;
Text::Color bg_color = Text::Color::Default;

dung::DungGine dungeon_engine;
dungeon_engine.load_dungeon(&bsp_tree);
dungeon_engine.style_dungeon();
dungeon_engine.draw(sh);
sh.print_screen_buffer(t, bg_color);
```

<img width="570" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/ea038a05-6c62-4c9c-991c-a0d4068f22f8">

