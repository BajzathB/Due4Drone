package com.example.d4djv3;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

import androidx.activity.EdgeToEdge;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import java.util.Arrays;
import java.util.List;

public class CascadeActivity  extends BaseActivity {

    TextView textViewPlusMinusValue;

    CheckBox checkBoxCPx;
    CheckBox checkBoxCIx;
    CheckBox checkBoxCPy;
    CheckBox checkBoxCIy;
    CheckBox checkBoxCSatI;
    CheckBox checkBoxCSatPID;
    CheckBox checkBoxCFFdx;
    CheckBox checkBoxCFFdy;

    List<CheckBox> checkBoxes;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_cascade);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.cascade), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });
        Log.d("BT", "CascadeActivty - onCreate - called");

        textViewPlusMinusValue = findViewById(R.id.textViewPlusMinusVal);

        Button buttonGet = findViewById(R.id.buttonGet);
        Button buttonSet = findViewById(R.id.buttonSet);
        Button buttonInc = findViewById(R.id.buttonInc);
        Button buttonDec = findViewById(R.id.buttonDec);

        checkBoxCPx = findViewById(R.id.checkBoxCPx);
        checkBoxCIx = findViewById(R.id.checkBoxCIx);
        checkBoxCPy = findViewById(R.id.checkBoxCPy);
        checkBoxCIy = findViewById(R.id.checkBoxCIy);
        checkBoxCSatI = findViewById(R.id.checkBoxCSatI);
        checkBoxCSatPID = findViewById(R.id.checkBoxCSatPID);
        checkBoxCFFdx = findViewById(R.id.checkBoxCFFdx);
        checkBoxCFFdy = findViewById(R.id.checkBoxCFFdy);

        checkBoxes = Arrays.asList(checkBoxCPx, checkBoxCIx, checkBoxCPy,
                checkBoxCIy, checkBoxCSatI, checkBoxCSatPID,
                checkBoxCFFdx, checkBoxCFFdy);

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
            float addVal = Float.parseFloat(add.substring(1));

            BTSocket.getInstance().AddVal(text, addVal);
        });
        buttonDec.setOnClickListener(v ->
        {
            EditText text = getEditText();
            String add = textViewPlusMinusValue.getText().toString();
            float addVal = Float.parseFloat(add.substring(1));

            BTSocket.getInstance().AddVal(text, -addVal);
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

    }
    @Override
    protected void onResume() {
        Log.d("BT", "MainActivty - onResume");
        super.onResume();
        BTSocket.getInstance().setBTcmdResultTextID(findViewById(R.id.textBTcmd));
        //todo reconnect bt on resume

    }

    @Override
    protected void onSwipeLeft() {
        Log.d("BT", "CascadeActivty - onTouchEvent - swiped left");
        // returns to previous activity
        finish();
    }

    @Override
    protected void onSwipeRight() {
        Log.d("BT", "CascadeActivty - onTouchEvent - swiped right");
        Intent intent = new Intent(CascadeActivity.this, StreamAccActivity.class);
        startActivity(intent);
    }

    private int getID() {
        int val = 0;
        if (checkBoxCPx.isChecked()) {
            val = 1021;
        } else if (checkBoxCIx.isChecked()) {
            val = 1022;
        } else if (checkBoxCPy.isChecked()) {
            val = 1024;
        } else if (checkBoxCIy.isChecked()) {
            val = 1025;
        } else if (checkBoxCSatI.isChecked()) {
            val = 1030;
        } else if (checkBoxCSatPID.isChecked()) {
            val = 1031;
        } else if (checkBoxCFFdx.isChecked()) {
            val = 1032;
        } else if (checkBoxCFFdy.isChecked()) {
            val = 1033;
        }

        return val;
    }
    private EditText getEditText() {
        EditText textValue = null;
        if (checkBoxCPx.isChecked()) {
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
