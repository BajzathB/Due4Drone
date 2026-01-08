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

public class StreamAccActivity extends BaseActivity
{

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

    Spinner accFilterParams;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_stream_acc);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.stream_acc), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        Log.d("BT", "StreamAccActivity - onCreate - called");

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
        graph.getViewport().setMaxY(5);
        graph.getViewport().setMinY(-5);

        //get the spinner from the xml.
        accFilterParams = findViewById(R.id.spinner_accFilterParams);
        //create a list of items for the spinner.
        String[] items = new String[]{"PT_c", "KF_q_ang", "KF_q_bias", "KF_r", "offsetX", "offsetY", "offsetZ"};
        //create an adapter to describe how the items are displayed, adapters are used in several places in android.
        //There are multiple variations of this, but this is the basic variant.
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_dropdown_item, items);
        //set the spinners adapter to the previously created one.
        accFilterParams.setAdapter(adapter);

        final CheckBox checkBox_acc_X = findViewById(R.id.checkBox_acc_raw_x);
        final CheckBox checkBox_acc_Y = findViewById(R.id.checkBox_acc_raw_y);
        final CheckBox checkBox_acc_Z = findViewById(R.id.checkBox_acc_raw_z);
        final CheckBox checkBox_acc_pt1_X = findViewById(R.id.checkBox_acc_pt1_x);
        final CheckBox checkBox_acc_pt1_Y = findViewById(R.id.checkBox_acc_pt1_y);
        final CheckBox checkBox_acc_pt1_Z = findViewById(R.id.checkBox_acc_pt1_z);
        final CheckBox checkBox_acc_pt2_X = findViewById(R.id.checkBox_acc_pt2_x);
        final CheckBox checkBox_acc_pt2_Y = findViewById(R.id.checkBox_acc_pt2_y);
        final CheckBox checkBox_acc_pt2_Z = findViewById(R.id.checkBox_acc_pt2_z);
        final CheckBox checkBox_acc_raw_r = findViewById(R.id.checkBox_acc_raw_r);
        final CheckBox checkBox_acc_raw_p = findViewById(R.id.checkBox_acc_raw_p);
        final CheckBox checkBox_acc_PT1_r = findViewById(R.id.checkBox_acc_PT1_r);
        final CheckBox checkBox_acc_PT1_p = findViewById(R.id.checkBox_acc_PT1_p);
        final CheckBox checkBox_acc_PT2_r = findViewById(R.id.checkBox_acc_PT2_r);
        final CheckBox checkBox_acc_PT2_p = findViewById(R.id.checkBox_acc_PT2_p);
        final CheckBox checkBox_acc_kf_p = findViewById(R.id.checkBox_acc_kf_p);
        final CheckBox checkBox_acc_kf_r = findViewById(R.id.checkBox_acc_kf_r);
        final CheckBox checkBox_acc_kfA1_r = findViewById(R.id.checkBox_acc_kfA1_r);
        final CheckBox checkBox_acc_kfA1_p = findViewById(R.id.checkBox_acc_kfA1_p);
        final CheckBox checkBox_acc_kfA2_r = findViewById(R.id.checkBox_acc_kfA2_r);
        final CheckBox checkBox_acc_kfA2_p = findViewById(R.id.checkBox_acc_kfA2_p);
        final CheckBox checkBox_acc_kfA1G1_r = findViewById(R.id.checkBox_acc_kfA1G1_r);
        final CheckBox checkBox_acc_kfA1G1_p = findViewById(R.id.checkBox_acc_kfA1G1_p);
        final CheckBox checkBox_acc_kfA2G2_r = findViewById(R.id.checkBox_acc_kfA2G2_r);
        final CheckBox checkBox_acc_kfA2G2_p = findViewById(R.id.checkBox_acc_kfA2G2_p);
        final CheckBox checkbox_dt = findViewById(R.id.checkBox_dt);

        final Button buttonGraphPlus = findViewById(R.id.buttonGraphPlus);
        final Button buttonGraphMinus = findViewById(R.id.buttonGraphMinus);
        final Button buttonAccFilterGet = findViewById(R.id.buttonAccFilterGet);
        final Button buttonAccFilterSet = findViewById(R.id.buttonAccFilterSet);
        final Button buttonAccFilterDec = findViewById(R.id.buttonAccFilterDec);
        final Button buttonAccFilterInc = findViewById(R.id.buttonAccFilterInc);
        final Button buttonGraphXPlus = findViewById(R.id.buttonGraphXPlus);
        final Button buttonGraphXMinus = findViewById(R.id.buttonGraphXMinus);

        checkBox_acc_X.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_X.isChecked();
            BTSocket.getInstance().BTStreamCommand(3019, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_Y.isChecked();
            BTSocket.getInstance().BTStreamCommand(3020, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_Z.isChecked();
            BTSocket.getInstance().BTStreamCommand(3021, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_pt1_X.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_pt1_X.isChecked();
            BTSocket.getInstance().BTStreamCommand(3022, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_pt1_Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_pt1_Y.isChecked();
            BTSocket.getInstance().BTStreamCommand(3023, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_pt1_Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_pt1_Z.isChecked();
            BTSocket.getInstance().BTStreamCommand(3024, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_pt2_X.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_pt2_X.isChecked();
            BTSocket.getInstance().BTStreamCommand(3025, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_pt2_Y.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_pt2_Y.isChecked();
            BTSocket.getInstance().BTStreamCommand(3026, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_pt2_Z.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_pt2_Z.isChecked();
            BTSocket.getInstance().BTStreamCommand(3027, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_raw_r.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_raw_r.isChecked();
            BTSocket.getInstance().BTStreamCommand(3028, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_raw_p.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_raw_p.isChecked();
            BTSocket.getInstance().BTStreamCommand(3029, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_PT1_r.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_PT1_r.isChecked();
            BTSocket.getInstance().BTStreamCommand(3030, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_PT1_p.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_PT1_p.isChecked();
            BTSocket.getInstance().BTStreamCommand(3031, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_PT2_r.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_PT2_r.isChecked();
            BTSocket.getInstance().BTStreamCommand(3032, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_PT2_p.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_PT2_p.isChecked();
            BTSocket.getInstance().BTStreamCommand(3033, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kf_r.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kf_r.isChecked();
            BTSocket.getInstance().BTStreamCommand(3034, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kf_p.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kf_p.isChecked();
            BTSocket.getInstance().BTStreamCommand(3035, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kfA1_r.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kfA1_r.isChecked();
            BTSocket.getInstance().BTStreamCommand(3036, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kfA1_p.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kfA1_p.isChecked();
            BTSocket.getInstance().BTStreamCommand(3037, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kfA2_r.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kfA2_r.isChecked();
            BTSocket.getInstance().BTStreamCommand(3038, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kfA2_p.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kfA2_p.isChecked();
            BTSocket.getInstance().BTStreamCommand(3039, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kfA1G1_r.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kfA1G1_r.isChecked();
            BTSocket.getInstance().BTStreamCommand(3040, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kfA1G1_p.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kfA1G1_p.isChecked();
            BTSocket.getInstance().BTStreamCommand(3041, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kfA2G2_r.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kfA2G2_r.isChecked();
            BTSocket.getInstance().BTStreamCommand(3042, isChecked);
            if (!isChecked) resetDatas();
        });
        checkBox_acc_kfA2G2_p.setOnClickListener(v ->
        {
            boolean isChecked = checkBox_acc_kfA2G2_p.isChecked();
            BTSocket.getInstance().BTStreamCommand(3043, isChecked);
            if (!isChecked) resetDatas();
        });
        checkbox_dt.setOnClickListener(v ->
        {
            boolean isChecked = checkbox_dt.isChecked();
            BTSocket.getInstance().BTStreamCommand(3044, isChecked);
            if (!isChecked) resetDatas();
        });

        accFilterParams.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id)
            {
                EditText textGyroFilterValue = findViewById(R.id.editTextText_accFilterValue);
                int selectedParamValID = getAccFilterValueID(accFilterParams.getSelectedItem().toString());
                BTSocket.getInstance().BTGetCommand(selectedParamValID,textGyroFilterValue);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        buttonAccFilterGet.setOnClickListener(v ->
        {
            EditText textAccFilterValue = findViewById(R.id.editTextText_accFilterValue);
            int selectedParamValID = getAccFilterValueID(accFilterParams.getSelectedItem().toString());
            BTSocket.getInstance().BTGetCommand(selectedParamValID,textAccFilterValue);
        });

        buttonAccFilterSet.setOnClickListener(v ->
        {
            EditText textAccFilterValue = findViewById(R.id.editTextText_accFilterValue);
            int selectedParamValID = getAccFilterValueID(accFilterParams.getSelectedItem().toString());
            BTSocket.getInstance().BTSetCommand(selectedParamValID, textAccFilterValue);
        });

        buttonAccFilterDec.setOnClickListener(v ->
        {
            EditText textAccFilterValue = findViewById(R.id.editTextText_accFilterValue);
            float val2Add = -1*getFraction2Add(accFilterParams.getSelectedItem().toString());
            BTSocket.getInstance().AddVal(textAccFilterValue, val2Add);
        });

        buttonAccFilterInc.setOnClickListener(v ->
        {
            EditText textAccFilterValue = findViewById(R.id.editTextText_accFilterValue);
            float val2Add = getFraction2Add(accFilterParams.getSelectedItem().toString());
            BTSocket.getInstance().AddVal(textAccFilterValue, val2Add);
        });

        buttonGraphPlus.setOnClickListener(v ->
        {
            double maxY = graph.getViewport().getMaxY(false);
            double minY = graph.getViewport().getMinY(false);
            if(maxY+2 <= 23)
            {
                graph.getViewport().setMaxY(maxY+2);
                graph.getViewport().setMinY(minY-2);
            }
        });

        buttonGraphMinus.setOnClickListener(v ->
        {
            double maxY = graph.getViewport().getMaxY(false);
            double minY = graph.getViewport().getMinY(false);
            if(maxY-2 > 0)
            {
                graph.getViewport().setMaxY(maxY-2);
                graph.getViewport().setMinY(minY+2);
            }
        });

        buttonGraphXPlus.setOnClickListener(v ->
        {
            double maxX = graph.getViewport().getMaxX(false);
            if(maxX+50 <= absoluteMaxPlottedPoints)
            {
                graph.getViewport().setMaxX(maxX+50);
                maxPlottedPoints = (int)maxX+50;
            }
        });

        buttonGraphXMinus.setOnClickListener(v ->
        {
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
        super.onPause();

    }

    String findNameById(int Id)
    {
        String name="-";

        switch (Id)
        {
            case 3019: name = "AccRawX"; break;
            case 3020: name = "AccRawY"; break;
            case 3021: name = "AccRawZ"; break;
            case 3022: name = "AccPT1X"; break;
            case 3023: name = "AccPT1Y"; break;
            case 3024: name = "AccPT1Z"; break;
            case 3025: name = "AccPT2X"; break;
            case 3026: name = "AccPT2Y"; break;
            case 3027: name = "AccPT2Z"; break;
            case 3028: name = "Roll"; break;
            case 3029: name = "Pitch"; break;
            case 3030: name = "RollPT1"; break;
            case 3031: name = "PitchPT1"; break;
            case 3032: name = "RollPT2"; break;
            case 3033: name = "PitchPT2"; break;
            case 3034: name = "RollKf"; break;
            case 3035: name = "PitchKf"; break;
            case 3036: name = "RollKfA1"; break;
            case 3037: name = "PitchKfA1"; break;
            case 3038: name = "RollKfA2"; break;
            case 3039: name = "PitchKfA2"; break;
            case 3040: name = "RollKfA1G1"; break;
            case 3041: name = "PitchKfA1G1"; break;
            case 3042: name = "RollKfA2G2"; break;
            case 3043: name = "PitchKfA2G2"; break;
            case 3044: name = "Dt"; break;

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

    int getAccFilterValueID(String selectedFilterValue)
    {
        int returnVal = 0;

        switch (selectedFilterValue)
        {
            case "PT_c":
            {
                returnVal = 2015;
                break;
            }
            case "KF_q_ang":
            {
                returnVal = 2016;
                break;
            }
            case "KF_q_bias":
            {
                returnVal = 2017;
                break;
            }
            case "KF_r":
            {
                returnVal = 2018;
                break;
            }
            case "offsetX":
            {
                returnVal = 2019;
                break;
            }
            case "offsetY":
            {
                returnVal = 2020;
                break;
            }
            case "offsetZ":
            {
                returnVal = 2021;
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
            case "KF_q_ang":
            case "KF_q_bias":
            {
                returnVal = 0.000001f;
                break;
            }
            case "KF_r":
            {
                returnVal = 1.0f;
                break;
            }
            case "offsetX":
            case "offsetY":
            case "offsetZ":
            {
                returnVal = 0.01f;
                break;
            }
        }

        return returnVal;
    }

    @Override
    protected void onSwipeLeft()
    {
        Log.d("BT", "MainActivty - onTouchEvent - swiped left");
        // returns to previous activity
        finish();
    }

    @Override
    protected void onSwipeRight()
    {
        Log.d("BT", "StreamAccActivity - onTouchEvent - swiped right");
        Intent intent = new Intent(StreamAccActivity.this, MotorsActivity.class);
        startActivity(intent);
    }
}
