package com.example.d4djv3;

import static java.lang.Float.parseFloat;

import android.bluetooth.BluetoothSocket;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.graphics.Color;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.Map;
import java.util.Calendar;
import java.util.Date;

public class BTSocket {
    private static BTSocket instance;
    private BluetoothSocket bluetoothSocket;
    private boolean BTConnected = false;
    private TextView cmdResultTextId;
    private Button cmdLamp;
    private volatile boolean isGetCmdFinished = false;
    private volatile boolean isSetCmdFinished;

    private final byte cycleDelay = 10;
    private final int responseDelay = 1500;


    private BTSocket() {
    }

    public static BTSocket getInstance() {
        if (instance == null) {
            instance = new BTSocket();
        }
        return instance;
    }

    public void setBluetoothSocket(BluetoothSocket socket) {
        this.bluetoothSocket = socket;
    }

    public void setBluetoothConnection(boolean flag) {
        this.BTConnected = flag;
    }

    public void setBTcmdResultTextID(TextView cmdResultTextId) {
        this.cmdResultTextId = cmdResultTextId;
    }

    public void setBTCmdLamp(Button cmdLamp) {
        this.cmdLamp = cmdLamp;
    }

    public boolean getBluetoothConnection() {
        return BTConnected;
    }

    public boolean getIsGetCmdFinished() {
        return isGetCmdFinished;
    }

    public boolean getIsSetCmdFinished() {
        return isSetCmdFinished;
    }

    void BTGetCommand(int id, EditText textOut) {
        if (BTConnected && id != 0 && textOut != null) {
            byte[] buffer = new byte[20];
            byte ctr = 0;

            //set number of data to zero first
            buffer[ctr++] = 0;

            //set get cmd
            buffer[ctr++] = 0x0C;

            //set id
            buffer[ctr++] = (byte) (id & 0xff);
            buffer[ctr++] = (byte) ((id & 0xff00) >> 8);

            //set CR-NL ending
            buffer[ctr++] = (byte) '\r';
            buffer[ctr++] = (byte) '\n';

            //number of data to send
            buffer[0] = (byte) (ctr - 1);

            try {
                Log.d("BT", "BTSocket - trying GET");
                bluetoothSocket.getOutputStream().write(buffer, 0, ctr);
            } catch (IOException e) {
                Log.d("BT", "BTSocket - buttonGet press - failed to write");
            }

            Handler getHandler = new Handler(Looper.getMainLooper());
            getHandler.post(() ->
            {
                cmdResultTextId.setText("Ongoing...");
                cmdLamp.setBackgroundColor(Color.YELLOW);
            });

            isGetCmdFinished = false;

            Thread tGet = new Thread() {
                boolean newParamReceived = false;
                boolean getFinished = false;
                String paramData = "";

                //wait for reply
                boolean isTimedOut = false;
                final long startTime = System.currentTimeMillis();

                @Override
                public void run() {
                    while (true) {
                        //get data from Rx thread, break while if new param received
                        Map<String, Object> BTdata = BtRxThread.getBTData();
                        newParamReceived = (boolean) BTdata.get("newParamReceived");
                        //Log.d("BT", "MainActivty - buttonGetPx press - checking response receive: " + newParamReceived);
                        if (newParamReceived) {
                            paramData = (String) BTdata.get("paramData");
                            double paramVal = Double.parseDouble(paramData);
                            if (paramVal == (int) paramVal) {
                                // it's a whole number
                                paramData = String.valueOf((int) paramVal);
                            } else {
                                paramData = String.valueOf(paramVal); // keep decimals
                            }
                            getFinished = true;
                        }
                        //check for timeout
                        long currentTime = System.currentTimeMillis();
                        if (currentTime - startTime > responseDelay) {
                            Log.d("BT", "BTSocket - buttonGet press - failed to receive response to GET in time");
                            isTimedOut = true;
                            paramData = "?";
                            getFinished = true;
                        }

                        if (getFinished) {
                            Handler getHandler = new Handler(Looper.getMainLooper());
                            getHandler.postAtFrontOfQueue(() ->
                            {
                                //write param value out
                                textOut.setText(paramData);

                                if (!isTimedOut) {
                                    cmdResultTextId.setText("GET OK");
                                    cmdLamp.setBackgroundColor(Color.GREEN);
                                } else {
                                    cmdResultTextId.setText("GET failed");
                                    cmdLamp.setBackgroundColor(Color.RED);
                                }
                                isGetCmdFinished = true;
                            });

                            //exit while loop
                            break;
                        } else {
                            delay(cycleDelay);
                        }
                    }
                }
            };
            tGet.start();
        }
    }

    void BTGetGlobalTime(int id, TextView textOut) {
        if (BTConnected && id != 0 && textOut != null) {
            byte[] buffer = new byte[20];
            byte ctr = 0;

            //set number of data to zero first
            buffer[ctr++] = 0;

            //set get cmd
            buffer[ctr++] = 0x0C;

            //set id
            buffer[ctr++] = (byte) (id & 0xff);
            buffer[ctr++] = (byte) ((id & 0xff00) >> 8);

            //set CR-NL ending
            buffer[ctr++] = (byte) '\r';
            buffer[ctr++] = (byte) '\n';

            //number of data to send
            buffer[0] = (byte) (ctr - 1);

            try {
                Log.d("BT", "BTSocket - trying GET");
                bluetoothSocket.getOutputStream().write(buffer, 0, ctr);
            } catch (IOException e) {
                Log.d("BT", "BTSocket - buttonGet press - failed to write");
            }

            Handler getHandler = new Handler(Looper.getMainLooper());
            getHandler.post(() ->
            {
                cmdResultTextId.setText("Ongoing...");
                cmdLamp.setBackgroundColor(Color.YELLOW);
            });

            isGetCmdFinished = false;

            Thread tGet = new Thread() {
                boolean newParamReceived = false;
                boolean getFinished = false;
                String paramData = "";

                //wait for reply
                boolean isTimedOut = false;
                final long startTime = System.currentTimeMillis();

                @Override
                public void run() {
                    while (true) {
                        //get data from Rx thread, break while if new param received
                        Map<String, Object> BTdata = BtRxThread.getBTData();
                        newParamReceived = (boolean) BTdata.get("newParamReceived");
                        //Log.d("BT", "MainActivty - buttonGetPx press - checking response receive: " + newParamReceived);
                        if (newParamReceived) {
                            paramData = (String) BTdata.get("paramData");
                            double paramVal = Double.parseDouble(paramData);
                            if (paramVal == (int) paramVal) {
                                // it's a whole number
                                paramData = String.valueOf((int) paramVal);
                            } else {
                                paramData = String.valueOf(paramVal); // keep decimals
                            }
                            getFinished = true;
                        }
                        //check for timeout
                        long currentTime = System.currentTimeMillis();
                        if (currentTime - startTime > responseDelay) {
                            Log.d("BT", "BTSocket - buttonGet press - failed to receive response to GET in time");
                            isTimedOut = true;
                            paramData = "?";
                            getFinished = true;
                        }

                        if (getFinished) {
                            Handler getHandler = new Handler(Looper.getMainLooper());
                            getHandler.postAtFrontOfQueue(() ->
                            {
                                //write formated time value out
                                String formatedTime = paramData.substring(1,2) + '.' +
                                        paramData.substring(3,4) + '.' +
                                        paramData.substring(5,6) + '.' +
                                        paramData.substring(7,8) + '-' +
                                        paramData.substring(9,10) + ':' +
                                        paramData.substring(11,12) + ':' +
                                        paramData.substring(13,14);
                                textOut.setText(formatedTime);

                                if (!isTimedOut) {
                                    cmdResultTextId.setText("GET OK");
                                    cmdLamp.setBackgroundColor(Color.GREEN);
                                } else {
                                    cmdResultTextId.setText("GET failed");
                                    cmdLamp.setBackgroundColor(Color.RED);
                                }
                                isGetCmdFinished = true;
                            });

                            //exit while loop
                            break;
                        } else {
                            delay(cycleDelay);
                        }
                    }
                }
            };
            tGet.start();
        }
    }

    void BTSetMeasurement(int cmd, boolean flag)
    {
        if (BTConnected && cmd != 0)
        {
            byte[] buffer = new byte[20];
            char flagChr = (flag) ? '1' : '0';

            byte ctr = 0;
            //set number of data to zero first
            buffer[ctr++] = 0;

            //set get cmd
            buffer[ctr++] = 0x03;

            //set id
            buffer[ctr++] = (byte) (cmd & 0xff);
            buffer[ctr++] = (byte) ((cmd & 0xff00) >> 8);

            //set flag
            buffer[ctr++] = (byte) flagChr;

            //set CR-NL ending
            buffer[ctr++] = (byte) '\r';
            buffer[ctr++] = (byte) '\n';

            //number of data to send
            buffer[0] = (byte) (ctr - 1);

            try {
                Log.d("BT", "BTSocket - trying SET measurement flag");
                bluetoothSocket.getOutputStream().write(buffer, 0, ctr);
            } catch (IOException e) {
                Log.d("BT", "BTSocket - failed to write measurement flag");
            }

            Handler getHandler = new Handler(Looper.getMainLooper());
            getHandler.post(() ->
            {
                cmdResultTextId.setText("Ongoing...");
                cmdLamp.setBackgroundColor(Color.YELLOW);
            });

            isSetCmdFinished = false;

            Thread tSet = new Thread() {
                boolean newParamReceived = false;
                boolean setFinished = false;
                String paramData = "";

                //wait for reply
                boolean isTimedOut = false;
                final long startTime = System.currentTimeMillis();

                @Override
                public void run()
                {
                    while (true)
                    {
                        //get data from Rx thread, break while if new param received
                        Map<String, Object> BTdata = BtRxThread.getBTData();
                        newParamReceived = (boolean) BTdata.get("newParamReceived");
                        if (newParamReceived)
                        {
                            paramData = (String) BTdata.get("paramData");
                            setFinished = true;
                        }

                        //check for timeout
                        long currentTime = System.currentTimeMillis();
                        if (currentTime - startTime > responseDelay)
                        {
                            Log.d("BT", "BTSocket - failed to receive response to SET measurement flag in time");
                            setFinished = true;
                            isTimedOut = true;
                        }

                        if (setFinished)
                        {
                            if (isTimedOut)
                            {
                                cmdResultTextId.setText("SET measurement flag failed");
                                cmdLamp.setBackgroundColor(Color.RED);
                            }
                            else
                            {
                                boolean valueToReceive = paramData.equals("1");

                                Handler setHandler = new Handler(Looper.getMainLooper());
                                setHandler.post(() ->
                                {
                                    if (valueToReceive == flag) {
                                        cmdResultTextId.setText("SET OK");
                                        cmdLamp.setBackgroundColor(Color.GREEN);
                                    } else {
                                        cmdResultTextId.setText("SET failed");
                                        cmdLamp.setBackgroundColor(Color.RED);
                                    }

                                    isSetCmdFinished = true;
                                });
                            }

                            //exit while loop
                            break;
                        }
                        else
                        {
                            delay(cycleDelay);
                        }
                    }

                }
            };
            tSet.start();
        }
    }

    void BTSetCommand(int cmd, EditText textIn) {
        if (BTConnected && cmd != 0 && textIn != null) {
            byte[] buffer = new byte[20];
            byte ctr = 0;
            //set number of data to zero first
            buffer[ctr++] = 0;

            //set get cmd
            buffer[ctr++] = 0x03;

            //set id
            buffer[ctr++] = (byte) (cmd & 0xff);
            buffer[ctr++] = (byte) ((cmd & 0xff00) >> 8);

            //set value as char
            Charset charset = StandardCharsets.US_ASCII;
            String textInStr = textIn.getText().toString();
            byte[] textBytes = textInStr.getBytes(charset);
            for (byte textByte : textBytes) {
                buffer[ctr++] = textByte;
            }

            //set CR-NL ending
            buffer[ctr++] = (byte) '\r';
            buffer[ctr++] = (byte) '\n';

            //number of data to send
            buffer[0] = (byte) (ctr - 1);

            try {
                Log.d("BT", "BTSocket - trying SET");
                bluetoothSocket.getOutputStream().write(buffer, 0, ctr);
            } catch (IOException e) {
                Log.d("BT", "BTSocket - failed to write");
            }

            Handler getHandler = new Handler(Looper.getMainLooper());
            getHandler.post(() ->
            {
                cmdResultTextId.setText("Ongoing...");
                cmdLamp.setBackgroundColor(Color.YELLOW);
            });

            isSetCmdFinished = false;

            Thread tSet = new Thread() {
                boolean newParamReceived = false;
                boolean setFinished = false;
                String paramData = "";

                //wait for reply
                boolean isTimedOut = false;
                final long startTime = System.currentTimeMillis();

                @Override
                public void run() {
                    while (true) {
                        //get data from Rx thread, break while if new param received
                        Map<String, Object> BTdata = BtRxThread.getBTData();
                        newParamReceived = (boolean) BTdata.get("newParamReceived");
                        if (newParamReceived) {
                            paramData = (String) BTdata.get("paramData");
                            setFinished = true;
                        }

                        //check for timeout
                        long currentTime = System.currentTimeMillis();
                        if (currentTime - startTime > responseDelay) {
                            Log.d("BT", "BTSocket - failed to receive response to SET in time");
                            setFinished = true;
                            isTimedOut = true;
                        }

                        if (setFinished) {
                            if (isTimedOut) {
                                Handler setHandler = new Handler(Looper.getMainLooper());
                                setHandler.post(() ->
                                {
                                    cmdResultTextId.setText("SET failed");
                                    cmdLamp.setBackgroundColor(Color.RED);
                                });
                            } else {
                                float valueToReceive = Float.parseFloat(paramData);
                                float valueFromTextBox = Float.parseFloat(textIn.getText().toString());

                                Handler setHandler = new Handler(Looper.getMainLooper());
                                setHandler.post(() ->
                                {
                                    if ((valueToReceive - valueFromTextBox) < 0.01f) {
                                        cmdResultTextId.setText("SET OK");
                                        cmdLamp.setBackgroundColor(Color.GREEN);
                                    } else {
                                        cmdResultTextId.setText("SET failed");
                                        cmdLamp.setBackgroundColor(Color.RED);
                                    }

                                    isSetCmdFinished = true;
                                });
                            }

                            //exit while loop
                            break;
                        } else {
                            delay(cycleDelay);
                        }
                    }

                }
            };
            tSet.start();
        }
    }

    void BTSetGlobalTime(int cmd, TextView textIn) {
        if (BTConnected && cmd != 0 && textIn != null) {
            byte[] buffer = new byte[20];
            byte ctr = 0;
            //set number of data to zero first
            buffer[ctr++] = 0;

            //set get cmd
            buffer[ctr++] = 0x03;

            //set id
            buffer[ctr++] = (byte) (cmd & 0xff);
            buffer[ctr++] = (byte) ((cmd & 0xff00) >> 8);

            //set value as char
            SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmmss", Locale.getDefault());
            String currentDateandTime = sdf.format(new Date());
            Charset charset = StandardCharsets.US_ASCII;
            byte[] textBytes = currentDateandTime.getBytes(charset);

            for (byte textByte : textBytes) {
                buffer[ctr++] = textByte;
            }

            //set CR-NL ending
            buffer[ctr++] = (byte) '\r';
            buffer[ctr++] = (byte) '\n';

            //number of data to send
            buffer[0] = (byte) (ctr - 1);

            try {
                Log.d("BT", "BTSocket - trying SET");
                bluetoothSocket.getOutputStream().write(buffer, 0, ctr);
            } catch (IOException e) {
                Log.d("BT", "BTSocket - failed to write");
            }

            Handler getHandler = new Handler(Looper.getMainLooper());
            getHandler.post(() ->
            {
                cmdResultTextId.setText("Ongoing...");
                cmdLamp.setBackgroundColor(Color.YELLOW);
            });

            isSetCmdFinished = false;

            Thread tSet = new Thread() {
                boolean newParamReceived = false;
                boolean setFinished = false;
                String paramData = "";

                //wait for reply
                boolean isTimedOut = false;
                final long startTime = System.currentTimeMillis();

                @Override
                public void run() {
                    while (true) {
                        //get data from Rx thread, break while if new param received
                        Map<String, Object> BTdata = BtRxThread.getBTData();
                        newParamReceived = (boolean) BTdata.get("newParamReceived");
                        if (newParamReceived) {
                            paramData = (String) BTdata.get("paramData");
                            setFinished = true;
                        }

                        //check for timeout
                        long currentTime = System.currentTimeMillis();
                        if (currentTime - startTime > responseDelay) {
                            Log.d("BT", "BTSocket - failed to receive response to SET in time");
                            setFinished = true;
                            isTimedOut = true;
                        }

                        if (setFinished) {
                            if (isTimedOut) {
                                Handler setHandler = new Handler(Looper.getMainLooper());
                                setHandler.post(() ->
                                {
                                    cmdResultTextId.setText("SET failed");
                                    cmdLamp.setBackgroundColor(Color.RED);
                                });
                            } else {
                                float valueToReceive = Float.parseFloat(currentDateandTime);
                                float valueFromTextBox = Float.parseFloat(textIn.getText().toString());

                                Handler setHandler = new Handler(Looper.getMainLooper());
                                setHandler.post(() ->
                                {
                                    if ((valueToReceive - valueFromTextBox) < 0.01f) {
                                        cmdResultTextId.setText("SET OK");
                                        cmdLamp.setBackgroundColor(Color.GREEN);
                                    } else {
                                        cmdResultTextId.setText("SET failed");
                                        cmdLamp.setBackgroundColor(Color.RED);
                                    }

                                    isSetCmdFinished = true;
                                });
                            }

                            //exit while loop
                            break;
                        } else {
                            delay(cycleDelay);
                        }
                    }

                }
            };
            tSet.start();
        }
    }

    void BTStreamCommand(int id, boolean isChecked) {
        byte[] buffer = new byte[20];
        byte ctr = 0;

        //set number of data to zero first
        buffer[ctr++] = 0;

        //set get cmd
        buffer[ctr++] = 0x30;

        //set id
        buffer[ctr++] = (byte) (id & 0xff);
        buffer[ctr++] = (byte) ((id & 0xff00) >> 8);

        //set On
        if (isChecked) {
            Log.d("BT", "BTSocket - onCreate - checkBox checked");
            buffer[ctr++] = 'O';    //ON
        } else {
            Log.d("BT", "BTSocket - onCreate - checkBox unchecked");
            buffer[ctr++] = 'F';    //OFF
        }

        //set CR-NL ending
        buffer[ctr++] = (byte) '\r';
        buffer[ctr++] = (byte) '\n';

        //number of data to send
        buffer[0] = (byte) (ctr - 1);

        try {
            Log.d("BT", "BTSocket - trying STREAM");
            bluetoothSocket.getOutputStream().write(buffer, 0, ctr);
        } catch (IOException e) {
            Log.d("BT", "BTSocket - checkBox_sensor_signal_X press - failed to write");
        }
    }

    void AddVal(EditText text, float add) {
        if (text != null) {
            String textStr = text.getText().toString();
            if (!textStr.isEmpty() && !textStr.equals("?")) {
                float value = Float.parseFloat(textStr) + add;
                String valStr;

                if (value == (int) value) {
                    // it's a whole number
                    valStr = String.valueOf((int) value);
                } else {
                    valStr = String.valueOf(value); // keep decimals
                }
                text.setText(valStr);
            }
        }
    }

    private void delay(int value) {
        try {
            Thread.sleep(value);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }
}

