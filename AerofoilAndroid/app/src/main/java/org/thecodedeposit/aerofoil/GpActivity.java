package org.thecodedeposit.aerofoil;

import org.libsdl.app.SDLActivity;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;



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

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent)
    {
        if (requestCode == 1111 && resultCode == RESULT_OK && intent != null)
        {
            Uri uri = intent.getData();
            int n = 0;
        }
        else
        {
            super.onActivityResult(requestCode, resultCode, intent);
        }
    }

    public String selectSourceExportPath(String fname)
    {
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT)
                .setType("application/zip")
                .addCategory(Intent.CATEGORY_OPENABLE)
                .putExtra(Intent.EXTRA_TITLE, "SourceCode.zip");
        startActivityForResult(intent, 1111);
        return "";
    }
}
