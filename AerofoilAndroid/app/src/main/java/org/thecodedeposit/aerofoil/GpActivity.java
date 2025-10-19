package org.thecodedeposit.aerofoil;

import org.libsdl.app.SDLActivity;

import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

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

    public void showTextInput() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
            }
        });
    }

    public void hideTextInput() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                View view = getWindow().getDecorView().getRootView();
                InputMethodManager imm = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(view.getWindowToken(), 0);
            }
        });
    }

    public void showInstructions()
    {
        Intent intent = new Intent(this, InstructionActivity.class);
        startActivity(intent);
    }
}
