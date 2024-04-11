   function Component() { // default constructor
   installer.installationFinished.connect(this, Component.prototype.installationFinished);
}

Component.prototype.createOperations = function() {
   component.createOperations();
   if(systemInfo.productType === "windows") {

      component.addOperation("CreateShortcut", "@TargetDir@/bin/SpotifyEnhancer.exe", "@StartMenuDir@/SpotifyEnhancer.lnk", "description=This is Spotify Pro!");

      component.addOperation("CreateShortcut", "@TargetDir@/bin/SpotifyEnhancer.exe", "@HomeDir@/Desktop/SpotifyEnhancer.lnk", "description=This is Spotify Pro!");

   }
}
Component.prototype.installationFinished = function()  {

}
