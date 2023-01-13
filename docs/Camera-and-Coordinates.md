# How Does the Camera Work?

Let's say that we have a game scene that is 800 by 500 pixels. The scene is centered at the origins of the world, so the top left corner would be $(-250, 400)$ and the bottom right corner would be $(250, -400)$.

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/camera/scene-coordinates.png" height=300 alt="A rectangular image with X and Y axes" vspace=20>

However, the size of our game window is only 400 by 250 pixels, so what the user sees is a smaller subsection of this scene.

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/camera/camera1.png" height=300 alt="Shows a smaller centered rectangle inside the scene" vspace=20>

In this case we could say that we have a camera positioned at $(0, 0)$, with a size of 400 x 250, so it shows the game world from $(-125, 200)$ at the top left corner of the view to $(125, -200)$ at the bottom right corner of the view.

If we move the camera to a position of $(100, -60)$ then it will show a different subsection of the scene, from $(-100, 65)$ at the top left to $(300, -185)$ at the bottom right.

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/camera/camera2.png" height=300 alt="The camera rectangle has shifted right and down" vspace=20>

## Camera Implementation

We can easily create a camera for ourselves simply by offsetting the position of all the game objects by the camera position. If the camera position is $(100, -60)$, that means that the world position of $(0, 0)$ is $(-100, 60)$ relative to the camera. So, if we subtract the camera's position from each object's position when we draw it, we will move the camera's view as its position changes. Whether you think of the camera moving and the world standing still or the camera standing still and the world moving around it, the result is the same to the user.

This works fine for simple 2D scenarios, but most of the time we take advantage of the camera-related functionality in whichever graphics output API we are using. For example, the DGL uses DirectX for graphics output, and as part of this a matrix is created which translates a game object's coordinates into where (or if) it's shown on screen.


# Screen Coordinates

The size of the game window stays the same, of course, no matter where the camera moves to. If we are measuring coordinates relative to the window, we usually measure from $(0, 0)$ at the top left corner to $(windowWidth, windowHeight)$ at the bottom right corner. This is usually referred to as the **screen coordinates** while the in-game coordinates are called the **world coordinates**. 

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/camera/camera3.png" height=300 alt="Shows text with the screen coordinates" vspace=20>

If we want to know where the user's mouse cursor is, we get that information in screen coordinates, because the mouse position is relative to the window.

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/camera/camera4.png" height=300 alt="Screen coordinates of a mouse cursor" vspace=20>

If we want to know where the cursor is relative to the objects in the game, then we will need to translate that screen coordinate into world coordinates. 

In this case our camera is at $(100, -60)$, the screen size is 400 x 250, and the mouse position is $(300, 80)$. We need to do two steps:

1. Adjust the mouse position so that it's relative to the center of the screen
1. Translate that adjusted position by the camera position

To adjust the mouse position we want to subtract half the width and height from its coordinates, and then flip the sign of the Y coordinate. This will give us the position relative to the center of the screen, instead of the top corner. Our position of $(300, 60)$ becomes:
    
$$(300 - 200, (80 - 125) * -1) = (100, (-45) * -1) = (100, 45)$$

If we add the camera position to this adjusted mouse position we get:

$$(100 + 100, 45 - 60) = (200, -15)$$

And now we know where in our game world the mouse cursor is currently located.

## Screen to World Function in DGL

Since the camera in DGL also has zoom functionality, it isn't always as simple as these calculations. Instead of doing this adjustment directly the DGL takes the view matrix that was created for DirectX and uses that to convert a set of screen coordinates into the equivalent world coordinates. 