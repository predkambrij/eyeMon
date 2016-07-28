package org.blatnik.eyemon;

import org.blatnik.eyemon.R;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

public class FdActivity extends Activity {
    private static final String TAG = "FdActivity";

    private MenuItem startService;
    private MenuItem stopService;
    private MenuItem farneback;
    private MenuItem template;
    
    private BroadcastReceiver receiver;


    public FdActivity() {
    }

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Window
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.face_detect_surface_view);

        final Button startStopButton = (Button) findViewById(R.id.cameraRecorderStartStopService);
        startStopButton.setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (MainService.serviceState == false) {
                        MainService.serviceState = true;
                        startStopButton.setText("Stop");
                        Log.i(TAG, "Staaart");
                        Intent intent = new Intent(FdActivity.this, MainService.class);
                        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        startService(intent);
                    } else {
                        startStopButton.setText("Start");
                        MainService.serviceState = false;
                        Log.i(TAG, "Stooop");
                        stopService(new Intent(FdActivity.this, MainService.class));
                    }
                }
            }
        );

        final Button cameraSelectorButton = (Button) findViewById(R.id.cameraRecorderCameraSelector);
        cameraSelectorButton.setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (MainService.frontCamera == false) {
                        MainService.frontCamera = true;
                        cameraSelectorButton.setText("Back");
                    } else {
                        MainService.frontCamera = false;
                        cameraSelectorButton.setText("Front");
                    }
                }
            }
        );

        Button farnebackButton = (Button) findViewById(R.id.cameraRecorderFarneback);
        farnebackButton.setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    MainService.method = MainService.METHOD_FARNEBACK;
                }
            }
        );
        Button templateButton = (Button) findViewById(R.id.cameraRecorderTemplate);
        templateButton.setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    MainService.method = MainService.METHOD_TEMPLATE_JNI;
                }
            }
        );
        
    }

    public void onPause() {
        super.onPause();
    }

    public void onResume() {
        super.onResume();
    }

    public void onDestroy() {
        super.onDestroy();
    }
    @Override
    protected void onStart() {
        super.onStart();

        receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                double frameRate = Double.parseDouble(intent.getStringExtra("lastFrameRate"));
                double avgMethodCallTime = Double.parseDouble(intent.getStringExtra("avgMethodCallTime"));
                double avgOtherTime = Double.parseDouble(intent.getStringExtra("avgOtherTime"));
                ImageView image = (ImageView) findViewById(R.id.imageView1);
                image.setImageBitmap(MainService.bitmapImage);
                TextView tv1 = (TextView) findViewById(R.id.textView1);
                tv1.setText(String.format("Frame rate: %.2f", frameRate));
                TextView tv2 = (TextView) findViewById(R.id.textView2);
                tv2.setText(String.format("Method: %.2f", avgMethodCallTime));
                TextView tv3 = (TextView) findViewById(R.id.textView3);
                tv3.setText(String.format("Other: %.2f", avgOtherTime));
            }
        };
        IntentFilter filter = new IntentFilter();
        filter.addAction(MainService.IMAGE_UPDATE_RESULT); 
        registerReceiver(receiver, filter);
    }

    @Override
    protected void onStop() {
        unregisterReceiver(receiver);
        super.onStop();
    }
    public boolean onCreateOptionsMenu(Menu menu) {
        startService = menu.add("startService");
        stopService  = menu.add("stopService");
        farneback  = menu.add("farneback");
        template  = menu.add("template");
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        if (item == startService) {
            Intent intent = new Intent(FdActivity.this, MainService.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startService(intent);
        } else if (item == stopService) {
            stopService(new Intent(FdActivity.this, MainService.class));
        } else if (item == farneback) {
            MainService.method = MainService.METHOD_FARNEBACK;
        } else if (item == template) {
            MainService.method = MainService.METHOD_TEMPLATE;
        }

        return true;
    }
}
