package com.akm.window;
import android.os.Bundle;
import android.graphics.Color;
import android.view.Gravity;
import androidx.appcompat.app.AppCompatActivity;
//import androidx.appcompat.widget.AppCompatTextView;

import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import androidx.core.view.WindowInsetsCompat;

//Orientation
import android.content.pm.ActivityInfo;

public class MainActivity extends AppCompatActivity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        getWindow().getDecorView().setBackgroundColor(Color.BLACK);

        /* 
        AppCompatTextView myTextView = new AppCompatTextView(this);
        myTextView.setTextColor(Color.GREEN);
        myTextView.setTextSize(30);
        myTextView.setGravity(Gravity.CENTER);
        myTextView.setText("Hello World RTR 6!!!");
        setContentView(myTextView); 
        */

        GLESView glesView = new GLESView(this);
        setContentView(glesView);

        //Hide action bar
        getSupportActionBar().hide();

        // Tell the android system to make your window expand edge to edge
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
        WindowInsetsControllerCompat windowInsetsController = WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        windowInsetsController.hide(WindowInsetsCompat.Type.systemBars() | WindowInsetsCompat.Type.ime());

        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }
}