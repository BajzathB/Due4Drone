package com.example.d4djv3;

import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.util.Map;

public class MotorsActivity extends AppCompatActivity
{
    private final byte cycleRotorUpdate = 10;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_motors);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.motors), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        Log.d("BT", "MotorsAcitivity - onCreate - called");

        CheckBox checkBox_m_fl = findViewById(R.id.checkBox_m_fl);
        CheckBox checkBox_m_fr = findViewById(R.id.checkBox_m_fr);
        CheckBox checkBox_m_rl = findViewById(R.id.checkBox_m_rl);
        CheckBox checkBox_m_rr = findViewById(R.id.checkBox_m_rr);
        CheckBox checkBox_m_all = findViewById(R.id.checkBox_m_all);

        checkBox_m_fl.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_m_fl.isChecked();
            BTSocket.getInstance().BTStreamCommand(3006, isChecked);

            if(!isChecked)
            {
                TextView textView_fl = findViewById(R.id.textView_motor_fl);
                textView_fl.setText(Integer.toString(125));
                ImageView ImageView_motor_fl = (ImageView) findViewById(R.id.image_motor_fl);
                ImageView_motor_fl.setBackgroundColor(Color.rgb(160,160,160));
            }
        });
        checkBox_m_fr.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_m_fr.isChecked();
            BTSocket.getInstance().BTStreamCommand(3007, isChecked);

            if(!isChecked)
            {
                TextView textView_fr = findViewById(R.id.textView_motor_fr);
                textView_fr.setText("125");
                ImageView ImageView_motor_fr = (ImageView) findViewById(R.id.image_motor_fr);
                ImageView_motor_fr.setBackgroundColor(Color.rgb(160,160,160));
            }
        });
        checkBox_m_rl.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_m_rl.isChecked();
            BTSocket.getInstance().BTStreamCommand(3008, isChecked);

            if(!isChecked)
            {
                TextView textView_rl = findViewById(R.id.textView_motor_rl);
                textView_rl.setText(Integer.toString(125));
                ImageView ImageView_motor_rl = (ImageView) findViewById(R.id.image_motor_rl);
                ImageView_motor_rl.setBackgroundColor(Color.rgb(160,160,160));
            }
        });
        checkBox_m_rr.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_m_rr.isChecked();
            BTSocket.getInstance().BTStreamCommand(3009, isChecked);

            if(!isChecked)
            {
                TextView textView_rr = findViewById(R.id.textView_motor_rr);
                textView_rr.setText(Integer.toString(125));
                ImageView ImageView_motor_rr = (ImageView) findViewById(R.id.image_motor_rr);
                ImageView_motor_rr.setBackgroundColor(Color.rgb(160,160,160));
            }
        });

        //set new result text
//        BTSocket.getInstance().setBTcmdResultTextID(findViewById(R.id.textBTcmdResult));
//
//        final CheckBox checkBox_motorFR = findViewById(R.id.checkBox_pidRate_sensor_signal_X);
//        final CheckBox checkBox_motorFL = findViewById(R.id.checkBox_pidRate_sensor_signal_Y);
//        final CheckBox checkBox_motorRR = findViewById(R.id.checkBox_pidRate_sensor_signal_Z);
//        final CheckBox checkBox_motorRL = findViewById(R.id.checkBox_gyro_signal_X);
//        final CheckBox checkBox_motorALL = findViewById(R.id.checkBox_gyro_signal_Y);
//
//        checkBox_motorFR.setOnClickListener(v ->
//        {
//            boolean isChecked = checkBox_motorFR.isChecked();
//            BTSocket.getInstance().BTStreamCommand(3000, isChecked);
//        });
//        checkBox_motorFL.setOnClickListener(v ->
//        {
//            boolean isChecked = checkBox_motorFL.isChecked();
//            BTSocket.getInstance().BTStreamCommand(3001, isChecked);
//        });
//        checkBox_motorRR.setOnClickListener(v ->
//        {
//            boolean isChecked = checkBox_motorRR.isChecked();
//            BTSocket.getInstance().BTStreamCommand(3002, isChecked);
//        });
//        checkBox_motorRL.setOnClickListener(v ->
//        {
//            boolean isChecked = checkBox_motorRL.isChecked();
//            BTSocket.getInstance().BTStreamCommand(3003, isChecked);
//        });
//        checkBox_motorALL.setOnClickListener(v ->
//        {
//            boolean isChecked = checkBox_motorALL.isChecked();
//            BTSocket.getInstance().BTStreamCommand(3004, isChecked);
//        });
    }

    @Override
    public void onResume()
    {
        super.onResume();
        Log.d("BT", "MotorsAcitivity - onResume - called");
        Thread tMotors = new Thread()
        {
            @Override
            public void run() {
            while (true)
            {
                //check if new steam data is sent
                Map<String, Object> BTdata = BtRxThread.getBTData();
                int streamID1 = (int) BTdata.get("streamDataID1");
                int streamID2 = (int) BTdata.get("streamDataID2");
                int streamID3 = (int) BTdata.get("streamDataID3");
                int streamID4 = (int) BTdata.get("streamDataID4");
                String streamDataM1 = null;
                String streamDataM2 = null;
                String streamDataM3 = null;
                String streamDataM4 = null;

                if(streamID1 == 3006)
                {
                    streamDataM1 = (String) BTdata.get("streamData1");
                    Log.d("BT", "MotorsAcitivity - onResume - streamDataM1 " + streamDataM1);
                }
                if(streamID2 == 3007)
                {
                    streamDataM2 = (String) BTdata.get("streamData2");
                }
                if(streamID3 == 3008)
                {
                    streamDataM3 = (String) BTdata.get("streamData3");
                }
                if(streamID4 == 3009)
                {
                    streamDataM4 = (String) BTdata.get("streamData4");
                }

                String finalStreamDataMFL = streamDataM1;
                String finalStreamDataMFR = streamDataM2;
                String finalStreamDataMRL = streamDataM3;
                String finalStreamDataMRR = streamDataM4;

                // update graphview
                Handler streamHandler = new Handler(Looper.getMainLooper());
                streamHandler.post(new Runnable()
                {
                    @Override
                    public void run()
                    {
                        if(finalStreamDataMFL != null)
                        {
                            Log.d("BT", "MotorsAcitivity - onResume - finalStreamDataMFL " + finalStreamDataMFL);
                            //set value
                            TextView textView_fl = findViewById(R.id.textView_motor_fl);
                            textView_fl.setText(finalStreamDataMFL);
                            //set color
                            ImageView ImageView_motor_fl = (ImageView) findViewById(R.id.image_motor_fl);
                            int val = interpolateRGB((int)Float.parseFloat(finalStreamDataMFL));
                            Log.d("BT", "MotorsAcitivity - onCreate - interpol: " + val);
//                            ImageView_motor_fl.setBackgroundColor(Color.rgb(0,val,0));
                        }
                        if(finalStreamDataMFR != null)
                        {
                            Log.d("BT", "MotorsAcitivity - onResume - finalStreamDataMFR" + finalStreamDataMFR);
                            //set value
                            TextView textView_fr = findViewById(R.id.textView_motor_fr);
                            textView_fr.setText(finalStreamDataMFR);
                            //set color
//                            int val = interpolateRGB((int)Float.parseFloat(finalStreamDataMFR));
//                            ImageView ImageView_motor_fr = (ImageView) findViewById(R.id.image_motor_fr);
//                            Log.d("BT", "MotorsAcitivity - onCreate - interpol: " + val);
//                            ImageView_motor_fr.setBackgroundColor(Color.rgb(0,interpolateRGB(mFR),0));
                        }
                        if(finalStreamDataMRL != null)
                        {
                            Log.d("BT", "MotorsAcitivity - onResume - finalStreamDataMRL" + finalStreamDataMRL);
                            //set value
                            TextView textView_rl = findViewById(R.id.textView_motor_rl);
                            textView_rl.setText(finalStreamDataMRL);
                            //set color
//                            int val = interpolateRGB((int)Float.parseFloat(finalStreamDataMRL));
//                            ImageView ImageView_motor_rl = (ImageView) findViewById(R.id.image_motor_rl);
//                            Log.d("BT", "MotorsAcitivity - onCreate - interpol: " + val);
//                            ImageView_motor_rl.setBackgroundColor(Color.rgb(0,val,0));
                        }
                        if(finalStreamDataMRR != null)
                        {
                            Log.d("BT", "MotorsAcitivity - onResume - finalStreamDataMRR" + finalStreamDataMRR);
                            //set value
                            TextView textView_rr = findViewById(R.id.textView_motor_rr);
                            textView_rr.setText(finalStreamDataMRR);
                            //set color
//                            int val = interpolateRGB(mRR);
//                            ImageView ImageView_motor_rr = (ImageView) findViewById(R.id.image_motor_rr);
//                            Log.d("BT", "MotorsAcitivity - onCreate - interpol: " + val);
//                            ImageView_motor_rr.setBackgroundColor(Color.rgb(0,interpolateRGB(Integer.parseInt(finalStreamData)),0));
                        }
                    }
                });


                // delay of thread
                try
                {
                    Thread.sleep(cycleRotorUpdate);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }

            }
            }
        };
        tMotors.start();

    }

    @Override
    public void onPause()
    {
        super.onPause();
    }

    // motor value between 125-250
    // output color between 0-255
    private int interpolateRGB(int motorVal)
    {
        float proportion = (float) (motorVal - 125) /125;
        return (int)(255 * proportion);
    }
}
