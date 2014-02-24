package com.brandymint.kitchentimer;

import android.content.Context;
import android.app.Activity;
import android.app.ActivityManager;
import android.util.Log;
import android.content.Intent;
import android.view.Window;
import android.view.WindowManager;
import android.os.Bundle;


public class MainActivity extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static MainActivity m_instance;
    
    public MainActivity ()
	{
	    m_instance = this;
	}
    @Override public void onCreate (Bundle savedInstanceState)
	{
	    super.onCreate (savedInstanceState);
	    final Window window = getWindow ();
	    int flags =
		WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD |
		WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON |
		WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON |
		WindowManager.LayoutParams.FLAG_FULLSCREEN |
		WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED;
	    if (window != null)
		window.setFlags (flags, flags);
	}

    public void wake_method ()
	{
	    final ActivityManager am = (ActivityManager) getSystemService (Context.ACTIVITY_SERVICE);
	    am.moveTaskToFront (getTaskId (), 0);
	}
    
    public static void wake ()
	{
	    m_instance.wake_method ();
	}
}
