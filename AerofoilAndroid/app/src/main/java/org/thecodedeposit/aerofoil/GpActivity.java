package org.thecodedeposit.aerofoil;

import org.libsdl.app.SDLActivity;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.OutputStream;

public class GpActivity extends SDLActivity
{
    private static final int SOURCE_EXPORT_REQUEST_ID = 20;

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

    public void openGithub()
    {
        Uri githubUrl = Uri.parse("https://github.com/markmehere/Aerofoil");
        Intent intent = new Intent(Intent.ACTION_VIEW, githubUrl);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                startActivity(intent);
            }
        });
    }
}
