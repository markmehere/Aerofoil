package org.libsdl.app;

import android.util.Log;

/**
    Simple runnable to start the SDL application
*/
public class SDLMain implements Runnable {
    @Override
    public void run() {
        while (!SDLActivity.mFullyLoaded.get()) {
            synchronized (SDLActivity.mLoadLibrariesLock) {
                try {
                    Log.v("SDL", "Waiting for intialization...");
                    SDLActivity.mLoadLibrariesLock.wait(); // Wait for the thread to notify
                    if (SDLActivity.mFullyLoaded.get()) {
                        Log.v("SDL", "Initialization complete!");
                    }
                } catch (InterruptedException e) {
                    Log.e("SDL", "Thread interrupted: " + e.getMessage());
                }
            }
        }
        
        String library = LoadLibrariesThread.getMainSharedObject(SDLActivity.mSingleton);
        String function = SDLActivity.mSingleton.getMainFunction();
        String[] arguments = SDLActivity.mSingleton.getArguments();

        try {
            android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_DISPLAY);
        } catch (Exception e) {
            Log.v("SDL", "modify thread properties failed " + e.toString());
        }
        
        Log.v("SDL", "Running main function " + function + " from library " + library);

        SDLActivity.nativeRunMain(library, function, arguments);

        Log.v("SDL", "Finished main function");

        if (SDLActivity.mSingleton != null && !SDLActivity.mSingleton.isFinishing()) {
            // Let's finish the Activity
            SDLActivity.mSDLThread = null;
            SDLActivity.mSingleton.finish();
        }  // else: Activity is already being destroyed

    }
}
