package com.akm.window;
import android.os.Bundle;
import android.graphics.Color;
import android.view.Gravity;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.AppCompatTextView;
import android.content.Context;

//Event related packages
import android.view.MotionEvent;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.GestureDetector.OnDoubleTapListener;

public class MyTextView extends AppCompatTextView implements OnGestureListener, OnDoubleTapListener
{
    //Event related variables
    private GestureDetector gestureDetector;

    public MyTextView(Context context)
    {
        super(context);
        setTextColor(Color.GREEN);
        setTextSize(30);
        setGravity(Gravity.CENTER);
        setText("Hello World RTR 6!!!");

        gestureDetector = new GestureDetector(context, this, null, false);

        //Set this class as double tap listener
        gestureDetector.setOnDoubleTapListener(this);
    }
    
    //Event related methods

    // Implement one method from view
    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        int eventaction = event.getAction();
        if(!gestureDetector.onTouchEvent(event))
            super.onTouchEvent(event);
        return true;
    }
    
    //OnGestureListener
    @Override
    public boolean onDown(MotionEvent e)
    {
        setTextColor(Color.GREEN);
        setText("onDown");
        return true;
    }

    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
    {
        setTextColor(Color.GREEN);
        setText("onFling");
        return true;
    }

    @Override
    public void onLongPress(MotionEvent e)
    {
        setTextColor(Color.GREEN);
        setText("onLongPress");
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) //Will be used as swipe
    {
        setTextColor(Color.GREEN);
        setText("onScroll");
        return true;
    }

    @Override
    public void onShowPress(MotionEvent e)  
    {
        setTextColor(Color.GREEN);
        setText("onShowPress");
    }

    @Override
    public boolean onSingleTapUp(MotionEvent e)
    {
        setTextColor(Color.GREEN);
        setText("onSingleTapUp");
        return true;
    }

    //OnDoubleTapListener
    @Override
    public boolean onDoubleTap(MotionEvent e)
    {
        setTextColor(Color.RED);
        setText("onDoubleTap");
        return true;
    }

    @Override
    public boolean onDoubleTapEvent(MotionEvent e)
    {
        setTextColor(Color.GREEN);
        setText("onDoubleTapEvent");
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent e)
    {
        setTextColor(Color.GREEN);
        setText("onSingleTapConfirmed");
        return true;
    }
}