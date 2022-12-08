# Creating a New Visual Studio Project to Use the DGL

These instructions will cover creating a Visual Studio 2022 project using C and setting it up to use the DGL files. It uses the DGL folder with the header and library files, which can be found on the [Releases](https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/releases) page. 

You can also look at the [DGL Template Project](https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/tree/main/DGL_Template_Project) to see an example of a working project.

---

## Creating the Visual Studio Project

1. Open Visual Studio 2022, and in the launcher screen select the ***Create a new project*** option.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture1.png" height=300 alt="Screenshot">

2. Scroll down until you find the ***Windows Desktop Application*** option, select it, and then click the "Next" button. This will create the project with some settings that are needed for a desktop application. 

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture2.png" height=300 alt="Screenshot">

3. Change the project name to whatever you want, and choose the location where you want to save the project.
    - A folder will be created for the project, with the project name, so you don't need to create a new folder for it.
    - Keep the ***Place solution and project in the same directory*** option un-checked. This will create a sub-folder for the project files, which we want to do. 
    - Click on the ***Create*** button to create the new project.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture3.png" height=300 alt="Screenshot">

4. When you look at the folder that was created for your project, you should see a .sln file with the name of the project, and a folder with the same name. 

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture4.png" alt="Screenshot">

    Inside the sub-folder you should see a list of files like this (with your project name instead of "Awesome Game").

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture5.png" alt="Screenshot">

5. Put the DGL folder into the base folder of your project. This will make the library files accessible to your project, while keeping them contained in their own folder.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture6.png" alt="Screenshot">

---

## Changing the Project Settings

Now that we have our project, we need to change some settings. 

1. Double-click on the .sln file to open the solution, then right-click on the project name to open the project property settings. 

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture11a.png" height=300 alt="Screenshot">

2. Make sure that the "Configuration" setting is set to "All Configurations" and the "Platform" setting is set to "All Platforms".

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture12.png" height=300 alt="Screenshot">

2. The first thing we will do is add the directories needed to find the DGL files. We'll start with the directory to look in for includes.
    - Click on the ***VC++ Directories*** section. 
    - For the ***Include Directories*** option, click on the drop-down arrow on the right end of the field, and open the edit window. 

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture13.png" height=300 alt="Screenshot">

    - Use the ***New Line*** button or double-click on the text field to add a new entry. 
    - Enter the following text: **`$(SolutionDir)DGL\inc`**. Press **Enter** to save the text after you type it.
         - *You can see what this macro expands to in the **Evaluated value** section just below the text window.*
    
    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture14.png" height=300 alt="Screenshot">

    - This tells the project to look for include files in the `DGL\inc` folder inside the same directory as the solution file. This is where the `DGL.h` file is stored.
    - Click on ***OK*** to save the text and return to the properties window.

2. Next we need to set the directory to look in for library files.
    - Edit the ***Library Directories*** option and add the following text: **`$(SolutionDir)DGL\lib\$(Platform)`**
    - This tells the project to look for library files in the `DGL\lib\x64` or `DGL\lib\Win32` folder in the same directory as the solution file, depending on which platform is being built.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture14a.png" height=300 alt="Screenshot">

3. Expand the ***C/C++*** list and click on the ***Advanced*** section. Change the ***Compile As*** option to **`Compile as C Code (/TC)`**. (If you were creating a C++ project you would skip this step.)

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture14b.png" height=300 alt="Screenshot">

4. Click on the ***Apply*** button to save the settings so far. 

5. In the ***Configuration*** drop-down, select **`Debug`**. Now we will be editing settings that only apply when building in Debug mode.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture15.png" height=300 alt="Screenshot">

6. Expand the ***Linker*** list and click on the ***Input*** section. 
    - Click on the drop-down arrow in the ***Additional Dependencies*** option to open the edit window. 

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture15a.png" height=300 alt="Screenshot">

    - Enter **`DGL_d.lib`**. This tells the project that, when building in Debug mode, it will need the `DGL_d.lib` file. It will look for this file in the directory that we set up previously.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture16.png" height=300 alt="Screenshot">

    - Click on the ***OK*** button to go back to the properties window, then click on the ***Apply*** button to save the changes.

7. Change the ***Configuration*** setting to **`Release`**, then edit the ***Additional Dependencies*** field again.
    - This time enter **`DGL.lib`**. This is the Release version of the library file. 
    - Click on the ***OK*** button to go back to the properties window, then click on the ***Apply*** button to save the changes.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture17.png" height=300 alt="Screenshot">

8. Change the ***Configuration*** setting back to **`Debug`**. Expand the ***Build Events*** list and click on the ***Post-Build Event*** section.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture18.png" height=300 alt="Screenshot">

    - Edit the ***Command Line*** field and enter the following text: **`xcopy "$(SolutionDir)DGL\lib\$(Platform)\DGL_d.dll" "$(OutDir)" /s /r /y /q`** 

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture18a.png" height=300 alt="Screenshot">

    - This will copy the `DGL_d.dll` file from the `DGL/lib/x64` or `DGL/lib/Win32` folder in the solution directory into the output directory of the project when building in Debug mode.
    - Click on the ***Apply*** button to save the setting.

9. Change the ***Configuration*** setting back to **`Release`** and edit the ***Command Line*** field again. 
    - Enter the following text: **`xcopy "$(SolutionDir)DGL\lib\$(Platform)\DGL.dll" "$(OutDir)" /s /r /y /q`** 

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture19.png" height=300 alt="Screenshot">

    - This will copy the `DGL.dll` file when building in Release mode. 
    - Click on the ***Apply*** button to save the setting.

10. Change the ***Configuration*** setting to ***All Configurations*** and click on the ***Advanced*** section. Change the ***Character Set*** setting to ***Use Multi-Byte Character Set***.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture20.png" height=300 alt="Screenshot">

11. Click on the ***OK*** button to save and close the property settings window.

---

## Changing the Default Files

Visual Studio creates some files for us by default, and we need to edit some of them.

1. When you expand the ***Source Files*** filter in the Solution Explorer you should see a .cpp file with the name of your project.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture7.png" height=300 alt="Screenshot">

2. We need to rename this file. Right-click on the file, choose the ***Rename*** option, and rename the file to **main.c**. (If you were creating a C++ project, you would not need to change the extension.)

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture9.png" height=300 alt="Screenshot">

3. Visual Studio also creates a header for this file, but we won't need it. Expand the ***Header Files*** filter, right-click on the .h file with the project name, and choose the ***Remove*** option. Click on the ***Delete*** button on the pop-up window to delete the file from the folder.

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture10.png" height=300 alt="Screenshot">
    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture10a.png" height=150 alt="Screenshot">

1. We also should remove the **`framework.h`** file, since we don't need that one either.
    - Follow the same process to remove **`framework.h`** from the project.
    - Your file structure should now look like this:

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture10b.png" height=300 alt="Screenshot">

4. Open the **main.c** file. We will be deleting most of the text in the file. 
    - Keep the `WinMain` function, but remove all the code from it.
    - Keep the `WndProc` function, but remove everything except the default case of the switch statement. 
    - Remove the existing include statements, and add includes for **`Resource.h`** and **`DGL.h`**.
    - Remove all other code from the file.
    - You should end up with a file that looks like this:

    <img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/new-project/Picture11.png" height=400 alt="Screenshot">

---

Check that your project compiles. If it does, you're ready to move on to working with the DGL.