package com.akm.window;
import android.os.Bundle;
import android.graphics.Color;
import android.view.Gravity;
import androidx.appcompat.app.AppCompatActivity;
//import androidx.appcompat.widget.AppCompatTextView;

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

        MyTextView myTextView = new MyTextView(this);
        setContentView(myTextView);
    }
}