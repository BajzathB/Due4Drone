package com.example.d4djv3;

import org.junit.Test;

import static org.junit.Assert.*;

import java.io.IOException;
import java.io.InputStream;

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */

public class BtRxThreadUT {
    @Test
    public void test_RxStreamData()
    {
        InputStream testInput = new InputStream() {
            @Override
            public int read() throws IOException {
                return 0;
            }
        };
        BtRxThread testClass = new BtRxThread(testInput);
        byte[] data = new byte[60];
        int indexS = 0;
        int indexP = 0;
        int indexEnd = 0;

        //1st: no stream data
        data[0] = '$';
        data[1] = '1';
        data[2] = '2';
        data[3] = '3';
        indexP = -1;
        indexEnd = 0;
        testClass.receiveDataStream(data, indexS, indexP, indexEnd);
        assertEquals(testClass.streamData1_RxThr, "");

        //2nd: 2 integer stream data
        data[0] = '$';
        data[1] = 'S';
        data[2] = (byte)0xB8;
        data[3] = (byte)0x0B;
        data[4] = '1';
        data[5] = '1';
        data[6] = '!';
        data[7] = '\r';
        data[8] = '\n';
        indexS = 1;
        indexP = -1;
        indexEnd = 7;
        testClass.receiveDataStream(data, indexS, indexP, indexEnd);
        assertEquals(testClass.streamDataID1_RxThr, 3000);
        assertEquals(testClass.streamData1_RxThr, "11");

        //3rd: 4 integer stream data
        data[0] = '$';
        data[1] = 'S';
        data[2] = (byte)0xB9;
        data[3] = (byte)0x0B;
        data[4] = '1';
        data[5] = '2';
        data[6] = '3';
        data[7] = '4';
        data[8] = '!';
        data[9] = '\r';
        data[10] = '\n';
        indexP = -1;
        indexEnd = 9;
        testClass.receiveDataStream(data, indexS, indexP, indexEnd);
        assertEquals(testClass.streamDataID1_RxThr, 3001);
        assertEquals(testClass.streamData1_RxThr, "1234");

        //4th: float stream data
        data[0] = '$';
        data[1] = 'S';
        data[2] = (byte)0xBA;
        data[3] = (byte)0x0B;
        data[4] = '1';
        data[5] = '.';
        data[6] = '2';
        data[7] = '3';
        data[8] = '4';
        data[9] = '!';
        data[10] = '\r';
        data[11] = '\n';
        indexP = -1;
        indexEnd = 10;
        testClass.receiveDataStream(data, indexS, indexP, indexEnd);
        assertEquals(testClass.streamDataID1_RxThr, 3002);
        assertEquals(testClass.streamData1_RxThr, "1.234");

        //4th: 2 int-int stream data
        data[0] = '$';
        data[1] = 'S';
        data[2] = (byte)0xB8;
        data[3] = (byte)0x0B;
        data[4] = '1';
        data[5] = '!';
        data[6] = (byte)0xB9;
        data[7] = (byte)0x0B;
        data[8] = '2';
        data[9] = '3';
        data[10] = '4';
        data[11] = '!';
        data[12] = '\r';
        data[13] = '\n';
        indexP = -1;
        indexEnd = 12;
        testClass.receiveDataStream(data, indexS, indexP, indexEnd);
        assertEquals(testClass.streamDataID1_RxThr, 3000);
        assertEquals(testClass.streamData1_RxThr, "1");
        assertEquals(testClass.streamDataID2_RxThr, 3001);
        assertEquals(testClass.streamData2_RxThr, "234");

        //5th: 3 float stream data
        data[0] = '$';
        data[1] = 'S';
        data[2] = (byte)0xB8;
        data[3] = (byte)0x0B;
        data[4] = '1';
        data[5] = '.';
        data[6] = '2';
        data[7] = '3';
        data[8] = '4';
        data[9] = '!';
        data[10] = (byte)0xB9;
        data[11] = (byte)0x0B;
        data[12] = '5';
        data[13] = '.';
        data[14] = '6';
        data[15] = '7';
        data[16] = '8';
        data[17] = '!';
        data[18] = (byte)0xBA;
        data[19] = (byte)0x0B;
        data[20] = '9';
        data[21] = '.';
        data[22] = '8';
        data[23] = '7';
        data[24] = '6';
        data[25] = '!';
        data[26] = '\r';
        data[27] = '\n';
        indexP = -1;
        indexEnd = 26;
        testClass.receiveDataStream(data, indexS, indexP, indexEnd);
        assertEquals(testClass.streamDataID1_RxThr, 3000);
        assertEquals(testClass.streamData1_RxThr, "1.234");
        assertEquals(testClass.streamDataID2_RxThr, 3001);
        assertEquals(testClass.streamData2_RxThr, "5.678");
        assertEquals(testClass.streamDataID3_RxThr, 3002);
        assertEquals(testClass.streamData3_RxThr, "9.876");

        //6th: 5 float stream data + param data
        data[0] = '$';
        data[1] = 'S';
        data[2] = (byte)0xB8;
        data[3] = (byte)0x0B;
        data[4] = '1';
        data[5] = '.';
        data[6] = '2';
        data[7] = '3';
        data[8] = '4';
        data[9] = '!';
        data[10] = (byte)0xB9;
        data[11] = (byte)0x0B;
        data[12] = '5';
        data[13] = '.';
        data[14] = '6';
        data[15] = '7';
        data[16] = '8';
        data[17] = '!';
        data[18] = (byte)0xBA;
        data[19] = (byte)0x0B;
        data[20] = '9';
        data[21] = '.';
        data[22] = '8';
        data[23] = '7';
        data[24] = '6';
        data[25] = '!';
        data[26] = (byte)0xBB;
        data[27] = (byte)0x0B;
        data[28] = '5';
        data[29] = '.';
        data[30] = '4';
        data[31] = '3';
        data[32] = '2';
        data[33] = '!';
        data[34] = (byte)0xBC;
        data[35] = (byte)0x0B;
        data[36] = '-';
        data[37] = '4';
        data[38] = '.';
        data[39] = '7';
        data[40] = '3';
        data[41] = '1';
        data[42] = '!';
        data[43] = 'P';
        data[44] = '1';
        data[45] = '.';
        data[46] = '6';
        data[47] = '7';
        data[48] = '9';
        data[49] = '\r';
        data[50] = '\n';
        indexP = 43;
        indexEnd = 49;
        testClass.receiveDataStream(data, indexS, indexP, indexEnd);
        testClass.receiveParam(data,indexP,indexEnd);
        assertEquals(testClass.streamDataID1_RxThr, 3000);
        assertEquals(testClass.streamData1_RxThr, "1.234");
        assertEquals(testClass.streamDataID2_RxThr, 3001);
        assertEquals(testClass.streamData2_RxThr, "5.678");
        assertEquals(testClass.streamDataID3_RxThr, 3002);
        assertEquals(testClass.streamData3_RxThr, "9.876");
        assertEquals(testClass.streamDataID4_RxThr, 3003);
        assertEquals(testClass.streamData4_RxThr, "5.432");
        assertEquals(testClass.streamDataID5_RxThr, 3004);
        assertEquals(testClass.streamData5_RxThr, "-4.731");
        assertEquals(testClass.paramData_RxThr, "1.679");
        assertEquals(testClass.newParamReceived_RxThr, true);
    }
}