package com.example.d4djv3;

import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Map;

public class BtRxThread extends Thread {
    InputStream btInStream_RxThr;
    boolean cancelRx_RxThr;
    boolean isRxStopped_RxThr = false;
    String paramData_RxThr = "";
    boolean newParamReceived_RxThr = false;
    String streamData1_RxThr = "";
    String streamData2_RxThr = "";
    String streamData3_RxThr = "";
    String streamData4_RxThr = "";
    String streamData5_RxThr = "";
    int streamDataID1_RxThr = 0;
    int streamDataID2_RxThr = 0;
    int streamDataID3_RxThr = 0;
    int streamDataID4_RxThr = 0;
    int streamDataID5_RxThr = 0;

    private final byte threadDelay = 10;

    // Create a Map to store BT data
    private static Map<String, Object> BTdata = new HashMap<>();

    // Add a public method that allows other classes to
    // access the data
    public static Map<String, Object> getBTData()
    {
        return BTdata;
    }

    public BtRxThread(InputStream InStream) {
        super("Thread BT rx");
        Log.d("BT", "BtRxThread - Rx thread constructor called");
        this.btInStream_RxThr = InStream;
    }

    public void run()
    {
        Log.d("BT", "BtRxThread - Rx thread run called");
        while (true) {
//            Log.d("BT", "BtRxThread - Rx thread running");
            Map<String, Object> mainData = MainActivity.getData();
            cancelRx_RxThr = (boolean)mainData.get("cancelRx");
            //stop looping if disconnected
            if (cancelRx_RxThr)
            {
                Log.d("BT", "BtRxThread - breaking from RX loop");
                isRxStopped_RxThr = true;
                //add rx stop into map
                BTdata.put("isRxStopped", isRxStopped_RxThr);
                //reset rx stopped flag to default
                isRxStopped_RxThr = false;
                //10ms delay of rx thread
                try {
                    Thread.sleep(threadDelay);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
                break;
            }

            //if inStream is not null
            if (null != btInStream_RxThr)
            {
                try
                {
                    //if data available
                    int numberOfBytesAvailable = btInStream_RxThr.available();
                    if (numberOfBytesAvailable > 0)
                    {
                        int numberOfBytesToRead = btInStream_RxThr.read();

//                        Log.d("BT", "BtRxThread - numberOfBytesToRead= " + numberOfBytesToRead);
//                        Log.d("BT", "BtRxThread - numberOfBytesAvailable= " + numberOfBytesAvailable);

                        //valid number to receive
                        if (0 < numberOfBytesToRead && numberOfBytesToRead < 255)
                        {
                            byte[] data = new byte[numberOfBytesToRead];
                            int numberOfBytesActuallyReceived = 0;

                            if(numberOfBytesToRead <= numberOfBytesAvailable)
                            {
                                //read the bytes
                                numberOfBytesActuallyReceived = btInStream_RxThr.read(data, 0, numberOfBytesToRead);
                                //Log.d("BT", "BtRxThread - numberOfBytesActuallyReceived= " + numberOfBytesActuallyReceived);
                            }
                            else
                            {
                                //send default map data
                                setMapData();

                                //read available amount of bytes-1
                                numberOfBytesActuallyReceived = btInStream_RxThr.read(data, 0, numberOfBytesAvailable-1);
                                Log.d("BT", "BtRxThread");
                                Log.d("BT", "BtRxThread - numberOfBytesActuallyReceived= " + numberOfBytesActuallyReceived);
                                Log.d("BT", "BtRxThread - numberOfBytesToRead=           " + numberOfBytesToRead);
                                Log.d("BT", "BtRxThread - numberOfBytesAvailable=        " + numberOfBytesAvailable);

                                int numberOfBytesAvailable2 = 0;
                                do
                                {
                                    numberOfBytesAvailable2 = btInStream_RxThr.available();
                                    Log.d("BT", "BtRxThread - 10ms delay for extra BT data");

                                    //delay for new data
                                    try {
                                        Thread.sleep(100);
                                    } catch (InterruptedException e) {
                                        Log.d("BT", "BtRxThread - rx stream 100ms waited failed");
                                        throw new RuntimeException(e);
                                    }
                                }
                                while((numberOfBytesToRead-numberOfBytesAvailable+1) > numberOfBytesAvailable2);

                                Log.d("BT", "BtRxThread - numberOfBytesAvailable2=       " + numberOfBytesAvailable2);
                                //read rest of the bytes
                                int numberOfRemainingBytesActuallyReceived = btInStream_RxThr.read(data, numberOfBytesAvailable-1, numberOfBytesToRead-numberOfBytesAvailable+1);
//                                Log.d("BT", "BtRxThread - after 1ms remaining received bytes= " + numberOfRemainingBytesActuallyReceived);
                                numberOfBytesActuallyReceived += numberOfRemainingBytesActuallyReceived;
//                                Log.d("BT", "BtRxThread - after 1ms numberOfBytesActuallyReceived= " + numberOfBytesActuallyReceived);

                            }

                            //if correct number of bytes received
                            if (numberOfBytesToRead == numberOfBytesActuallyReceived)
                            {
                                //debug the bytes
//                                for (byte iter = 0; iter < numberOfBytesToRead; iter++) {
//                                    Log.d("BT", "BtRxThread - datalength[" + iter + "]: " + data[iter]);
//                                }

                                //if $ start and the last 2 is \r\n then process data further
                                if ('$' == (char) data[0] && '\r' == (char) data[numberOfBytesToRead - 2]
                                        && '\n' == (char) data[numberOfBytesToRead - 1])
                                {
                                    //Log.d("BT", "BtRxThread - $ start and /r/n end found");

                                    //look for S and P if there is any in the data
                                    int indexOfS = findLetter(data, (byte) ('S'));
                                    int indexOfP = findLetter(data, (byte) ('P'));
                                    int indexOfEnd = numberOfBytesToRead - 2;   //valid data always ends with \r\n
//                                    Log.d("BT", "BtRxThread - indexes: S - " + indexOfS + ", P - " + indexOfP + ", /r/n - " + indexOfEnd);

                                    //receive data stream if there is any
                                    receiveDataStream(data, indexOfS, indexOfP, indexOfEnd);

                                    //receive parameter if there is any
                                    receiveParam(data, indexOfP, indexOfEnd);

                                    //if UARTrecov messega is received
                                    //checkForUARTrecv(data, indexOfS, indexOfP, f_activity)

                                }
                                else
                                {
                                    Log.d("BT", "BtRxThread - $ and /r/n missalignment");
                                    do
                                    {
                                        if(btInStream_RxThr.available() <= 0)
                                        {
                                            //delay for new data
                                            try {
                                                Thread.sleep(10);
                                            } catch (InterruptedException e) {
                                                Log.d("BT", "BtRxThread - 10ms waited failed");
                                                throw new RuntimeException(e);
                                            }
                                        }
                                    }
                                    while('\n' != (char)btInStream_RxThr.read());
                                }
                            }
                        }
                    }
                    else
                    {
                        //btInStream_RxThr.readAllBytes();
                    }
                } catch (IOException e) {
                    Log.d("BT", "BtRxThread - thread rx failed");
                    e.printStackTrace();
                    throw new RuntimeException(e);
                }
            }

            // Add data to the Map
            setMapData();

            //10ms delay of rx thread
            try {
                Thread.sleep(threadDelay);
            } catch (InterruptedException e) {
                Log.d("BT", "BtRxThread - thread sleep failed");
                throw new RuntimeException(e);
            }
        }
    }

    void setMapData()
    {
        //stream data
        BTdata.put("streamDataID1", streamDataID1_RxThr);
        BTdata.put("streamData1", streamData1_RxThr);
        BTdata.put("streamDataID2", streamDataID2_RxThr);
        BTdata.put("streamData2", streamData2_RxThr);
        BTdata.put("streamDataID3", streamDataID3_RxThr);
        BTdata.put("streamData3", streamData3_RxThr);
        BTdata.put("streamDataID4", streamDataID4_RxThr);
        BTdata.put("streamData4", streamData4_RxThr);
        BTdata.put("streamDataID5", streamDataID5_RxThr);
        BTdata.put("streamData5", streamData5_RxThr);
        //reset ID to default
        streamDataID1_RxThr = 0;
        streamDataID2_RxThr = 0;
        streamDataID3_RxThr = 0;
        streamDataID4_RxThr = 0;
        streamDataID5_RxThr = 0;

        //param data
        BTdata.put("newParamReceived", newParamReceived_RxThr);
        BTdata.put("paramData", paramData_RxThr);
        newParamReceived_RxThr = false; //reset flag to default

        isRxStopped_RxThr = false;
        BTdata.put("isRxStopped", isRxStopped_RxThr);

    }

    void receiveParam(byte[] data, int indexP, int indexEnd) {
        //if indexP is valid read parameter value
        if (indexP > -1)
        {
            Log.d("BT", "BtRxThread - index of P is valid");
            int paramLength = indexEnd - indexP;
//            Log.d("BT", "BtRxThread - paramLength= " + (paramLength-1));
            byte[] paramBytes = new byte[paramLength];
            int iter = 0;
            for (int i = indexP + 1; i < indexEnd; i++) {
//                Log.d("BT", "BtRxThread - data[i]= " + data[i]);
                paramBytes[iter++] = data[i];
            }
            //save the float value into paramData
            paramData_RxThr = new String(paramBytes);
            //removing random '?' ending
            paramData_RxThr = paramData_RxThr.substring(0, paramData_RxThr.length() - 1);
            Log.d("BT", "BtRxThread - paramData= " + paramData_RxThr);
            //signal that new paramData was received
            newParamReceived_RxThr = true;

        }
    }

    void receiveDataStream(byte[] data, int indexS, int indexP, int indexEnd)
    {
        int indexOfMarker = findLetter(data, (byte) ('!'));
        //if indexS is valid read stream data(s)
        if(indexS > -1 && indexOfMarker > -1)
        {
//            Log.d("BT", "BtRxThread - index of S and '!' is valid");
            int indexOfStart = indexS;
            int steamDataLength = 0;
            int iter = 0;

            //1st stream value
            streamDataID1_RxThr = (int)data[indexOfStart+1] & 0xFF;
            streamDataID1_RxThr += ((int)data[indexOfStart+2] & 0xFF) << 8;
            steamDataLength = indexOfMarker - 1 - indexOfStart - 2 ;
            byte[] streamBytes1 = new byte[steamDataLength];
            iter = 0;
            for (int i = indexOfStart + 3; i < indexOfMarker; i++)
            {
                streamBytes1[iter++] = data[i];
            }
            streamData1_RxThr = new String(streamBytes1);
            //Log.d("BT", "BtRxThread - receiveDataStream - streamDataID1: " + streamDataID1_RxThr);
            //Log.d("BT", "BtRxThread - receiveDataStream - streamData1: " + streamData1_RxThr);
            if(indexOfMarker+1 == indexEnd || indexOfMarker+1 == indexP) return;

            //2nd stream value
            indexOfStart = indexOfMarker;
            indexOfMarker = findLetter(data, (byte) ('!'), (byte)(indexOfStart+1));
            streamDataID2_RxThr = (int)data[indexOfStart+1] & 0xFF;
            streamDataID2_RxThr += ((int)data[indexOfStart+2] & 0xFF) << 8;
            steamDataLength = indexOfMarker - 1 - indexOfStart - 2 ;
            byte[] streamBytes2 = new byte[steamDataLength];
            iter = 0;
            for (int i = indexOfStart + 3; i < indexOfMarker; i++)
            {
                streamBytes2[iter++] = data[i];
            }
            streamData2_RxThr = new String(streamBytes2);
            if(indexOfMarker+1 == indexEnd || indexOfMarker+1 == indexP) return;

            //3rd stream value
            indexOfStart = indexOfMarker;
            indexOfMarker = findLetter(data, (byte) ('!'), (byte)(indexOfStart+1));
            streamDataID3_RxThr = (int)data[indexOfStart+1] & 0xFF;
            streamDataID3_RxThr += ((int)data[indexOfStart+2] & 0xFF) << 8;
            steamDataLength = indexOfMarker - 1 - indexOfStart - 2 ;
            byte[] streamBytes3 = new byte[steamDataLength];
            iter = 0;
            for (int i = indexOfStart + 3; i < indexOfMarker; i++)
            {
                streamBytes3[iter++] = data[i];
            }
            streamData3_RxThr = new String(streamBytes3);
            if(indexOfMarker+1 == indexEnd || indexOfMarker+1 == indexP) return;

            //4th stream value
            indexOfStart = indexOfMarker;
            indexOfMarker = findLetter(data, (byte) ('!'), (byte)(indexOfStart+1));
            streamDataID4_RxThr = (int)data[indexOfStart+1] & 0xFF;
            streamDataID4_RxThr += ((int)data[indexOfStart+2] & 0xFF) << 8;
            steamDataLength = indexOfMarker - 1 - indexOfStart - 2 ;
            byte[] streamBytes4 = new byte[steamDataLength];
            iter = 0;
            for (int i = indexOfStart + 3; i < indexOfMarker; i++)
            {
                streamBytes4[iter++] = data[i];
            }
            streamData4_RxThr = new String(streamBytes4);
            if(indexOfMarker+1 == indexEnd || indexOfMarker+1 == indexP) return;

            //5th stream value
            indexOfStart = indexOfMarker;
            indexOfMarker = findLetter(data, (byte) ('!'), (byte)(indexOfStart+1));
            streamDataID5_RxThr = (int)data[indexOfStart+1] & 0xFF;
            streamDataID5_RxThr += ((int)data[indexOfStart+2] & 0xFF) << 8;
            steamDataLength = indexOfMarker - 1 - indexOfStart - 2 ;
            byte[] streamBytes5 = new byte[steamDataLength];
            iter = 0;
            for (int i = indexOfStart + 3; i < indexOfMarker; i++)
            {
                streamBytes5[iter++] = data[i];
            }
            streamData5_RxThr = new String(streamBytes5);
        }
    }

    int findLetter(byte[] bytes, byte letter)
    {
        int position = -1;
        for (int iter = 0; iter < bytes.length; iter++)
        {
            if (letter == bytes[iter]) {
                position = iter;
                break;
            }
        }

        return position;
    }

    int findLetter(byte[] bytes, byte letter, byte startIndex)
    {
        int position = -1;
        for (int iter = startIndex; iter < bytes.length; iter++)
        {
            if (letter == bytes[iter]) {
                position = iter;
                break;
            }
        }

        return position;
    }
}
