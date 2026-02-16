package com.example.d4djv3;

import static java.lang.Float.parseFloat;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.LegendRenderer;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import java.util.Map;

public class StreamActivity extends BaseActivity
{
//    private Handler mHandler = new Handler();
//    private Runnable mTimer1;

    GraphView graph;
    int pointsPlotted = 1;
    LineGraphSeries<DataPoint> series1 = new LineGraphSeries<>();
    LineGraphSeries<DataPoint> series2 = new LineGraphSeries<>();
    LineGraphSeries<DataPoint> series3 = new LineGraphSeries<>();
    LineGraphSeries<DataPoint> series4 = new LineGraphSeries<>();
    LineGraphSeries<DataPoint> series5 = new LineGraphSeries<>();
    LineGraphSeries<DataPoint> verticalLineSeries = new LineGraphSeries<>();;

    private final byte cycleGraphUpdate = 10;
    private final int absoluteMaxPlottedPoints = 1000;
    private int maxPlottedPoints = 200;

    DataPoint[] datas1 = new DataPoint[absoluteMaxPlottedPoints];
    DataPoint[] datas2 = new DataPoint[absoluteMaxPlottedPoints];
    DataPoint[] datas3 = new DataPoint[absoluteMaxPlottedPoints];
    DataPoint[] datas4 = new DataPoint[absoluteMaxPlottedPoints];
    DataPoint[] datas5 = new DataPoint[absoluteMaxPlottedPoints];

    Spinner gyroFilterParams;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_stream);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.stream), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        Log.d("BT", "StreamActivity - onCreate - called");

        resetDatas();

        //set new result text
        BTSocket.getInstance().setBTcmdResultTextID(findViewById(R.id.textBTcmdResult));

        graph = (GraphView) findViewById(R.id.graph);
        series2.setColor(Color.GREEN);
        series3.setColor(Color.CYAN);
        series4.setColor(Color.GRAY);
        series5.setColor(Color.YELLOW);
        graph.addSeries(series1);
        graph.addSeries(series2);
        graph.addSeries(series3);
        graph.addSeries(series4);
        graph.addSeries(series5);

        graph.addSeries(verticalLineSeries);
        verticalLineSeries.setColor(Color.GRAY);
        verticalLineSeries.setThickness(2);

        //graph.getViewport().setScrollable(false);
        graph.getViewport().setXAxisBoundsManual(true);
        graph.getViewport().setMinX(0);
        graph.getViewport().setMaxX(maxPlottedPoints);
        graph.getViewport().setScrollableY(true);
        graph.getViewport().setYAxisBoundsManual(true);
        graph.getViewport().setMaxY(100);
        graph.getViewport().setMinY(-100);

        //get the spinner from the xml.
        gyroFilterParams = findViewById(R.id.spinner_accFilterParams);
        //create a list of items for the spinner.
        String[] items = new String[]{"PT_c", "KF_q", "KF_r"};
        //create an adapter to describe how the items are displayed, adapters are used in several places in android.
        //There are multiple variations of this, but this is the basic variant.
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, items);
        //set the spinners adapter to the previously created one.
        gyroFilterParams.setAdapter(adapter);

        final CheckBox checkBox_gyro_X = findViewById(R.id.checkBox_gyro_raw_X);
        final CheckBox checkBox_gyro_Y = findViewById(R.id.checkBox_gyro_raw_Y);
        final CheckBox checkBox_gyro_Z = findViewById(R.id.checkBox_gyro_raw_Z);
        final CheckBox checkBox_gyro_pt1_X = findViewById(R.id.checkBox_gyro_pt1_X);
        final CheckBox checkBox_gyro_pt1_Y = findViewById(R.id.checkBox_gyro_pt1_Y);
        final CheckBox checkBox_gyro_pt1_Z = findViewById(R.id.checkBox_gyro_pt1_Z);
        final CheckBox checkBox_gyro_pt2_X = findViewById(R.id.checkBox_gyro_pt2_X);
        final CheckBox checkBox_gyro_pt2_Y = findViewById(R.id.checkBox_gyro_pt2_Y);
        final CheckBox checkBox_gyro_pt2_Z = findViewById(R.id.checkBox_gyro_pt2_Z);
        final CheckBox checkBox_gyro_kf_X = findViewById(R.id.checkBox_gyro_kf_X);
        final CheckBox checkBox_gyro_kf_Y = findViewById(R.id.checkBox_gyro_kf_Y);
        final CheckBox checkBox_gyro_kf_Z = findViewById(R.id.checkBox_gyro_kf_Z);

        final Button buttonGraphPlus = findViewById(R.id.buttonGraphPlus);
        final Button buttonGraphMinus = findViewById(R.id.buttonGraphMinus);
        final Button buttonGyroFilterGet = findViewById(R.id.buttonGyroFilterGet);
        final Button buttonGyroFilterSet = findViewById(R.id.buttonGyroFilterSet);
        final Button buttonGyroFilterDec = findViewById(R.id.buttonGyroFilterDec);
        final Button buttonGyroFilterInc = findViewById(R.id.buttonGyroFilterInc);
        final Button buttonGraphXPlus = findViewById(R.id.buttonGraphXPlus);
        final Button buttonGraphXMinus = findViewById(R.id.buttonGraphXMinus);

        checkBox_gyro_X.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_X.isChecked();
            BTSocket.getInstance().BTStreamCommand(3003, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_Y.isChecked();
            BTSocket.getInstance().BTStreamCommand(3004, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_Z.isChecked();
            BTSocket.getInstance().BTStreamCommand(3005, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_pt1_X.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_pt1_X.isChecked();
            BTSocket.getInstance().BTStreamCommand(3010, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_pt1_Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_pt1_Y.isChecked();
            BTSocket.getInstance().BTStreamCommand(3011, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_pt1_Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_pt1_Z.isChecked();
            BTSocket.getInstance().BTStreamCommand(3012, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_pt2_X.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_pt2_X.isChecked();
            BTSocket.getInstance().BTStreamCommand(3013, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_pt2_Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_pt2_Y.isChecked();
            BTSocket.getInstance().BTStreamCommand(3014, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_pt2_Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_pt2_Z.isChecked();
            BTSocket.getInstance().BTStreamCommand(3015, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_kf_X.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_kf_X.isChecked();
            BTSocket.getInstance().BTStreamCommand(3016, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_kf_Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_kf_Y.isChecked();
            BTSocket.getInstance().BTStreamCommand(3017, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_gyro_kf_Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_gyro_kf_Z.isChecked();
            BTSocket.getInstance().BTStreamCommand(3018, isChecked);
            if (!isChecked) resetDatas();
        });

        gyroFilterParams.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
            {
                EditText textGyroFilterValue = findViewById(R.id.editTextText_gyroFilterValue);
                int selectedParamValID = getGyroFilterValueID(gyroFilterParams.getSelectedItem().toString());
                BTSocket.getInstance().BTGetCommand(selectedParamValID,textGyroFilterValue);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        buttonGyroFilterGet.setOnClickListener(v ->
        {
            EditText textGyroFilterValue = findViewById(R.id.editTextText_gyroFilterValue);
            int selectedParamValID = getGyroFilterValueID(gyroFilterParams.getSelectedItem().toString());
            BTSocket.getInstance().BTGetCommand(selectedParamValID,textGyroFilterValue);
        });

        buttonGyroFilterSet.setOnClickListener(v ->
        {
            EditText textGyroParamC = findViewById(R.id.editTextText_gyroFilterValue);
            int selectedParamValID = getGyroFilterValueID(gyroFilterParams.getSelectedItem().toString());
            BTSocket.getInstance().BTSetCommand(selectedParamValID, textGyroParamC);
        });

        buttonGyroFilterDec.setOnClickListener(v ->
        {
            EditText textGyroParamC = findViewById(R.id.editTextText_gyroFilterValue);
            float val2Add = -1*getFraction2Add(gyroFilterParams.getSelectedItem().toString());
            BTSocket.getInstance().AddVal(textGyroParamC, val2Add);
        });

        buttonGyroFilterInc.setOnClickListener(v ->
        {
            EditText textGyroParamC = findViewById(R.id.editTextText_gyroFilterValue);
            float val2Add = getFraction2Add(gyroFilterParams.getSelectedItem().toString());
            BTSocket.getInstance().AddVal(textGyroParamC, val2Add);
        });

        buttonGraphPlus.setOnClickListener(v ->
        {
            Log.d("BT", "StreamActivity - buttonGraphPlus called");
            double maxY = graph.getViewport().getMaxY(false);
            double minY = graph.getViewport().getMinY(false);
            if(maxY+100 <= 2000)
            {
                graph.getViewport().setMaxY(maxY+100);
                graph.getViewport().setMinY(minY-100);
            }
        });

        buttonGraphMinus.setOnClickListener(v ->
        {
            Log.d("BT", "StreamActivity - buttonGraphMinus called");
            double maxY = graph.getViewport().getMaxY(false);
            double minY = graph.getViewport().getMinY(false);
            if(maxY-100 > 100)
            {
                graph.getViewport().setMaxY(maxY-100);
                graph.getViewport().setMinY(minY+100);
            }
        });

        buttonGraphXPlus.setOnClickListener(v ->
        {
            Log.d("BT", "StreamActivity - buttonGraphXPlus called");
            double maxX = graph.getViewport().getMaxX(false);
            if(maxX+50 <= absoluteMaxPlottedPoints)
            {
                graph.getViewport().setMaxX(maxX+50);
                maxPlottedPoints = (int)maxX+50;
            }
        });

        buttonGraphXMinus.setOnClickListener(v ->
        {
            Log.d("BT", "StreamActivity - buttonGraphXMinus called");
            double maxX = graph.getViewport().getMaxX(false);
            if(0 < maxX-50)
            {
                graph.getViewport().setMaxX(maxX-50);
                maxPlottedPoints = (int)maxX-50;
            }
        });
    }

    @Override
    public void onResume()
    {
        super.onResume();

        Thread tGraph = new Thread()
        {
            @Override
            public void run()
            {
                while (true)
                {
//                    Log.d("BT", "StreamActivity - onResume - thread running");
                    //check if new steam data is sent
                    Map<String, Object> BTdata = BtRxThread.getBTData();
                    int streamID1 = (int)BTdata.get("streamDataID1");
                    int streamID2 = (int)BTdata.get("streamDataID2");
                    int streamID3 = (int)BTdata.get("streamDataID3");
                    int streamID4 = (int)BTdata.get("streamDataID4");
                    int streamID5 = (int)BTdata.get("streamDataID5");
                    String streamData1 = null;
                    String streamData2 = null;
                    String streamData3 = null;
                    String streamData4 = null;
                    String streamData5 = null;

                    if(streamID1 > 0)
                    {
                        streamData1 = (String) BTdata.get("streamData1");
                        if (streamData1 != null)
                        {
                            datas1[pointsPlotted] = new DataPoint(pointsPlotted, parseFloat(streamData1));
//                            Log.d("BT", "StreamActivity - onResume - streamData1: " + streamData1);
                        }
                    }
                    if(streamID2 > 0)
                    {
                        streamData2 = (String) BTdata.get("streamData2");
                        if (streamData2 != null)
                        {
                            datas2[pointsPlotted] = new DataPoint(pointsPlotted, parseFloat(streamData2));
                        }
                    }
                    if(streamID3 > 0)
                    {
                        streamData3 = (String) BTdata.get("streamData3");
                        if (streamData3 != null)
                        {
                            datas3[pointsPlotted] = new DataPoint(pointsPlotted, parseFloat(streamData3));
                        }
                    }
                    if(streamID4 > 0)
                    {
                        streamData4 = (String) BTdata.get("streamData4");
                        if (streamData4 != null)
                        {
                            datas4[pointsPlotted] = new DataPoint(pointsPlotted, parseFloat(streamData4));
                        }
                    }
                    if(streamID5 > 0)
                    {
                        streamData5 = (String) BTdata.get("streamData5");
                        if (streamData5 != null)
                        {
                            datas5[pointsPlotted] = new DataPoint(pointsPlotted, parseFloat(streamData5));
                        }
                    }

                    String finalStreamData1 = streamData1;
                    String finalStreamData2 = streamData2;
                    String finalStreamData3 = streamData3;
                    String finalStreamData4 = streamData4;
                    String finalStreamData5 = streamData5;

                    // update graphview
                    Handler streamHandler = new Handler(Looper.getMainLooper());
                    streamHandler.post(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            if(finalStreamData1 != null)
                            {
                                series1.resetData(datas1);
                                series1.setTitle(findNameById(streamID1));
//                                Log.d("BT", "StreamActivity - onResume - finalStreamData1: " + finalStreamData1);

                                DataPoint[] linePoints = new DataPoint[] {
                                        new DataPoint(pointsPlotted, graph.getViewport().getMinY(false)),
                                        new DataPoint(pointsPlotted, graph.getViewport().getMaxY(false))
                                };
                                verticalLineSeries.resetData(linePoints);
                            }
                            if(finalStreamData2 != null)
                            {
                                series2.resetData(datas2);
                                series2.setTitle(findNameById(streamID2));
                            }
                            if(finalStreamData3 != null)
                            {
                                series3.resetData(datas3);
                                series3.setTitle(findNameById(streamID3));
                            }
                            if(finalStreamData4 != null)
                            {
                                series4.resetData(datas4);
                                series4.setTitle(findNameById(streamID4));
                            }
                            if(finalStreamData5 != null)
                            {
                                series5.resetData(datas5);
                                series5.setTitle(findNameById(streamID5));
                            }
                        }
                    });

                    if(streamID1 > 0)
                    {
                        pointsPlotted++;
                        //Log.d("BT", "StreamActivity - pointsPlotted: " + pointsPlotted);
                        if(pointsPlotted >= maxPlottedPoints)
                        {
                            //Log.d("BT", "StreamActivity - resetting pointsPlotted");
                            pointsPlotted = 1;
                            //Log.d("BT", "StreamActivity - resetData");
                        }

                        graph.getLegendRenderer().setVisible(true);
                        graph.getLegendRenderer().setAlign(LegendRenderer.LegendAlign.TOP);
                    }

                    // delay of thread
                    try
                    {
                        Thread.sleep(cycleGraphUpdate);
                    } catch (InterruptedException e) {
                        throw new RuntimeException(e);
                    }
                }
            }
        };
        tGraph.start();
    }

    @Override
    public void onPause()
    {
//        mHandler.removeCallbacks(mTimer1);
        super.onPause();
    }

    String findNameById(int Id)
    {
        String name="-";

        switch (Id)
        {
            case 3000: name = "PidRateX"; break;
            case 3001: name = "PidRateY"; break;
            case 3002: name = "PidRateZ"; break;
            case 3003: name = "GyroRawX"; break;
            case 3004: name = "GyroRawY"; break;
            case 3005: name = "GyroRawZ"; break;
            case 3010: name = "GyroPT1X"; break;
            case 3011: name = "GyroPT1Y"; break;
            case 3012: name = "GyroPT1Z"; break;
            case 3013: name = "GyroPT2X"; break;
            case 3014: name = "GyroPT2Y"; break;
            case 3015: name = "GyroPT2Z"; break;
            case 3016: name = "GyroKfX"; break;
            case 3017: name = "GyroKfY"; break;
            case 3018: name = "GyroKfZ"; break;

            default: name = "???"; break;

        }

        return name;
    }

    void resetDatas()
    {
        for(int itr=0; itr < 1000; itr++)
        {
            datas1[itr] = new DataPoint(itr, 0);
            datas2[itr] = new DataPoint(itr, 0);
            datas3[itr] = new DataPoint(itr, 0);
            datas4[itr] = new DataPoint(itr, 0);
            datas5[itr] = new DataPoint(itr, 0);
        }
    }

    int getGyroFilterValueID(String selectedFilterValue)
    {
        int returnVal = 0;

        switch (selectedFilterValue)
        {
            case "PT_c":
            {
                returnVal = 2012;
                break;
            }
            case "KF_q":
            {
                returnVal = 2013;
                break;
            }
            case "KF_r":
            {
                returnVal = 2014;
                break;
            }
        }

        return returnVal;
    }

    float getFraction2Add(String selectedFilterValue)
    {
        float returnVal = 0;

        switch (selectedFilterValue)
        {
            case "PT_c":
            {
                returnVal = 0.5f;
                break;
            }
            case "KF_q":
            {
                returnVal = 0.001f;
                break;
            }
            case "KF_r":
            {
                returnVal = 0.5f;
                break;
            }
        }

        return returnVal;
    }

    @Override
    protected void onSwipeLeft()
    {
        Log.d("BT", "StreamActivty - onTouchEvent - swiped left");
        // returns to previous activity
        finish();
    }

    @Override
    protected void onSwipeRight()
    {
//        Log.d("BT", "StreamActivty - onTouchEvent - swiped right");
//        Intent intent = new Intent(StreamActivity.this, StreamAccActivity.class);
//        startActivity(intent);
    }
}