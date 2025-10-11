package com.akm.window;
import android.os.Bundle;
import android.graphics.Color;
import android.view.Gravity;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatTextView;
import android.content.Context;

public class MyTextView extends AppCompatTextView
{
    public MyTextView(Context context)
    {
        super(context);
        setTextColor(Color.GREEN);
        setTextSize(30);
        setGravity(Gravity.CENTER);
        setText("Hello World RTR 6!!!");
    }
}