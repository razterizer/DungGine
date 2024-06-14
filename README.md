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

`BSPTree::draw_regions()`:

<img width="574" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/d3dba22a-f41d-482b-9a8f-0490e6f24835">

`BSPTree::draw_rooms()`:

<img width="560" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/b4b672a8-9ce5-4d74-8d71-c23b37ac1042">

`BSPTree::draw_rooms()` followed by `BSPTree::draw_corridors_recursive()` **Warning: WIP. Not working properly yet.**

<img width="570" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/1f9223ce-94cb-46dc-88e0-c3c3446cdebf">

`BSPTree::draw_rooms()` followed by `BSPTree::draw_corridors_flat()`

<img width="573" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/fd23bf8e-17b2-4055-84d6-d6bdd87538ef">

Here's `DungGine::draw()` after call to `load_dungeon()` followed by `style_dungeon()` (which styles it randomly).

<img width="570" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/ea038a05-6c62-4c9c-991c-a0d4068f22f8">

