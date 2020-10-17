package org.thecodedeposit.aerofoil;

import org.libsdl.app.SDLActivity;
import android.content.res.AssetManager;
import android.os.Bundle;

public class GpActivity extends SDLActivity
{
    private AssetManager assetManager;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        assetManager = getAssets();
    }

    public String[] scanAssetDirectory(String directory)
    {
        try
        {
            return this.assetManager.list(directory);
        }
        catch (java.io.IOException ex)
        {
            return new String[0];
        }
    }
}
