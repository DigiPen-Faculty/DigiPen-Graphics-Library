# Creating a New Visual Studio Project

Open Visual Studio and select the "Create a new project" option.

![Test](images/new-project/Picture1.png)

Scroll down until you find the "Windows Desktop Application" option, select it, and then click the "Next" button.

![Test](images/new-project/Picture2.png)

Change the project name to whatever you want, and choose the location where you want to save the project. Keep the "Place solution and project in the same directory" option un-checked. This will create a sub-folder for the project files, which we want to do. Then click on the "Create" button.

![Test](images/new-project/Picture3.png)

When you look at the folder where you created your project, you should see a .sln file with the name of the project, and a folder with the same name. 

![Test](images/new-project/Picture4.png)

Inside the folder you should see a list of files like this (with your project name instead of "New Game Project").

![Test](images/new-project/Picture5.png)

Put the DGL folder into the base folder of your project, like this:

![Test](images/new-project/Picture6.png)

Double-click on the .sln file to open the solution. When you expand the "Source Files" filter you should see a .cpp file with the name of your project.

![Test](images/new-project/Picture7.png)

Right-click on the file, choose the "Rename" option, and rename the file to main.c.

![Test](images/new-project/Picture8.png) ![Test](images/new-project/Picture9.png)

Expand the "Header Files" filter, and delete the .h file with your project name. We won't be using this file.

![Test](images/new-project/Picture10.png)

Open the main.c file. We will be deleting most of the text in the file. You will want to keep the WinMain and WndProc functions, but remove all the code from WinMain and most of the code from WndProc. You should end up with a file that looks like this:

![Test](images/new-project/Picture11.png)

Right-click on the project name to open the project property settings. Make sure that the "Configuration" setting is set to "All Configurations" and the "Platform" setting is set to "All Platforms".

![Test](images/new-project/Picture12.png)