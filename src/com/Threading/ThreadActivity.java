package com.Threading;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class ThreadActivity extends Activity {
	Button goBtn;
	TextView text;

	private static native void threader();
	//private static native void passGps(int x);

	private static native String goThread();

	static {
		System.loadLibrary("threading");
	}

	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		goBtn = (Button) findViewById(R.id.goBtn);
		text = (TextView) findViewById(R.id.text);

		goBtn.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {
				text.setText(goThread());
				threader();
				/*while(threader() != 1){
					
				}
				text.setText("Sensor Acquisition Complete Bitch");*/
			}
		});

	}
}