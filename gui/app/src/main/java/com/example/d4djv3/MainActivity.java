package com.example.d4djv3;

import android.Manifest;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.provider.Settings;
import android.util.Log;
import android.util.Pair;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

import androidx.activity.EdgeToEdge;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

public class MainActivity extends BaseActivity {

    private BluetoothAdapter bluetoothAdapter;
    private boolean btEnabled;
    private boolean permissionMissing;
    ActivityResultLauncher<String> requestBluetoothPermissionLauncherForRefresh;
    Thread btReceiveThread = null;

    BluetoothSocket btSocket;
    InputStream btInStream;
    OutputStream btOutStream;

    TextView textBTstatus;
    Button BTbutton;

    TextView textViewPlusMinusValue;

    CheckBox checkBoxPx;
    CheckBox checkBoxIx;
    CheckBox checkBoxDx;
    CheckBox checkBoxPy;
    CheckBox checkBoxIy;
    CheckBox checkBoxDy;
    CheckBox checkBoxPz;
    CheckBox checkBoxIz;
    CheckBox checkBoxFFx;
    CheckBox checkBoxFFy;
    CheckBox checkBoxFFdx;
    CheckBox checkBoxFFdy;
    CheckBox checkBoxSatI;
    CheckBox checkBoxSatPID;
    CheckBox checkBoxDTermC;
    CheckBox checkBoxCPx;
    CheckBox checkBoxCIx;
    CheckBox checkBoxCPy;
    CheckBox checkBoxCIy;
    CheckBox checkBoxCFFdx;
    CheckBox checkBoxCFFdy;
    CheckBox checkBoxCSatI;
    CheckBox checkBoxCSatPID;

    List<CheckBox> checkBoxes;

    boolean newParamReceived = false;
    String paramData = "";

    float x1, y1, x2, y2;

    //creat a map for data to send BT Rx thread
    private static Map<String, Object> dataFromMain = new HashMap<>();

    // Add a public method that allows BT Rx class to access the data
    public static Map<String, Object> getData() {
        return dataFromMain;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.stream), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        Log.d("BT", "MainActivty - onCreate - called");

        textBTstatus = findViewById(R.id.textBTStatus);
        BTbutton = findViewById(R.id.button);
        //
        textViewPlusMinusValue = findViewById(R.id.textViewPlusMinusVal);
        //
        Button buttonGet = findViewById(R.id.buttonGet);
        Button buttonSet = findViewById(R.id.buttonSet);
        Button buttonInc = findViewById(R.id.buttonInc);
        Button buttonDec = findViewById(R.id.buttonDec);
        Button buttonGetAll = findViewById(R.id.buttonGetAll);
        Button buttonSetAll = findViewById(R.id.buttonSetAll);
        //
        checkBoxPx = findViewById(R.id.checkBoxPx);
        checkBoxIx = findViewById(R.id.checkBoxIx);
        checkBoxDx = findViewById(R.id.checkBoxDx);
        checkBoxPy = findViewById(R.id.checkBoxPy);
        checkBoxIy = findViewById(R.id.checkBoxIy);
        checkBoxDy = findViewById(R.id.checkBoxDy);
        checkBoxPz = findViewById(R.id.checkBoxPz);
        checkBoxIz = findViewById(R.id.checkBoxIz);
        checkBoxFFx = findViewById(R.id.checkBoxFFx);
        checkBoxFFy = findViewById(R.id.checkBoxFFy);
        checkBoxFFdx = findViewById(R.id.checkBoxFFdx);
        checkBoxFFdy = findViewById(R.id.checkBoxFFdy);
        checkBoxSatI = findViewById(R.id.checkBoxSatI);
        checkBoxSatPID = findViewById(R.id.checkBoxSatPID);
        checkBoxDTermC = findViewById(R.id.checkBoxDTermC);
        checkBoxCPx = findViewById(R.id.checkBoxCPx);
        checkBoxCIx = findViewById(R.id.checkBoxCIx);
        checkBoxCPy = findViewById(R.id.checkBoxCPy);
        checkBoxCIy = findViewById(R.id.checkBoxCIy);
        checkBoxCFFdx = findViewById(R.id.checkBoxCFFdx);
        checkBoxCFFdy = findViewById(R.id.checkBoxCFFdy);
        checkBoxCSatI = findViewById(R.id.checkBoxCSatI);
        checkBoxCSatPID = findViewById(R.id.checkBoxCSatPID);

        checkBoxes = Arrays.asList(checkBoxPx, checkBoxIx, checkBoxDx,
                checkBoxPy, checkBoxIy, checkBoxDy,
                checkBoxPz, checkBoxIz, checkBoxFFx,
                checkBoxFFy, checkBoxFFdx, checkBoxFFdy,
                checkBoxSatI, checkBoxSatPID, checkBoxDTermC,
                checkBoxCPx, checkBoxCIx, checkBoxCPy, checkBoxCIy,
                checkBoxCFFdx, checkBoxCFFdy, checkBoxCSatI, checkBoxCSatPID);

        //check BT hardware availability
        if (getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH)) {
            bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        } else {
            setBTTexts("", "no BT on device");
        }

        BTEnableAndPermisson();

        BTbutton.setOnClickListener(v ->
        {
            Button buttomLamp = findViewById(R.id.buttonCmdLamp);
            buttomLamp.setBackgroundColor(Color.YELLOW);

            Log.d("BT", "MainActivty - BTbutton press");
            if (!btEnabled) {
                Log.d("BT", "MainActivty - BTbutton press - BT enabled missing");
                Intent intent = new Intent();
                intent.setAction(Settings.ACTION_BLUETOOTH_SETTINGS);
                startActivity(intent);
            } else if (permissionMissing) {
                Log.d("BT", "MainActivty - BTbutton press - BT permission missing");
                setBTTexts("", "BT permission missing, click button");
                buttomLamp.setBackgroundColor(Color.RED);
            } else {
                Log.d("BT", "MainActivty - BTbutton press - BT connect");
                if (!BTSocket.getInstance().getBluetoothConnection()) {
                    Log.d("BT", "MainActivty - BTbutton press - trying to connect");
                    setBTTexts("", "Connecting...");

                    Thread tConnect = new Thread() {
                        @Override
                        public void run() {
                            boolean isConnected = false;

                            for (BluetoothDevice device : bluetoothAdapter.getBondedDevices()) {
                                if (device.getName() != null && device.getName().contains("HC-05")) {
                                    isConnected = tryConnect(device);
                                    if (isConnected) {
                                        break;
                                    }
                                }
                            }

                            if (isConnected) {
                                setBTTexts("Disconnect", "Connected");
                                buttomLamp.setBackgroundColor(Color.GREEN);
                            } else {
                                setBTTexts("", "Could not connect!");
                                buttomLamp.setBackgroundColor(Color.RED);
                            }
                        }
                    };
                    tConnect.start();
                } else {
                    Log.d("BT", "MainActivty - BTbutton press - trying to disconnect");
                    setBTTexts("", "Disconnecting...");

                    tryDisconnect();

                    buttomLamp.setBackgroundColor(0xFF673AB7);

                    BTSocket.getInstance().setBluetoothConnection(false);
                }
            }
        });

        checkBoxPx.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxPx);
        });
        checkBoxIx.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxIx);
        });
        checkBoxDx.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxDx);
        });
        checkBoxPy.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxPy);
        });
        checkBoxIy.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxIy);
        });
        checkBoxDy.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxDy);
        });
        checkBoxPz.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxPz);
        });
        checkBoxIz.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxIz);
        });
        checkBoxFFx.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxFFx);
        });
        checkBoxFFy.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxFFy);
        });
        checkBoxFFdx.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxFFdx);
        });
        checkBoxFFdy.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxFFdy);
        });
        checkBoxSatI.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxSatI);
        });
        checkBoxSatPID.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxSatPID);
        });
        checkBoxDTermC.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxDTermC);
        });
        checkBoxCPx.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxCPx);
        });
        checkBoxCIx.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxCIx);
        });
        checkBoxCPy.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxCPy);
        });
        checkBoxCIy.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxCIy);
        });
        checkBoxCSatI.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxCSatI);
        });
        checkBoxCSatPID.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxCSatPID);
        });
        checkBoxCFFdx.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxCFFdx);
        });
        checkBoxCFFdy.setOnClickListener(v ->
        {
            uncheckExpect(checkBoxCFFdy);
        });

        buttonGet.setOnClickListener(v ->
        {
            EditText text = getEditText();
            int id = getID();

            BTSocket.getInstance().BTGetCommand(id, text);
        });
        buttonSet.setOnClickListener(v ->
        {
            EditText text = getEditText();
            int id = getID();

            BTSocket.getInstance().BTSetCommand(id, text);
        });
        buttonInc.setOnClickListener(v ->
        {
            EditText text = getEditText();
            String add = textViewPlusMinusValue.getText().toString();
            int addVal = Integer.parseInt(add.substring(1));

            BTSocket.getInstance().AddVal(text, addVal);
        });
        buttonDec.setOnClickListener(v ->
        {
            EditText text = getEditText();
            String add = textViewPlusMinusValue.getText().toString();
            int addVal = Integer.parseInt(add.substring(1));

            BTSocket.getInstance().AddVal(text, -addVal);
        });
        buttonGetAll.setOnClickListener(v ->
        {
            List<Pair<EditText, Integer>> getList = new ArrayList<>();

            getList.add(new Pair<>(findViewById(R.id.editTextPx), 2000));
            getList.add(new Pair<>(findViewById(R.id.editTextIx), 2001));
            getList.add(new Pair<>(findViewById(R.id.editTextDx), 2002));
            getList.add(new Pair<>(findViewById(R.id.editTextPy), 2003));
            getList.add(new Pair<>(findViewById(R.id.editTextIy), 2004));
            getList.add(new Pair<>(findViewById(R.id.editTextDy), 2005));
            getList.add(new Pair<>(findViewById(R.id.editTextPz), 2006));
            getList.add(new Pair<>(findViewById(R.id.editTextIz), 2007));
            getList.add(new Pair<>(findViewById(R.id.editTextFFx), 2034));
            getList.add(new Pair<>(findViewById(R.id.editTextFFy), 2035));
            getList.add(new Pair<>(findViewById(R.id.editTextFFdx), 2036));
            getList.add(new Pair<>(findViewById(R.id.editTextFFdy), 2037));
            getList.add(new Pair<>(findViewById(R.id.editTextSatI), 2009));
            getList.add(new Pair<>(findViewById(R.id.editTextSatPID), 2010));
            getList.add(new Pair<>(findViewById(R.id.editTextDTermC), 2011));
            getList.add(new Pair<>(findViewById(R.id.editTextCPx), 2022));
            getList.add(new Pair<>(findViewById(R.id.editTextCIx), 2023));
            getList.add(new Pair<>(findViewById(R.id.editTextCPy), 2025));
            getList.add(new Pair<>(findViewById(R.id.editTextCIy), 2026));
            getList.add(new Pair<>(findViewById(R.id.editTextCSatI), 2031));
            getList.add(new Pair<>(findViewById(R.id.editTextCSatPID), 2032));
            getList.add(new Pair<>(findViewById(R.id.editTextCFFdx), 2040));
            getList.add(new Pair<>(findViewById(R.id.editTextCFFdy), 2041));

            Thread tGetAll = new Thread() {
                @Override
                public void run() {
                    for (Pair<EditText, Integer> get : getList) {
                        EditText text = get.first;
                        int id = get.second;

                        BTSocket.getInstance().BTGetCommand(id, text);
                        while (!BTSocket.getInstance().getIsGetCmdFinished()) {
                            try {
                                Thread.sleep(50);
                            } catch (InterruptedException e) {
                                throw new RuntimeException(e);
                            }
                        }
                    }
                }
            };
            tGetAll.start();
        });
        buttonSetAll.setOnClickListener(v ->
        {
            List<Pair<EditText, Integer>> setList = new ArrayList<>();

            setList.add(new Pair<>(findViewById(R.id.editTextPx), 2000));
            setList.add(new Pair<>(findViewById(R.id.editTextIx), 2001));
            setList.add(new Pair<>(findViewById(R.id.editTextDx), 2002));
            setList.add(new Pair<>(findViewById(R.id.editTextPy), 2003));
            setList.add(new Pair<>(findViewById(R.id.editTextIy), 2004));
            setList.add(new Pair<>(findViewById(R.id.editTextDy), 2005));
            setList.add(new Pair<>(findViewById(R.id.editTextPz), 2006));
            setList.add(new Pair<>(findViewById(R.id.editTextIz), 2007));
            setList.add(new Pair<>(findViewById(R.id.editTextFFx), 2034));
            setList.add(new Pair<>(findViewById(R.id.editTextFFy), 2035));
            setList.add(new Pair<>(findViewById(R.id.editTextFFdx), 2036));
            setList.add(new Pair<>(findViewById(R.id.editTextFFdy), 2037));
            setList.add(new Pair<>(findViewById(R.id.editTextSatI), 2009));
            setList.add(new Pair<>(findViewById(R.id.editTextSatPID), 2010));
            setList.add(new Pair<>(findViewById(R.id.editTextDTermC), 2011));
            setList.add(new Pair<>(findViewById(R.id.editTextCPx), 2022));
            setList.add(new Pair<>(findViewById(R.id.editTextCIx), 2023));
            setList.add(new Pair<>(findViewById(R.id.editTextCPy), 2025));
            setList.add(new Pair<>(findViewById(R.id.editTextCIy), 2026));
            setList.add(new Pair<>(findViewById(R.id.editTextCSatI), 2031));
            setList.add(new Pair<>(findViewById(R.id.editTextCSatPID), 2032));
            setList.add(new Pair<>(findViewById(R.id.editTextCFFdx), 2040));
            setList.add(new Pair<>(findViewById(R.id.editTextCFFdy), 2041));

            Thread tSetAll = new Thread() {
                @Override
                public void run() {
                    for (Pair<EditText, Integer> set : setList) {
                        EditText text = set.first;
                        int id = set.second;

                        text.setCursorVisible(true);
                        text.selectAll();
                        BTSocket.getInstance().BTSetCommand(id, text);
                        while (!BTSocket.getInstance().getIsSetCmdFinished()) {
                            try {
                                Thread.sleep(50);
                            } catch (InterruptedException e) {
                                throw new RuntimeException(e);
                            }
                        }
                        text.setCursorVisible(false);
                    }
                }
            };
            tSetAll.start();
        });

        textViewPlusMinusValue.setOnClickListener(v ->
        {
            String plusMinusValue = textViewPlusMinusValue.getText().toString();

            if (plusMinusValue.equals("±1")) {
                textViewPlusMinusValue.setText("±10");
            } else if (plusMinusValue.equals("±10")) {
                textViewPlusMinusValue.setText("±100");
            } else {
                textViewPlusMinusValue.setText("±1");
            }
        });

        requestBluetoothPermissionLauncherForRefresh = registerForActivityResult(
                new ActivityResultContracts.RequestPermission(),
                granted -> onPermissionsResult(granted, this::BTEnableAndPermisson));

    }

    @Override
    protected void onResume() {
        Log.d("BT", "MainActivty - onResume");
        super.onResume();
        if (!BTSocket.getInstance().getBluetoothConnection()) {
            BTEnableAndPermisson();
        } else {
            BTSocket.getInstance().setBTcmdResultTextID(findViewById(R.id.textBTcmd));

            //todo reconnect bt on resume
        }

    }

    @Override
    protected void onSwipeLeft() {
        Log.d("BT", "MainActivty - onTouchEvent - swiped left");
        Intent intent = new Intent(MainActivity.this, MeasurementActivity.class);
        startActivity(intent);
    }

    @Override
    protected void onSwipeRight() {
        Log.d("BT", "MainActivty - onTouchEvent - swiped right");
        Intent intent = new Intent(MainActivity.this, StreamAccActivity.class);
        startActivity(intent);
    }

//    @Override
//    public boolean onTouchEvent(MotionEvent e)
//    {
//        switch (e.getAction())
//        {
//            case MotionEvent.ACTION_DOWN:
//            {
//                Log.d("BT", "MainActivty - onTouchEvent - touch down");
//                x1 = e.getX();
//                y1 = e.getY();
//                break;
//            }
//            case MotionEvent.ACTION_UP:
//            {
//                x2 = e.getX();
//                y2 = e.getY();
//
//                float dx = x2 - x1;
//
//                if(BTSocket.getInstance().getBluetoothConnection())
//                {
//                    if (dx < -250)
//                    {
//                        Log.d("BT", "MainActivty - onTouchEvent - swiped right: " + dx);
//                        Intent intent = new Intent(MainActivity.this, StreamActivity.class);
//                        startActivity(intent);
//                    }
//                    else if (dx > 250)
//                    {
//                        Log.d("BT", "MainActivty - onTouchEvent - swiped left: " + dx);
//                        Intent intent = new Intent(MainActivity.this, StreamAccActivity.class);
//                        startActivity(intent);
//                    }
//                }
//                else
//                {
//                    TextView textBTstatus = findViewById(R.id.textBTStatus);
//                    setBTTexts("", "BT not connected yet");
//                }
//                break;
//            }
//        }
//
//        return true;
//    }

    private boolean tryConnect(BluetoothDevice device) {
        try {
            UUID BLUETOOTH_SPP = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
            btSocket = device.createRfcommSocketToServiceRecord(BLUETOOTH_SPP);
            btSocket.connect();
            btInStream = btSocket.getInputStream();
            btOutStream = btSocket.getOutputStream();

            //save socket info and ids
            BTSocket.getInstance().setBluetoothSocket(btSocket);
            BTSocket.getInstance().setBluetoothConnection(true);
            BTSocket.getInstance().setBTcmdResultTextID(findViewById(R.id.textBTcmd));
            BTSocket.getInstance().setBTCmdLamp(findViewById(R.id.buttonCmdLamp));

            //set default
            dataFromMain.put("cancelRx", false);

            btReceiveThread = new BtRxThread(btInStream);
            btReceiveThread.start();

            return true;
        } catch (Exception e) {
            Log.d("BT", "MainActivty - BTbutton press - failed to connect");
            if (btSocket != null) {
                try {
                    btSocket.close();
                } catch (IOException ex) {
                    Log.e("BT", "MainActivty - Error closing socket", ex);
                }
            }

            return false;
        }
    }

    private void tryDisconnect() {
        try {
            //stop rx thread
            dataFromMain.put("cancelRx", true);
            Thread.sleep(10);

            boolean isRxStopped = false;
            long startTime = System.currentTimeMillis();
            while (true) {
                //get data from Rx thread, break while-loop if new param received
                Map<String, Object> BTdata = BtRxThread.getBTData();
                isRxStopped = (boolean) BTdata.get("isRxStopped");
                if (isRxStopped) {
                    break;
                }
                //check for timeout
                long currentTime = System.currentTimeMillis();
                if (currentTime - startTime > 2000) {
                    Log.d("BT", "MainActivty - BTbutton press - timed out to disconnect");
                    break;
                }
            }

            btSocket.close();

            setBTTexts("Connect", "Disconnected");
            Log.d("BT", "MainActivty - BTbutton press - BT disconnected");
        } catch (IOException e) {
            Log.d("BT", "MainActivty - BTbutton press - failed to disconnect");
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    private void setBTTexts(String buttonText, String textText) {
        Handler textHandler = new Handler(Looper.getMainLooper());
        textHandler.post(new Runnable() {
            @Override
            public void run() {
                if (buttonText != "") {
                    BTbutton.setText(buttonText);
                }
                if (textText != "") {
                    textBTstatus.setText(textText);
                }
            }
        });
    }

    protected void BTEnableAndPermisson() {
        Log.d("BT", "MainActivty - BTConnectionHandler");
        TextView textBTstatus = findViewById(R.id.textBTStatus);
        if (bluetoothAdapter != null) {
            btEnabled = true;
            permissionMissing = checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED;

            if (!bluetoothAdapter.isEnabled()) {
                textBTstatus.setText("BT needs to be turned on");
                btEnabled = false;
            } else if (permissionMissing) {
                textBTstatus.setText("BT permission missing");
                boolean showRationale = shouldShowRequestPermissionRationale(Manifest.permission.BLUETOOTH_CONNECT);
                //showRationale = false;
                if (showRationale) {
                    showRationaleDialog((dialog, which) ->
                            requestBluetoothPermissionLauncherForRefresh.launch(Manifest.permission.BLUETOOTH_CONNECT));
                } else {
                    requestBluetoothPermissionLauncherForRefresh.launch(Manifest.permission.BLUETOOTH_CONNECT);
                }

            } else {
                //do nothing
            }
        }
    }

    /**
     * Android 12 permission handling
     */
    private void showRationaleDialog(DialogInterface.OnClickListener listener) {
        Log.d("BT", "MainActivty-showRationaleDialog");
        final AlertDialog.Builder builder = new AlertDialog.Builder(getApplicationContext());
        builder.setTitle("Bluetooth permission");
        builder.setMessage("Bluetooth permission is required by this App. Please grant in next dialog.");
        builder.setNegativeButton("Cancel", null);
        builder.setPositiveButton("Continue", listener);
        builder.show();
    }

    interface PermissionGrantedCallback {
        void call();
    }

    void onPermissionsResult(boolean granted, PermissionGrantedCallback cb) {
        Log.d("BT", "MainActivty-onPermissionsResult");
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.S)
            return;
        boolean showRationale = shouldShowRequestPermissionRationale(Manifest.permission.BLUETOOTH_CONNECT);
        if (granted) {
            cb.call();
        } else if (showRationale) {
            showRationaleDialog((dialog, which) -> cb.call());
        } else {
            showSettingsDialog();
        }
    }

    private void showSettingsDialog() {
        Log.d("BT", "MainActivty-showSettingsDialog");
        String s = getResources().getString(getResources().getIdentifier("@android:string/permgrouplab_nearby_devices", null, null));
        final AlertDialog.Builder builder = new AlertDialog.Builder(getApplicationContext());
        builder.setTitle("Bluetooth permission");
        builder.setMessage(String.format("Bluetooth permission was permanently denied. You have to enable permission \\\"%s\\\" in App settings.", s));
        builder.setNegativeButton("Cancel", null);
        builder.setPositiveButton("Settings", (dialog, which) ->
                startActivity(new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS,
                        Uri.parse("package:" + "D4Dj"))));
        builder.show();
    }

    private int getID() {
        int val = 0;
        if (checkBoxPx.isChecked()) {
            val = 2000;
        } else if (checkBoxIx.isChecked()) {
            val = 2001;
        } else if (checkBoxDx.isChecked()) {
            val = 2002;
        } else if (checkBoxPy.isChecked()) {
            val = 2003;
        } else if (checkBoxIy.isChecked()) {
            val = 2004;
        } else if (checkBoxDy.isChecked()) {
            val = 2005;
        } else if (checkBoxPz.isChecked()) {
            val = 2006;
        } else if (checkBoxIz.isChecked()) {
            val = 2007;
        } else if (checkBoxSatI.isChecked()) {
            val = 2009;
        } else if (checkBoxSatPID.isChecked()) {
            val = 2010;
        } else if (checkBoxDTermC.isChecked()) {
            val = 2011;
        } else if (checkBoxFFx.isChecked()) {
            val = 2034;
        } else if (checkBoxFFy.isChecked()) {
            val = 2035;
        } else if (checkBoxFFdx.isChecked()) {
            val = 2036;
        } else if (checkBoxFFdy.isChecked()) {
            val = 2037;
        } else if (checkBoxCPx.isChecked()) {
            val = 2022;
        } else if (checkBoxCIx.isChecked()) {
            val = 2023;
        } else if (checkBoxCPy.isChecked()) {
            val = 2025;
        } else if (checkBoxCIy.isChecked()) {
            val = 2026;
        } else if (checkBoxCSatI.isChecked()) {
            val = 2031;
        } else if (checkBoxCSatPID.isChecked()) {
            val = 2032;
        } else if (checkBoxCFFdx.isChecked()) {
            val = 2040;
        } else if (checkBoxCFFdy.isChecked()) {
            val = 2041;
        }

        return val;
    }

    private EditText getEditText() {
        EditText textValue = null;
        if (checkBoxPx.isChecked()) {
            textValue = findViewById(R.id.editTextPx);
        } else if (checkBoxIx.isChecked()) {
            textValue = findViewById(R.id.editTextIx);
        } else if (checkBoxDx.isChecked()) {
            textValue = findViewById(R.id.editTextDx);
        } else if (checkBoxPy.isChecked()) {
            textValue = findViewById(R.id.editTextPy);
        } else if (checkBoxIy.isChecked()) {
            textValue = findViewById(R.id.editTextIy);
        } else if (checkBoxDy.isChecked()) {
            textValue = findViewById(R.id.editTextDy);
        } else if (checkBoxPz.isChecked()) {
            textValue = findViewById(R.id.editTextPz);
        } else if (checkBoxIz.isChecked()) {
            textValue = findViewById(R.id.editTextIz);
        } else if (checkBoxFFx.isChecked()) {
            textValue = findViewById(R.id.editTextFFx);
        } else if (checkBoxFFy.isChecked()) {
            textValue = findViewById(R.id.editTextFFy);
        } else if (checkBoxFFdx.isChecked()) {
            textValue = findViewById(R.id.editTextFFdx);
        } else if (checkBoxFFdy.isChecked()) {
            textValue = findViewById(R.id.editTextFFdy);
        } else if (checkBoxSatI.isChecked()) {
            textValue = findViewById(R.id.editTextSatI);
        } else if (checkBoxSatPID.isChecked()) {
            textValue = findViewById(R.id.editTextSatPID);
        } else if (checkBoxDTermC.isChecked()) {
            textValue = findViewById(R.id.editTextDTermC);
        } else if (checkBoxCPx.isChecked()) {
            textValue = findViewById(R.id.editTextCPx);
        } else if (checkBoxCIx.isChecked()) {
            textValue = findViewById(R.id.editTextCIx);
        } else if (checkBoxCPy.isChecked()) {
            textValue = findViewById(R.id.editTextCPy);
        } else if (checkBoxCIy.isChecked()) {
            textValue = findViewById(R.id.editTextCIy);
        } else if (checkBoxCSatI.isChecked()) {
            textValue = findViewById(R.id.editTextCSatI);
        } else if (checkBoxCSatPID.isChecked()) {
            textValue = findViewById(R.id.editTextCSatPID);
        } else if (checkBoxCFFdx.isChecked()) {
            textValue = findViewById(R.id.editTextCFFdx);
        } else if (checkBoxCFFdy.isChecked()) {
            textValue = findViewById(R.id.editTextCFFdy);
        }

        return textValue;
    }

    private void uncheckExpect(CheckBox chckBox)
    {
        if(chckBox.isChecked())
        {
            for (CheckBox checkBox : checkBoxes)
            {
                if(checkBox != chckBox)
                {
                    checkBox.setChecked(false);
                }
            }
        }
    }
}