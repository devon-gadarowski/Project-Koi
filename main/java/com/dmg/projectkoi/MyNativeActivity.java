package com.dmg.projectkoi;

import android.util.Log;

public class MyNativeActivity extends android.app.NativeActivity
{
	static 
	{
		Log.i("Project-Koi", "Loading libraries...\n");
		System.loadLibrary("vrapi");
	}
}
