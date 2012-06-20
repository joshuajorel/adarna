package com.Threading;

import android.app.Activity;
import android.content.Context;
import android.location.GpsStatus;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import java.lang.System;

public class ThreadActivity extends Activity implements Handler.Callback{
	Button goBtn, stopBtn;
	private TextView text;
	private LocationManager lm;
	private final int UPDATE_TEXT = 1;
	private boolean gpsflag = true;
	private Handler updateTextView;
	private static native void threader();
	//private static native void setGps(double lat, double lon, double alt);
	private static native void setGps(double lat, double lon, double alt, float accuracy);
	private static native void stop();

	static {
		System.loadLibrary("threading");
	}
	
	GPSThread gpsThread;
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		gpsThread = new GPSThread(this);
		
		goBtn = (Button) findViewById(R.id.goBtn);
		stopBtn = (Button) findViewById(R.id.stopBtn);
		text = (TextView) findViewById(R.id.text);
		
		updateTextView = new Handler(this);
		
		goBtn.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {
				
				//threader();
				if(gpsflag) {
					gpsThread.start();
					gpsflag = false;
				}
					gpsThread.startGPS();
					//callThreader();

				//setGps();

			}
		});
		
		stopBtn.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {
				text.setText("Sensor Acquisition Stopped");
				stop();
				gpsThread.endGPS();
			}
		});

	}
	private void callThreader(){
		threader();
	}
	
	
	
	private class GPSThread extends Thread implements LocationListener, GpsStatus.Listener {
	
		private Context mContext;
		private LocationManager lm;
		
		public GPSThread(Context mCont){
			mContext = mCont;
			lm = (LocationManager) getSystemService(Context.LOCATION_SERVICE);
			lm.addGpsStatusListener(this);
			Log.i("Constructor", "Constructed");
			System.out.println("hello world");
		}
		
		public void startGPS() {
			lm.requestLocationUpdates(LocationManager.GPS_PROVIDER, 500, 1, this);
			Log.i("gps", lm.GPS_PROVIDER);
		}
		@Override
	    public void run() {
	        HandlerThread handlerThread = new HandlerThread("GPSThread");
	        handlerThread.start();
	 
	        Looper looper = handlerThread.getLooper();
	 
	        Handler handler = new Handler(looper);


	    }
		
		public void onLocationChanged(Location arg0) {
			
			//setGps(arg0.getLatitude(), arg0.getLongitude(), arg0.getAltitude());
			///delete below later
		
	        //tv.setText("lat="+arg0.getLatitude()+", lon="+arg0.getLongitude()+", alt="+arg0.getAltitude());
			setGps(arg0.getLatitude(), arg0.getLongitude(), arg0.getAltitude(), arg0.getAccuracy());
			
		}
		
		public void onProviderDisabled(String arg0) {
		}

		public void onProviderEnabled(String arg0) {
		}

		public void onStatusChanged(String arg0, int arg1, Bundle arg2) {
			
		}
		
		public void endGPS() {
			lm.removeUpdates(this);
		}

		public void onGpsStatusChanged(int event) {
			Log.i("GPS", "status "+ event);
			if(event == GpsStatus.GPS_EVENT_FIRST_FIX){
				Log.i("GPS Start", "Calling threader");
				String myShit = new String("Sensor Acquisition Start");
				updateTextView.obtainMessage(UPDATE_TEXT, myShit).sendToTarget();
			}
		}
	}



	public boolean handleMessage(Message msg) {
		if(msg.what == UPDATE_TEXT){
			text.setText((String)msg.obj);
			callThreader();
			return true;
		}
		return false;
	}
}