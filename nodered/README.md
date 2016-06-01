This folder contains exported flows from the Node Red app which supports the Solar Demo.  Open each txt file, copy the text & use the import from clipboard capability in Node Red to import each of the flows into a separate tab.

For the app to work correctly, some images need to be imported into the Node Red app from the ZIP file contained in this folder.  Do do so, first create a Git repository using the ADD GIT item at the top right of the overview screen for your Node Red app.  Once it has been created, navigate to the GIT repository & import the images as follows:

![GIT Folders(gitfolders.jpg)

and restage your application.

If the images have been installed correctly, you will be able to see them as the following URL (for the bat0.png image)

http://yourapplicationpath.mybluemix.net/images/bat0.png

Note: Do not use the top level /images folder - they must be stored in /public/images
