function Component() {
   if (installer) {
      installer.installationFinished.connect(this, Component.prototype.installationFinished);
   }
}

Component.prototype.createOperations = function() {
   component.createOperations();
   if(systemInfo.productType === "windows") {

      component.addOperation("CreateShortcut", "@TargetDir@/bin/SpotifyEnhancer.exe", "@StartMenuDir@/SpotifyEnhancer.lnk", "description=SpotifyEnhancer runs silently in the background and monitors the Spotify window title. As soon as an advertisement is detected, it closes Spotify and immediately restarts it, resuming your music without interaction.");

      component.addOperation("CreateShortcut", "@TargetDir@/bin/SpotifyEnhancer.exe", "@HomeDir@/Desktop/SpotifyEnhancer.lnk", "description=SpotifyEnhancer runs silently in the background and monitors the Spotify window title. As soon as an advertisement is detected, it closes Spotify and immediately restarts it, resuming your music without interaction.");

   }
}
Component.prototype.installationFinished = function()  {
   try {
       if (installer.isInstaller()) {
           if (installer.status == QInstaller.Success) {
               if (systemInfo.productType === "windows") {
                   // Execute SpotifyEnhancer after installation
                   if(installer.executeDetached("@TargetDir@/bin/SpotifyEnhancer.exe", ["show"]) !== 0) {
                       print("Failed to start the application!");
                   } else {
                       print("Application started successfully.");
                   }
               } else {
                   print("Not on Windows. Skipping application execution.");
               }
           } else {
               print("Installation was not successful. Application will not be started.");
           }
       } else {
           print("Not in installer mode. Application will not be started.");
       }
   } catch(e) {
       console.log(e);
   }

}
