# DungGine

`DungGine` is a terminal based dungeon engine for RPGs and Rogue-likes and that uses [`Termin8or`](https://github.com/razterizer/Termin8or) for rendering.
This is a header only library.
The engine works very well together with the `GameEngine` class of lib [`Termin8or`](https://github.com/razterizer/Termin8or).

This library is very new and currently only provides a class `BSPTree` that is responsible for creating the rooms in the "dungeon".

## Headers

* `BSPTree.h`
  - `generate()` : Generates the BSP regions recursively.
  - `pad_rooms()` : Pads the regions into rooms.
  - `create_corridors()` : Creates corridors. **Warning: Not working properly yet**.
  - `ceate_corridors2()` : Non-recursive method of creating corridors. Much more robust than `ceate_corridors()` but is very greedy.
  - `draw_regions()` : Draws the regions.
  - `draw_rooms()` : Draws the rooms.
  - `draw_corridors()` : Draws the corridors.
  - `draw_corridors2()` : Draws the non-recursive corridors.
  - `print_tree()` : Debug printing of the tree.

## Examples

`BSPTree::draw_regions()`:

<img width="574" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/d3dba22a-f41d-482b-9a8f-0490e6f24835">

`BSPTree::draw_rooms()`:

<img width="560" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/b4b672a8-9ce5-4d74-8d71-c23b37ac1042">

`BSPTree::draw_rooms()` followed by `BSPTree::draw_corridors()` **Warning: WIP. Not working properly yet.**

<img width="570" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/1f9223ce-94cb-46dc-88e0-c3c3446cdebf">

`BSPTree::draw_rooms()` followed by `BSPTree::draw_corridors2()`

<img width="573" alt="image" src="https://github.com/razterizer/DungGine/assets/32767250/fd23bf8e-17b2-4055-84d6-d6bdd87538ef">
