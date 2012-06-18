package com.Threading;

import android.app.Activity;
import android.content.Context;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import java.lang.System;

public class ThreadActivity extends Activity{
	Button goBtn;
	TextView text;
	private TextView tv;
	private LocationManager lm;
	
	
	private static native void threader();
	//private static native void setGps(double lat, double lon, double alt);
	private static native void setGps(long shit);
	private static native String goThread();

	static {
		System.loadLibrary("threading");
	}

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		GPSThread gpsThread = new GPSThread(this);
		gpsThread.run();
		
		goBtn = (Button) findViewById(R.id.goBtn);
		text = (TextView) findViewById(R.id.text);
        
		goBtn.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {
				text.setText(goThread());
				threader();
				
				//setGps();

			}
		});

	}
	
	private class GPSThread extends Thread implements LocationListener {
		

	
		private Context mContext;
		private LocationManager lm;
		
		public GPSThread(Context mCont){
			mContext = mCont;
			lm = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
			Log.i("Constructor", "Constructed");
			System.out.println("hello world");
	        lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 500, 1, this);
		}
		
		@Override
	    public void run() {
	        HandlerThread handlerThread = new HandlerThread("GPSThread");
	        handlerThread.start();
	 
	        Looper looper = handlerThread.getLooper();
	 
	        Handler handler = new Handler(looper);
	        setGps(System.currentTimeMillis());


	    }
		
		public void onLocationChanged(Location arg0) {
			
			//setGps(arg0.getLatitude(), arg0.getLongitude(), arg0.getAltitude());
			///delete below later
		
	        //tv.setText("lat="+lat+", lon="+lon);
			setGps(System.currentTimeMillis());
		}
		
		public void onProviderDisabled(String arg0) {
			Log.e("GPS", " provider disabled " + arg0);
			setGps(System.currentTimeMillis());
		}

		public void onProviderEnabled(String arg0) {
			Log.e("GPS"," provider enabled " + arg0);
			setGps(System.currentTimeMillis());
		}

		public void onStatusChanged(String arg0, int arg1, Bundle arg2) {
		    Log.e("GPS", " status changed to " + arg0 + " [" + arg1 + "]");
			setGps(System.currentTimeMillis());
		}
		
	}
}