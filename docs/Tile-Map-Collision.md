# A Simple Method for Tile-Based Collision and Resolution

This method is designed for a tile-based level where the player (or any other object) can collide with specific tiles, and allows for unrestricted player movement and size. It assumes you are using a two-dimensional array of values to track each tile.

Whatever method of movement is used, it should be calculated ***but not applied*** before checking for collisions. As part of this method, the player will be moved outside of any tiles by setting its position directly. Depending on your movement system you may also need to change other things such as velocity or jump tracking.

The summary of the method is:

- [Calculate and save the map offset](#calculate-the-map-offset)
- [Move along one axis only](#move-along-one-axis-only)
- [Get the bounding tiles](#1-get-the-bounding-tiles)
- [Step through each tile around the player](#2-step-through-each-tile-around-the-player)
- [Calculate the tile position and check collision](#3-calculate-the-tile-position-and-check-collision)
- [Reset position depending on axis of movement](#4-reset-position-depending-on-axis-of-movement)
- [Move along other axis, repeat collision steps](#move-along-the-other-axis-repeat-collision-steps)

To start, we need to know:

1. the player position,
1. the player size,
1. the tile size,
1. the map height and width (in tiles),
1. the player movement vector.

> I'm using "player" here because it's easy to talk about, but of course this could be any object.

If the player can reach the edge of the map, you will also need to check that your indexes don't go below zero or above the map width and height.

---

## Calculate and save the map offset

Assuming our map has (0, 0) at the center, we need to calculate what it would take to move the left corner of the map to (0, 0), so that we can index into the map array. The array of tiles should be set up so that `Map[0][0]` is the top left tile and `Map[mapHeight][mapWidth]` is the bottom right tile. The layout of the array should match the visual layout of the map.

```C
int Map[MAP_HEIGHT][MAP_WIDTH] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
```

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/tilemap/blank-map.png" height=300 alt="Screenshot" vspace=20>

The map offset is half the width and height of the map, in tiles, multiplied by the tile size. Save this value so you can access it during the collision calculations.

```C
mapOffset.x = mapWidthInTiles / 2.0f * tileWidth
mapOffset.y = mapHeightInTiles / 2.0f * tileHeight
```

---

## Move along one axis only

First we apply the movement vector to the player position for either the X or Y axis, then we run the following four steps to check for collision and, if necessary, reset the player position, and then we repeat the steps for the other axis. Doing each movement axis separately is key to having the collision resolution work cleanly.

```C
playerPos.x += playerMovement.x
```

---

## 1. Get the bounding tiles

We need to know which tiles are surrounding the player so that we can check collision with them. If we know the rows and columns that the edges of the player are touching, we will know the set of tiles that we need to check.

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/tilemap/edges.png" height=300 alt="Screenshot" vspace=20>
<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/tilemap/edges-highlight.png" height=300 alt="Screenshot" vspace=20>

We can find out the index of each row and column by taking the position of each edge of the player, adding the map offset, dividing by the size of the tiles, and casting the result to an integer.

```C
leftColumn = (int)((playerPos.x - halfPlayerWidth + mapOffset.x) / tileSize)
rightColumn = (int)((playerPos.x + halfPlayerWidth + mapOffset.x) / tileSize)
topRow = (int)((playerPos.y - halfPlayerHeight + mapOffset.y) / tileSize)
bottomRow = (int)((playerPos.y + halfPlayerHeight + mapOffset.y) / tileSize)
```

If the player can reach the edge of the map, make sure to cap these indexes appropriately.

----

## 2. Step through each tile around the player

Using the column and row indexes that we calculated, step through the tiles around the player, and check if this is a tile we need to collide with.

```C
for (int column = leftColumn; column <= rightColumn; ++column)
{
    for (int row = topRow; row <= bottomRow; ++row)
    {
        if (Map[row][column] == WALL)
        ...
    }
}
```

---

## 3. Calculate the tile position and check collision

Turn the tile index into its actual position by multiplying by the tile size and subtracting the map offset, then use the player and tile positions and sizes to check collision using a box-box collision detection method.

```C
tilePos.x = column * tileSize - mapOffset.x + halfTileSize
tilePos.y = row * tileSize - mapOffset.y + halfTileSize

if (CheckCollision(playerPos.x, playerPos.y, playerSize.x, playerSize.y, tilePos.x, tilePos.y, tileSize.x, tileSize.y))
...
```

---

## 4. Reset position depending on axis of movement

If the player is colliding with the tile, we will reset their position depending on the axis we are moving on, moving them to be just outside the edge of the tile.

```C
if (movingHorizontally)
{
    if (tilePos.x < playerPos.x)
        playerPos.x = tilePos.x + halfTileSize + halfPlayerWidth + 1
    else
        playerPos.x = tilePos.x - halfTileSize - halfPlayerWidth - 1
}
else
{
    if (tilePos.y < playerPos.y)
        playerPos.y = tilePos.y + halfTileSize + halfPlayerHeight + 1
    else
        playerPos.y = tilePos.y - halfTileSize - halfPlayerHeight - 1
}
```

---

## Move along the other axis, repeat collision steps

Now we need to apply the player movement in the other axis and repeat the same four steps.

```C
playerPos.y += playerMovement.y

...
```