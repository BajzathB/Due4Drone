package com.example.d4djv3;

import android.view.MotionEvent;
import androidx.appcompat.app.AppCompatActivity;

public class BaseActivity extends AppCompatActivity
{
    float x1,y1,x2,y2;

    @Override
    public boolean onTouchEvent(MotionEvent e)
    {
        switch (e.getAction())
        {
            case MotionEvent.ACTION_DOWN:
            {
                x1 = e.getX();
                y1 = e.getY();
                return true;
            }
            case MotionEvent.ACTION_UP:
            {
                x2 = e.getX();
                y2 = e.getY();
                float dx = x2 - x1;

                handleSwipe(dx);
                return true;
            }
        }

        return super.onTouchEvent(e);
    }

    private void handleSwipe(float dx)
    {
        if (!BTSocket.getInstance().getBluetoothConnection())
        {
//            setBTTexts("", "BT not connected yet");
            return;
        }

        if (dx < -250)
        {
            onSwipeLeft();
        }
        else if (dx > 250)
        {
            onSwipeRight();
        }
    }

    // override these in child activities
    protected void onSwipeLeft() {}
    protected void onSwipeRight() {}
}
