package com.example.d4djv3;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

import androidx.activity.EdgeToEdge;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class MeasurementActivity extends BaseActivity
{
    TextView charCtrMin;
    TextView charCtrMax;
    TextView hdrChrCnt;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_measurement);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.measurement), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        Log.d("BT", "MeasurementActivity - onCreate - called");


        BTSocket.getInstance().setBTcmdResultTextID(findViewById(R.id.textViewBTcmdResult));
        BTSocket.getInstance().setBTCmdLamp(findViewById(R.id.buttonCmdLamp));

        charCtrMin = findViewById(R.id.textViewCharCtrMin);
        charCtrMax = findViewById(R.id.textViewCharCtrMax);
        hdrChrCnt = findViewById(R.id.textViewHdrCharCnt);

        //gyro
        final CheckBox checkBoxGrawX = findViewById(R.id.checkBoxGrawX);
        final CheckBox checkBoxGrawY = findViewById(R.id.checkBoxGrawY);
        final CheckBox checkBoxGrawZ = findViewById(R.id.checkBoxGrawZ);
        final CheckBox checkBoxGpt1X = findViewById(R.id.checkBoxGpt1X);
        final CheckBox checkBoxGpt1Y = findViewById(R.id.checkBoxGpt1Y);
        final CheckBox checkBoxGpt1Z = findViewById(R.id.checkBoxGpt1Z);
        final CheckBox checkBoxGpt2X = findViewById(R.id.checkBoxGpt2X);
        final CheckBox checkBoxGpt2Y = findViewById(R.id.checkBoxGpt2Y);
        final CheckBox checkBoxGpt2Z = findViewById(R.id.checkBoxGpt2Z);
        //acc
        final CheckBox checkBoxArawX = findViewById(R.id.checkBoxArawX);
        final CheckBox checkBoxArawY = findViewById(R.id.checkBoxArawY);
        final CheckBox checkBoxArawZ = findViewById(R.id.checkBoxArawZ);
        final CheckBox checkBoxApt1X = findViewById(R.id.checkBoxApt1X);
        final CheckBox checkBoxApt1Y = findViewById(R.id.checkBoxApt1Y);
        final CheckBox checkBoxApt1Z = findViewById(R.id.checkBoxApt1Z);
        final CheckBox checkBoxApt2X = findViewById(R.id.checkBoxApt2X);
        final CheckBox checkBoxApt2Y = findViewById(R.id.checkBoxApt2Y);
        final CheckBox checkBoxApt2Z = findViewById(R.id.checkBoxApt2Z);
        //angle
        final CheckBox checkBoxAngleRawRoll = findViewById(R.id.checkBoxAngleRawRoll);
        final CheckBox checkBoxAngleRawPitch = findViewById(R.id.checkBoxAngleRawPitch);
        final CheckBox checkBoxAnglePT1Roll = findViewById(R.id.checkBoxAnglept1Roll);
        final CheckBox checkBoxAnglePT1Pitch = findViewById(R.id.checkBoxAnglept1Pitch);
        final CheckBox checkBoxAnglePT2Roll = findViewById(R.id.checkBoxAnglept2Roll);
        final CheckBox checkBoxAnglePT2Pitch = findViewById(R.id.checkBoxAnglept2Pitch);
        final CheckBox checkBoxAngleKFrawRoll = findViewById(R.id.checkBoxAngleKFrawRoll);
        final CheckBox checkBoxAngleKFrawPitch = findViewById(R.id.checkBoxAngleKFrawPitch);
        final CheckBox checkBoxAngleKFPT10Roll = findViewById(R.id.checkBoxAngleKFpt10Roll);
        final CheckBox checkBoxAngleKFPT10Pitch = findViewById(R.id.checkBoxAngleKFpt10Pitch);
        final CheckBox checkBoxAngleKFPT20Roll = findViewById(R.id.checkBoxAngleKFpt20Roll);
        final CheckBox checkBoxAngleKFPT20Pitch = findViewById(R.id.checkBoxAngleKFpt20Pitch);
        final CheckBox checkBoxAngleKFPT11Roll = findViewById(R.id.checkBoxAngleKFpt11Roll);
        final CheckBox checkBoxAngleKFPT11Pitch = findViewById(R.id.checkBoxAngleKFpt11Pitch);
        final CheckBox checkBoxAngleKFPT21Roll = findViewById(R.id.checkBoxAngleKFpt21Roll);
        final CheckBox checkBoxAngleKFPT21Pitch = findViewById(R.id.checkBoxAngleKFpt21Pitch);
        final CheckBox checkBoxAngleKFPT22Roll = findViewById(R.id.checkBoxAngleKFpt22Roll);
        final CheckBox checkBoxAngleKFPT22Pitch = findViewById(R.id.checkBoxAngleKFpt22Pitch);
        //PID control
        final CheckBox checkBoxrefSigX = findViewById(R.id.checkBoxrefSigX);
        final CheckBox checkBoxrefSigY = findViewById(R.id.checkBoxrefSigY);
        final CheckBox checkBoxrefSigZ = findViewById(R.id.checkBoxrefSigZ);
        final CheckBox checkBoxSensorX = findViewById(R.id.checkBoxSensorX);
        final CheckBox checkBoxSensorY = findViewById(R.id.checkBoxSensorY);
        final CheckBox checkBoxSensorZ = findViewById(R.id.checkBoxSensorZ);
        final CheckBox checkBoxPoutX = findViewById(R.id.checkBoxPoutX);
        final CheckBox checkBoxPoutY = findViewById(R.id.checkBoxPoutY);
        final CheckBox checkBoxPoutZ = findViewById(R.id.checkBoxPoutZ);
        final CheckBox checkBoxIoutX = findViewById(R.id.checkBoxIoutX);
        final CheckBox checkBoxIoutY = findViewById(R.id.checkBoxIoutY);
        final CheckBox checkBoxIoutZ = findViewById(R.id.checkBoxIoutZ);
        final CheckBox checkBoxDoutX = findViewById(R.id.checkBoxDoutX);
        final CheckBox checkBoxDoutY = findViewById(R.id.checkBoxDoutY);
        final CheckBox checkBoxDoutZ = findViewById(R.id.checkBoxDoutZ);
        final CheckBox checkBoxFFoutX = findViewById(R.id.checkBoxFFoutX);
        final CheckBox checkBoxFFoutY = findViewById(R.id.checkBoxFFoutY);
        final CheckBox checkBoxFFoutZ = findViewById(R.id.checkBoxFFoutZ);
        final CheckBox checkBoxUX = findViewById(R.id.checkBoxUX);
        final CheckBox checkBoxUY = findViewById(R.id.checkBoxUY);
        final CheckBox checkBoxUZ = findViewById(R.id.checkBoxUZ);

        final Button buttonGetMeasState = findViewById(R.id.buttonGetMeasState);
        final Button buttonResetMeas = findViewById(R.id.buttonResetMeas);
        final Button buttonReinitSDCard = findViewById(R.id.buttonReInitSDCard);
        final Button buttonGlobelTimeSet = findViewById(R.id.buttonGlobalTimeSet);
        final Button buttonGlobelTimeGet = findViewById(R.id.buttonGlobalTimeGet);


        buttonGetMeasState.setOnClickListener(v ->
        {
            final EditText textMeasState = findViewById(R.id.editTextMeasState);
            BTSocket.getInstance().BTGetCommand(1000, textMeasState);
        });
        buttonResetMeas.setOnClickListener(v ->
        {
            final EditText textMeasState = findViewById(R.id.editTextMeasState);
            BTSocket.getInstance().BTGetCommand(1001, textMeasState);
        });
        buttonReinitSDCard.setOnClickListener(v ->
        {
            final EditText textMeasState = findViewById(R.id.editTextMeasState);
            BTSocket.getInstance().BTGetCommand(1002, textMeasState);
        });
        buttonGlobelTimeSet.setOnClickListener(v ->
        {
            final TextView textViewGlobalTime = findViewById(R.id.textViewGlobalTime);
            BTSocket.getInstance().BTSetGlobalTime(1, textViewGlobalTime);
        });
        buttonGlobelTimeGet.setOnClickListener(v ->
        {
            final TextView textViewGlobalTime = findViewById(R.id.textViewGlobalTime);
            BTSocket.getInstance().BTGetGlobalTime(1, textViewGlobalTime);

        });
        checkBoxGrawX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGrawX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(101, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxGrawY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGrawY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(102, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxGrawZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGrawZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(103, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxGpt1X.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGpt1X.isChecked();
            BTSocket.getInstance().BTSetMeasurement(104, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxGpt1Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGpt1Y.isChecked();
            BTSocket.getInstance().BTSetMeasurement(105, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxGpt1Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGpt1Z.isChecked();
            BTSocket.getInstance().BTSetMeasurement(106, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxGpt2X.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGpt2X.isChecked();
            BTSocket.getInstance().BTSetMeasurement(107, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxGpt2Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGpt2Y.isChecked();
            BTSocket.getInstance().BTSetMeasurement(108, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxGpt2Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxGpt2Z.isChecked();
            BTSocket.getInstance().BTSetMeasurement(109, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxArawX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxArawX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(110, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxArawY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxArawY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(111, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxArawZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxArawZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(112, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxApt1X.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxApt1X.isChecked();
            BTSocket.getInstance().BTSetMeasurement(113, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxApt1Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxApt1Y.isChecked();
            BTSocket.getInstance().BTSetMeasurement(114, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxApt1Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxApt1Z.isChecked();
            BTSocket.getInstance().BTSetMeasurement(115, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxApt2X.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxApt2X.isChecked();
            BTSocket.getInstance().BTSetMeasurement(116, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxApt2Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxApt2Y.isChecked();
            BTSocket.getInstance().BTSetMeasurement(117, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxApt2Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxApt2Z.isChecked();
            BTSocket.getInstance().BTSetMeasurement(118, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleRawRoll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleRawRoll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(119, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleRawPitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleRawPitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(120, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAnglePT1Roll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAnglePT1Roll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(121, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAnglePT1Pitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAnglePT1Pitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(122, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAnglePT2Roll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAnglePT2Roll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(123, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAnglePT2Pitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAnglePT2Pitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(124, isChecked);
            setHdrCharCnt(isChecked, 5);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFrawRoll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFrawRoll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(125, isChecked);
            setHdrCharCnt(isChecked, 7);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFrawPitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFrawPitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(126, isChecked);
            setHdrCharCnt(isChecked, 7);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT10Roll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT10Roll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(127, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT10Pitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT10Pitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(128, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT20Roll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT20Roll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(129, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT20Pitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT20Pitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(130, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT11Roll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT11Roll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(131, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT11Pitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT11Pitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(132, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT21Roll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT21Roll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(133, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT21Pitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT21Pitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(134, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT22Roll.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT22Roll.isChecked();
            BTSocket.getInstance().BTSetMeasurement(135, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxAngleKFPT22Pitch.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxAngleKFPT22Pitch.isChecked();
            BTSocket.getInstance().BTSetMeasurement(136, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxrefSigX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxrefSigX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(137, isChecked);
            setHdrCharCnt(isChecked, 7);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxrefSigY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxrefSigY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(138, isChecked);
            setHdrCharCnt(isChecked, 7);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxrefSigZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxrefSigZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(139, isChecked);
            setHdrCharCnt(isChecked, 7);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxSensorX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxSensorX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(140, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxSensorY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxSensorY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(141, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxSensorZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxSensorZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(142, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxPoutX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxPoutX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(143, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxPoutY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxPoutY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(144, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxPoutZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxPoutZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(145, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxIoutX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxIoutX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(146, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxIoutY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxIoutY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(147, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxIoutZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxIoutZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(148, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxDoutX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxDoutX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(149, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxDoutY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxDoutY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(150, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxDoutZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxDoutZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(151, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxFFoutX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxFFoutX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(152, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxFFoutY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxFFoutY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(153, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxFFoutZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxFFoutZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(154, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxUX.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxUX.isChecked();
            BTSocket.getInstance().BTSetMeasurement(155, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxUY.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxUY.isChecked();
            BTSocket.getInstance().BTSetMeasurement(156, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });
        checkBoxUZ.setOnClickListener(v ->
        {
            boolean isChecked = checkBoxUZ.isChecked();
            BTSocket.getInstance().BTSetMeasurement(157, isChecked);
            setHdrCharCnt(isChecked, 8);
            setCharRange(isChecked, 5, 7);
        });

    }

    @Override
    public void onResume()
    {
        super.onResume();
        Log.d("BT", "MeasurementActivity - onResume");
    }

    @Override
    public void onPause()
    {
//        mHandler.removeCallbacks(mTimer1);
        super.onPause();
    }

    @Override
    protected void onSwipeLeft()
    {
        Log.d("BT", "MeasurementActivity - onTouchEvent - swiped left");
//        Intent intent = new Intent(MainActivity.this, MeasurementActivity.class);
//        startActivity(intent);
    }

    @Override
    protected void onSwipeRight()
    {
        Log.d("BT", "MeasurementActivity - onTouchEvent - swiped right");
        // returns to previous activity
        finish();
    }

    private void setCharRange(boolean isChecked, int min, int max)
    {
        new Handler(Looper.getMainLooper()).post(() ->
        {

            int ctrMin = Integer.parseInt(charCtrMin.getText().toString());
            int ctrMax = Integer.parseInt(charCtrMax.getText().toString());

            if (isChecked)
            {
                ctrMin += min;
                ctrMax += max;
            }
            else
            {
                ctrMin -= min;
                ctrMax -= max;
            }

            charCtrMin.setText(String.valueOf(ctrMin));
            charCtrMax.setText(String.valueOf(ctrMax));
        });
    }

    private void setHdrCharCnt(boolean isChecked, int count)
    {
        new Handler(Looper.getMainLooper()).post(() ->
        {
            int hdrCnt = Integer.parseInt(hdrChrCnt.getText().toString());

            if (isChecked)
            {
                hdrCnt += count;
            }
            else
            {
                hdrCnt -= count;
            }

            hdrChrCnt.setText(String.valueOf(hdrCnt));
        });
    }
}
