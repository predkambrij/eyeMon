package org.opencv.samples.facedetect;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;

public class FdActivity extends Activity {
    private static final String TAG = "FdActivity";

    private MenuItem startService;
    private MenuItem stopService;


    public FdActivity() {
    }

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Window
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.face_detect_surface_view);

        Button startButton = (Button) findViewById(R.id.cameraRecorderStartService);
        startButton.setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.i(TAG, "Staaart");
                    Intent intent = new Intent(FdActivity.this, MainService.class);
                    intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startService(intent);
                }
            }
        );
        Button stopButton = (Button) findViewById(R.id.cameraRecorderStopService);
        stopButton.setOnClickListener(
            new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Log.i(TAG, "Stooop");
                    stopService(new Intent(FdActivity.this, MainService.class));
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

    public boolean onCreateOptionsMenu(Menu menu) {
        startService = menu.add("startService");
        stopService  = menu.add("stopService");
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        if (item == startService) {
            Intent intent = new Intent(FdActivity.this, MainService.class);
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startService(intent);
        } else if (item == stopService) {
            stopService(new Intent(FdActivity.this, MainService.class));
        }

        return true;
    }
}
