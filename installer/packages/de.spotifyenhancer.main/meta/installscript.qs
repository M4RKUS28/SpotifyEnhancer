function Component()
{
	// default constructor
}

Component.prototype.createOperations = function()
{
	// call default implementation to actually install Fraktalgenerator.exe
	component.createOperations();
	
	if(systemInfo.productType === "windows") {
		component.addOperation("CreateShortcut", "@TargetDir@/bin/SpotifyEnhancer.exe",
							"@StartMenuDir@/SpotifyEnhancer.lnk",
							"description=A programm to automatically restart Spotify when advertisement is played.");
		//component.addOperation("CreateShortcut", "@TargetDir@/bin/SpotifyEnhancer.exe",
		//					"@HomeDir@/Desktop/SpotifyEnhancer.lnk",
		//					"description=A programm to automatically restart Spotify when advertisement is played.");					
	
	}


}