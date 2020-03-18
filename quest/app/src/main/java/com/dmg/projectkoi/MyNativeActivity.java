package com.dmg.projectkoi;

public class MyNativeActivity extends android.app.NativeActivity
{
	static 
	{
		System.loadLibrary("android");
		System.loadLibrary("VkLayer_khronos_validation");
		System.loadLibrary("vrapi");
	}
}
