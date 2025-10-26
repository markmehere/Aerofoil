package org.libsdl.app;

import android.app.Activity;
import android.widget.RelativeLayout;
import android.util.Log;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

public class LoadLibrariesThread extends Thread {
    private static final int SDL_MAJOR_VERSION = 2;
    private static final int SDL_MINOR_VERSION = 32;
    private static final int SDL_MICRO_VERSION = 6;

    // If shared libraries (e.g. SDL or the native application) could not be loaded.
    public static volatile boolean mBrokenLibraries = true;

    private volatile String errorMsgBrokenLib;

    private static String TAG = "LoadLibraries";

    private static final String[] librariesToLoad = new String[] {
        "SDL2",
        // "SDL2_image",
        // "SDL2_mixer",
        // "SDL2_net",
        // "SDL2_ttf",
        "main"
    };

    public LoadLibrariesThread() {
        super(TAG);
    }

    public static String getMainSharedObject(SDLActivity activity) {
        String library;
        if (librariesToLoad.length > 0) {
            library = "lib" + librariesToLoad[librariesToLoad.length - 1] + ".so";
        } else {
            library = "libmain.so";
        }
        return activity.getContext().getApplicationInfo().nativeLibraryDir + "/" + library;
    }

    private void loadLibraries() {
        for (String lib : librariesToLoad) {
            SDL.loadLibrary(lib, SDLActivity.mSingleton);
        }
    }

    private SDLSurface createSDLSurface(Context context) {
        return new SDLSurface(context);
    }

    @Override
    public void run() {
        try {
            loadLibraries();
            mBrokenLibraries = false; /* success */
        } catch(UnsatisfiedLinkError e) {
            System.err.println(e.getMessage());
            mBrokenLibraries = true;
            errorMsgBrokenLib = e.getMessage();
        } catch(Exception e) {
            System.err.println(e.getMessage());
            mBrokenLibraries = true;
            errorMsgBrokenLib = e.getMessage();
        }

        if (!mBrokenLibraries) {
            String expected_version = String.valueOf(SDL_MAJOR_VERSION) + "." +
                                        String.valueOf(SDL_MINOR_VERSION) + "." +
                                        String.valueOf(SDL_MICRO_VERSION);
            String version = SDLActivity.nativeGetVersion();
            if (!version.equals(expected_version)) {
                mBrokenLibraries = true;
                errorMsgBrokenLib = "SDL C/Java version mismatch (expected " + expected_version + ", got " + version + ")";
            }
        }

        if (mBrokenLibraries) {
            SDLActivity.mSingleton.runOnUiThread(() -> {
                AlertDialog.Builder dlgAlert  = new AlertDialog.Builder(SDLActivity.mSingleton);
                dlgAlert.setMessage("An error occurred while trying to start the application. Please try again and/or reinstall."
                    + System.getProperty("line.separator")
                    + System.getProperty("line.separator")
                    + "Error: " + errorMsgBrokenLib);
                dlgAlert.setTitle("SDL Error");
                dlgAlert.setPositiveButton("Exit",
                    new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog,int id) {
                            SDLActivity.mSingleton.finish();
                        }
                    });
                dlgAlert.setCancelable(false);
                dlgAlert.create().show();
            });
        }

        // Set up JNI
        SDL.setupJNI();

        // Initialize state
        SDL.initialize();

        // So we can call stuff from static callbacks
        SDL.setContext(SDLActivity.mSingleton);

        SDLActivity.mClipboardHandler = new SDLClipboardHandler();

        SDLActivity.mHIDDeviceManager = HIDDeviceManager.acquire(SDLActivity.mSingleton);

        SDLActivity.mSingleton.runOnUiThread(() -> {
            // Set up the surface
            SDLActivity.mSurface = createSDLSurface(SDLActivity.mSingleton);

            SDLActivity.mLayout = new RelativeLayout(SDLActivity.mSingleton);
            SDLActivity.mLayout.addView(SDLActivity.mSurface);

            // Get our current screen orientation and pass it down.
            SDLActivity.mCurrentOrientation = SDLActivity.getCurrentOrientation();
            // Only record current orientation
            SDLActivity.onNativeOrientationChanged(SDLActivity.mCurrentOrientation);

            try {
                SDLActivity.mCurrentLocale = SDLActivity.mSingleton.getContext().getResources().getConfiguration().getLocales().get(0);
            }
            catch (Exception ignored) { }

            SDLActivity.mSingleton.setContentView(SDLActivity.mLayout);

            SDLActivity.mSingleton.setWindowStyle(false);

            SDLActivity.mSingleton.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(SDLActivity.mSingleton);

            SDLActivity.mSurface.handleResume();

            SDLActivity.mFullyLoaded.set(true);

            Log.v(TAG, "Initialization complete!");
            
            synchronized (SDLActivity.mLoadLibrariesLock) {
                SDLActivity.mLoadLibrariesLock.notifyAll(); // Notify waiting threads after UI setup
            }
        });
    }

}