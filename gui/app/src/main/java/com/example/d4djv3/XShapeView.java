package com.example.d4djv3;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.View;

public class XShapeView extends View {
    private Paint paint;

    public XShapeView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public XShapeView(Context context) {
        super(context);
        init();
    }

    private void init() {
        paint = new Paint();
        paint.setColor(0xFFFF0000); // Red color
        paint.setStrokeWidth(10); // Line thickness
        paint.setAntiAlias(true);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        int width = getWidth();
        int height = getHeight();

        // Draw the two diagonal lines forming an X
        canvas.drawLine(0, 0, width, height, paint);
        canvas.drawLine(width, 0, 0, height, paint);
    }
}