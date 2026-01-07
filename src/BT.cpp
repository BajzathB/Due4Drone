// UART communication via the bluetooth module
//
//Bluetooth Rx - PA12 A peri - USART1 RXD1
//Bluetooth Tx - PA13 A peri - USART1 TXD1
//protocol structure
//TX:
//number of bytes to send
//signaling start: $
//streaming data: starting with S
//                2byte id + data1 + '!'
//                2byte id + data2 + '!'
//                2byte id + data2 + '!'
//sending 1 param value only one time: starting with P
//                                     data
//finishing data send with \r\n
//example: 0A--50-35-35-2E-30-30-30-0D-0A
//
//RX:
//number of bytes to receive afterward
//set(0b11)/get(0b1100)/stream(0b110000)
//identifier 2byte, 16bit data in split into 2 byte, 1-99 bool, 100-999 uint8, 1000-1999 uint16, 2000-2999 float
//	if get: not further bytes needed to receive, only send back data
//	if set: further bytes needed, the value to set in asci format
//  if stream: no further bytes needed ro receive, only stream back data
//\r\n frame ending
//example in hex: 08-03-00-64-aa-bb-cc-0D-0A
//test: 05-0C-D0-07-0D-0A

//AT settings:
//EN pin to be 5V and button pushed on poweron to enter AT mode
//460800baud 1stop 0pairty


#include "pch.h"
#include "BT.h"
#include "BT_IDs.h"
#include "sysTime.h"
#include "MotorControl.h"
#include "SPI.h"
#include "SPI_SDcard.h"

#ifdef UNIT_TEST

#include "../test/helper/support4Testing.h"
#include "../test/helper/support4Testing.hpp"

extern DummySerial SerialUSB;
extern DummySerial Serial2;
extern Pio* PIOA;
extern Usart* USART1;
extern spi_st SPI;
#else

#include "arduino.h"
#include "variant.h"

#endif

bt BT;

void SetupBT(void)
{
	PIOA->PIO_WPSR = 0x50494F00; //disable write protection mode on parallel io line A
	PIOA->PIO_PDR |= PIO_PDR_P12 | PIO_PDR_P13; //enable peripheral control on PA12-PA13
	PIOA->PIO_ABSR &= ~PIO_ABSR_P12 & ~PIO_ABSR_P13;  //select peripheral A by inverse of B on PA12-PA13
	PIOA->PIO_WPSR = 0x50494F01; //reenable write protection mode on parallel io line A

	pmc_enable_periph_clk(ID_USART1);

	//software reset TX-RX
	USART1->US_CR |= US_CR_RSTTX | US_CR_RSTRX;

	USART1->US_WPMR = 0x55534100; //disable write protection on usart
	USART1->US_MR |= US_MR_USART_MODE_NORMAL //set normal mode
		| US_MR_USCLKS_MCK  //master clock selected
		| US_MR_CHRL_8_BIT  //8 bit data
		| US_MR_PAR_NO  //no parity bit
		| US_MR_NBSTOP_1_BIT  //1 stop bit
		| US_MR_OVER;  // 8x oversampling for achiving minimal error between actual and expected baud rate

	//calculated baud rate is 461538,5  error is 0,16%
	USART1->US_BRGR = US_BRGR_CD(22) | US_BRGR_FP(6);
	USART1->US_WPMR = 0x55534101; //enable write protection on usart
	
	NVIC_SetPriority(USART1_IRQn, 5);
	NVIC_EnableIRQ(USART1_IRQn);

	//RX
	//start looking for 1 byte
	USART1->US_CR |= US_CR_RXEN;//enable receive
	BT.input.ctr = 1;
	USART1->US_RPR = (uint32_t)BT.input.vector;
	USART1->US_RCR = BT.input.ctr;
	USART1->US_PTCR |= US_PTCR_RXTEN; //start pdc receive
	USART1->US_IER |= US_IER_ENDRX; //enable endrx interrupt

	//TX
	//set tx next counter to 1 to trigger correct statuses of TX
	//USART1->US_TCR = 1;
	//load 1st element with $ signal, (2nd element will be the data length)
	BT.output.vector[1] = '$';

}

void RunBT(const controllerIn_st* ctrlIn, const controllerOut_st* ctrlOut)
{

	// uint8_t diff = ID_pitch_PT2 - ID_bitshift_substracter;
	// uint64_t shift = uint64_t(1) << (ID_pitch_PT2 - ID_bitshift_substracter);
	// uint64_t strFlags = shift;
	// uint64_t flag = (strFlags & (uint64_t(1) << (ID_pitch_PT2 - ID_bitshift_substracter))) > 0;

	//SerialUSB.print(NoBReceviedCtr); SerialUSB.print("\t");
	// SerialUSB.print(diff); SerialUSB.print("\t");
	// SerialUSB.print(uint32_t(shift >> 32));
	// SerialUSB.print(uint32_t(strFlags >> 32));
	// SerialUSB.println(uint32_t(flag >> 32));

	BTReceive();
	
	BTTransmit(ctrlIn,ctrlOut);


	// if (SerialUSB.available() > 0)
	// {
	// 	uint8_t incomingByte = SerialUSB.read();


	// 	//BT.input.ctr = 1;
	// 	//USART1->US_RPR = (uint32_t)BT.input.vector;
	// 	//USART1->US_RCR = BT.input.ctr;
	// 	//USART1->US_PTCR |= US_PTCR_RXTEN; //start pdc receive
	// }
}

void USART1_Handler(void)
{
	uint32_t usart_status = USART1->US_CSR;
	//SerialUSB.println(usart_status);

	if (usart_status & US_CSR_ENDRX)
	{
		if (WAITING_FOR_NUMBEROFBYTES == BT.rxDataState)
		{
			BT.input.ctr = BT.input.vector[0];
			USART1->US_RPR = (uint32_t)BT.input.vector;
			USART1->US_RCR = BT.input.ctr;
			USART1->US_PTCR |= US_PTCR_RXTEN; //start pdc receive

			BT.rxDataState = RECEVING_DATA_BYTES;
		}
		else if (RECEVING_DATA_BYTES == BT.rxDataState)
		{
			//enable receive
			USART1->US_CR |= US_CR_RXDIS;
			BT.rxDataState = FRAME_RECEIVED;
			USART1->US_RCR = 1; //counter set to 1 to unset interrupt flag
		}
		
	}
}

void BTReceive(void)
{
	if (FRAME_RECEIVED == BT.rxDataState)
	{
		ProcessRxFrame();

		//setup new receive
		USART1->US_RPR = (uint32_t)BT.input.vector;
		USART1->US_RCR = 1;
		BT.rxDataState = WAITING_FOR_NUMBEROFBYTES;
		USART1->US_CR |= US_CR_RXEN;

//		SerialUSB.print("received: "); SerialUSB.write(BT.input.ctr); SerialUSB.print(" / "); SerialUSB.print(BT.input.ctr); SerialUSB.print(" byte - ");
//		for (uint8_t i = 0; i < BT.input.ctr; i++)
//		{
//			SerialUSB.print(BT.input.vector[i]);
//		}
//
//		SerialUSB.print("rxFrame: cmd - "); SerialUSB.print(BT.rxFrame.cmd);
//		SerialUSB.print(" ,id - "); SerialUSB.print(BT.rxFrame.id);
//		//SerialUSB.print(" ,pidRatePx - "); SerialUSB.print(getPIDrates()->P.x);
//		SerialUSB.print(" ,paramData - "); SerialUSB.print(BT.txFrame.paramData);
//		SerialUSB.print(" ,sendParam - "); SerialUSB.print(BT.txFrame.sendParam);
//
//		SerialUSB.println();

	}
}

void BTTransmit(const controllerIn_st* ctrlIn, const controllerOut_st* ctrlOut)
{
	bool triggerTx{ false };
	BT.output.ctr = 2;

    //only enter if at least 10ms elapsed since previous data send
    if (BT.txDeltaT > 0.011f)
    {
        //if there is data to stream
        if (BT.txFrame.streamDataFlags > 0 || BT.txFrame.streamDataFlags2 > 0)
        {
            pid_st* pidRate{ getPIDrates() };
            gyroData_st* gyroData{ getGyroData()};
			accData_st* accData{ getAccData() };
            MotorSpeeds motorSpeeds;
            getMotorSpeeds(&motorSpeeds);

            BT.output.vector[BT.output.ctr++] = 'S';

			//flagset1
            if ((BT.txFrame.streamDataFlags & (1 << (ID_pidRate_sensor_signal_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_pidRate_sensor_signal_X, pidRate->sensor.signal.x);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_pidRate_sensor_signal_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_pidRate_sensor_signal_Y, pidRate->sensor.signal.y);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_pidRate_sensor_signal_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_pidRate_sensor_signal_Z, pidRate->sensor.signal.z);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_signal_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_signal_X, ctrlIn->gyro.signal.x);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_signal_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_signal_Y, ctrlIn->gyro.signal.y);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_signal_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_signal_Z, ctrlIn->gyro.signal.z);

            if ((BT.txFrame.streamDataFlags & (1 << (ID_motorCommand_FL - ID_bitshift_substracter))) > 0) SetStreamData(ID_motorCommand_FL, motorSpeeds.FL);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_motorCommand_FR - ID_bitshift_substracter))) > 0) SetStreamData(ID_motorCommand_FR, motorSpeeds.FR);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_motorCommand_RL - ID_bitshift_substracter))) > 0) SetStreamData(ID_motorCommand_RL, motorSpeeds.RL);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_motorCommand_RR - ID_bitshift_substracter))) > 0) SetStreamData(ID_motorCommand_RR, motorSpeeds.RR);

            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT1_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT1_X, gyroData->PT1.signal.x);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT1_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT1_Y, gyroData->PT1.signal.y);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT1_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT1_Z, gyroData->PT1.signal.z);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT2_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT2_X, gyroData->PT2.signal.x);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT2_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT2_Y, gyroData->PT2.signal.y);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT2_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT2_Z, gyroData->PT2.signal.z);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_KALMAN_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_KALMAN_X, gyroData->KF.x.value);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_KALMAN_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_KALMAN_Y, gyroData->KF.y.value);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_KALMAN_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_KALMAN_Z, gyroData->KF.z.value);

			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_signal_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_signal_X, ctrlIn->acc.signal.x);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_signal_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_signal_Y, ctrlIn->acc.signal.y);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_signal_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_signal_Z, ctrlIn->acc.signal.z);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT1_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT1_X, accData->PT1.signal.x);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT1_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT1_Y, accData->PT1.signal.y);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT1_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT1_Z, accData->PT1.signal.z);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT2_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT2_X, accData->PT2.signal.x);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT2_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT2_Y, accData->PT2.signal.y);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT2_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT2_Z, accData->PT2.signal.z);

			if ((BT.txFrame.streamDataFlags & (1 << (ID_roll - ID_bitshift_substracter))) > 0) SetStreamData(ID_roll, accData->rollAngle);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_pitch - ID_bitshift_substracter))) > 0) SetStreamData(ID_pitch, accData->pitchAngle);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_roll_PT1 - ID_bitshift_substracter))) > 0) SetStreamData(ID_roll_PT1, accData->rollAnglePT1Acc);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_pitch_PT1 - ID_bitshift_substracter))) > 0) SetStreamData(ID_pitch_PT1, accData->pitchAnglePT1Acc);
			//flagset2
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_roll_PT2 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_roll_PT2, accData->rollAnglePT2Acc);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_pitch_PT2 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_pitch_PT2, accData->pitchAnglePT2Acc);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_roll_KF - ID_bitshift_substracter2))) > 0) SetStreamData(ID_roll_KF, accData->angleKF.roll.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_pitch_KF - ID_bitshift_substracter2))) > 0) SetStreamData(ID_pitch_KF, accData->angleKF.pitch.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_roll_KF_AccPT1 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_roll_KF_AccPT1, accData->angleKFPT10.roll.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_pitch_KF_AccPT1 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_pitch_KF_AccPT1, accData->angleKFPT10.pitch.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_roll_KF_AccPT2 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_roll_KF_AccPT2, accData->angleKFPT20.roll.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_pitch_KF_AccPT2 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_pitch_KF_AccPT2, accData->angleKFPT20.pitch.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_roll_KF_AccPT1_GyroPT1 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_roll_KF_AccPT1_GyroPT1, accData->angleKFPT11.roll.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_pitch_KF_AccPT1_GyroPT1 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_pitch_KF_AccPT1_GyroPT1, accData->angleKFPT11.pitch.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_roll_KF_AccPT2_GyroPT2 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_roll_KF_AccPT2_GyroPT2, accData->angleKFPT22.roll.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_pitch_KF_AccPT2_GyroPT2 - ID_bitshift_substracter2))) > 0) SetStreamData(ID_pitch_KF_AccPT2_GyroPT2, accData->angleKFPT22.pitch.angle);
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_loop_time_ms - ID_bitshift_substracter2))) > 0) SetStreamData(ID_loop_time_ms, ctrlIn->loopTime*1000.0f);

            //trigger tx
            triggerTx = true;
        }

        //if there is param to send
        if (true == BT.txFrame.sendParam)
        {
            BT.output.vector[BT.output.ctr++] = 'P';

            CalcCharAndFillOutput(BT.txFrame.paramData, BT.txFrame.numberOfFrac);

            //trigger tx
            triggerTx = true;
            //reset param send
            BT.txFrame.sendParam = false;
        }

        //fill number of bytes and trigger transmit
        if (true == triggerTx)
        {
            //ending of message
            BT.output.vector[BT.output.ctr++] = '\r';
            BT.output.vector[BT.output.ctr++] = '\n';

            //set number of data
            BT.output.vector[0] = BT.output.ctr - 1;

            //loading tx registers
            USART1->US_TPR = (uint32_t)BT.output.vector;
            USART1->US_TCR = BT.output.ctr;
            //enable uart tx
            USART1->US_CR |= US_CR_TXEN;
            //start pdc uart transmit
            USART1->US_PTCR |= US_PTCR_TXTEN;
            //reset trigger
            triggerTx = false;
            //reset deltaT
            BT.txDeltaT = 0.0f;
        }
    }
    else
    {
        BT.txDeltaT += getSysLoopTime();
    }
}

void ProcessRxFrame(void)
{
	if (isFrameEndReceived())
	{
		BT.rxFrame.cmd = E_command(BT.input.vector[0]);
		BT.rxFrame.id = (uint16_t(BT.input.vector[2]) << 8) | uint16_t(BT.input.vector[1]);

		if (CMD_SET == BT.rxFrame.cmd)
		{
			pid_st* pidRateSet{ getPIDrates() };
			pid_st* pidCascadseSet{ getPIDcascade() };
			gyroData_st* gyroDataSet{ getGyroData() };
			accData_st* accDataSet{ getAccData() };
			spi_st* spi{ getSPI() };
			Meas2Card* meas2card{ getMeas2Card() };

			switch (BT.rxFrame.id)
			{
				case ID_control_PID_rate_P_X: pidRateSet->P.x = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_I_X: pidRateSet->I.x = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_D_X: pidRateSet->D.x = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_P_Y: pidRateSet->P.y = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_I_Y: pidRateSet->I.y = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_D_Y: pidRateSet->D.y = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_P_Z: pidRateSet->P.z = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_I_Z: pidRateSet->I.z = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_D_Z: pidRateSet->D.z = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_FF_X: pidRateSet->FFr.x = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_FF_Y: pidRateSet->FFr.y = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_FF_DX: pidRateSet->FFdr.x = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_FF_DY: pidRateSet->FFdr.y = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_saturation_I: pidRateSet->saturationI = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_saturation_PID: pidRateSet->saturationPID = ConvertStrToFloat(&BT.input); break;
				case ID_control_PID_rate_Dterm_C: pidRateSet->DTermC = ConvertStrToFloat(&BT.input); break;
				case ID_gyro_filter_paramC: gyroDataSet->paramC = ConvertStrToFloat(&BT.input); break;
				case ID_gyro_kalman_filter_q: gyroDataSet->KF.q = ConvertStrToFloat(&BT.input);	break;
				case ID_gyro_kalman_filter_r: gyroDataSet->KF.r = ConvertStrToFloat(&BT.input);	break;
				case ID_acc_filter_paramC: accDataSet->paramC = ConvertStrToFloat(&BT.input); break;
				case ID_acc_kalman_filter_q_angle: accDataSet->q_angle = ConvertStrToFloat(&BT.input);	break;
				case ID_acc_kalman_filter_q_bias: accDataSet->q_bias = ConvertStrToFloat(&BT.input);	break;
				case ID_acc_kalman_filter_r: accDataSet->r_measure = ConvertStrToFloat(&BT.input);	break;
				case ID_spi_acc_offset_x: spi->acc.offset.x = ConvertStrToFloat(&BT.input);	break;
				case ID_spi_acc_offset_y: spi->acc.offset.y = ConvertStrToFloat(&BT.input);	break;
				case ID_spi_acc_offset_z: spi->acc.offset.z = ConvertStrToFloat(&BT.input);	break;
                case ID_control_PID_cascade_P_X: pidCascadseSet->P.x = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_I_X: pidCascadseSet->I.x = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_D_X: pidCascadseSet->D.x = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_P_Y: pidCascadseSet->P.y = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_I_Y: pidCascadseSet->I.y = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_D_Y: pidCascadseSet->D.y = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_P_Z: pidCascadseSet->P.z = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_I_Z: pidCascadseSet->I.z = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_D_Z: pidCascadseSet->D.z = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_saturation_I: pidCascadseSet->saturationI = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_saturation_PID: pidCascadseSet->saturationPID = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_Dterm_C: pidCascadseSet->DTermC = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_FF_X: pidCascadseSet->FFr.x = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_FF_Y: pidCascadseSet->FFr.y = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_FF_DX: pidCascadseSet->FFdr.x = ConvertStrToFloat(&BT.input); break;
                case ID_control_PID_cascade_FF_DY: pidCascadseSet->FFdr.y = ConvertStrToFloat(&BT.input); break;

				// meas 2 sdcard
				case ID_meas_2_card_sysTime: meas2card->measureSysTime = ConvertStrToBool(&BT.input); break;
                //gyro
				case ID_meas_2_card_gyro_raw_X: meas2card->measureGyroRawX = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_raw_Y: meas2card->measureGyroRawY = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_raw_Z: meas2card->measureGyroRawZ = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_X: meas2card->measureGyroPT1X = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_Y: meas2card->measureGyroPT1Y = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_Z: meas2card->measureGyroPT1Z = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT2_X: meas2card->measureGyroPT2X = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT2_Y: meas2card->measureGyroPT2Y = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT2_Z: meas2card->measureGyroPT2Z = ConvertStrToBool(&BT.input); break;
                //acc
				case ID_meas_2_card_acc_raw_X: meas2card->measureAccRawX = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_raw_Y: meas2card->measureAccRawY = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_raw_Z: meas2card->measureAccRawZ = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_X: meas2card->measureAccPT1X = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_Y: meas2card->measureAccPT1Y = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_Z: meas2card->measureAccPT1Z = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT2_X: meas2card->measureAccPT2X = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT2_Y: meas2card->measureAccPT2Y = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT2_Z: meas2card->measureAccPT2Z = ConvertStrToBool(&BT.input); break;
                //angle
				case ID_meas_2_card_angle_raw_roll: meas2card->measureAngleRawRoll = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_raw_pitch: meas2card->measureAngleRawPitch = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_PT1_roll: meas2card->measureAnglePT1Roll = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_PT1_pitch: meas2card->measureAnglePT1Pitch = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_PT2_roll: meas2card->measureAnglePT2Roll = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_PT2_pitch: meas2card->measureAnglePT2Pitch = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_KF_raw_roll: meas2card->measureAngleKFRawRoll = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_KF_raw_pitch: meas2card->measureAngleKFRawPitch = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT10_roll: meas2card->measureAngleKFPT10Roll = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT10_pitch: meas2card->measureAngleKFPT10Pitch = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT20_roll: meas2card->measureAngleKFPT20Roll = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT20_pitch: meas2card->measureAngleKFPT20Pitch = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT11_roll: meas2card->measureAngleKFPT11Roll = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT11_pitch: meas2card->measureAngleKFPT11Pitch = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT21_roll: meas2card->measureAngleKFPT21Roll = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT21_pitch: meas2card->measureAngleKFPT21Pitch = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT22_roll: meas2card->measureAngleKFPT22Roll = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_KF_PT22_pitch: meas2card->measureAngleKFPT22Pitch = ConvertStrToBool(&BT.input); break;
                //PID control
                case ID_meas_2_card_PID_refsig_X: meas2card->measurePIDRefsigX = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_refsig_Y: meas2card->measurePIDRefsigY = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_refsig_Z: meas2card->measurePIDRefsigZ = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_sensor_X: meas2card->measurePIDSensorX = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_sensor_Y: meas2card->measurePIDSensorY = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_sensor_Z: meas2card->measurePIDSensorZ = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Pout_X: meas2card->measurePIDPoutX = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Pout_Y: meas2card->measurePIDPoutY = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Pout_Z: meas2card->measurePIDPoutZ = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Iout_X: meas2card->measurePIDIoutX = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Iout_Y: meas2card->measurePIDIoutY = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Iout_Z: meas2card->measurePIDIoutZ = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Dout_X: meas2card->measurePIDDoutX = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Dout_Y: meas2card->measurePIDDoutY = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Dout_Z: meas2card->measurePIDDoutZ = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_FFout_X: meas2card->measurePIDFFoutX = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_FFout_Y: meas2card->measurePIDFFoutY = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_FFout_Z: meas2card->measurePIDFFoutZ = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_U_X: meas2card->measurePIDUX = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_U_Y: meas2card->measurePIDUY = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_U_Z: meas2card->measurePIDUZ = ConvertStrToBool(&BT.input); break;

				default:/*nothing*/;
			}
		}
		
		if (CMD_GET == BT.rxFrame.cmd || CMD_SET == BT.rxFrame.cmd)
		{
			BT.txFrame.sendParam = true;
			pid_st* pidRateGet{ getPIDrates() };
            pid_st* pidCascadseSet{ getPIDcascade() };
			gyroData_st* gyroDataGet{ getGyroData() };
			accData_st* accDataSet{ getAccData() };
			spi_st* spi{ getSPI() };
            SpiSDcard_st* sdcard{ getSPISdCard() };
			Meas2Card* meas2card{ getMeas2Card() };

			switch (BT.rxFrame.id)
			{
				case ID_control_PID_rate_P_X: BT.txFrame.paramData = pidRateGet->P.x; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_I_X: BT.txFrame.paramData = pidRateGet->I.x; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_D_X: BT.txFrame.paramData = pidRateGet->D.x; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_P_Y: BT.txFrame.paramData = pidRateGet->P.y; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_I_Y: BT.txFrame.paramData = pidRateGet->I.y; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_D_Y: BT.txFrame.paramData = pidRateGet->D.y; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_P_Z: BT.txFrame.paramData = pidRateGet->P.z; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_I_Z: BT.txFrame.paramData = pidRateGet->I.z; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_D_Z: BT.txFrame.paramData = pidRateGet->D.z; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_FF_X: BT.txFrame.paramData = pidRateGet->FFr.x; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_FF_Y: BT.txFrame.paramData = pidRateGet->FFr.y; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_FF_DX: BT.txFrame.paramData = pidRateGet->FFdr.x; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_FF_DY: BT.txFrame.paramData = pidRateGet->FFdr.y; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_saturation_I: BT.txFrame.paramData = pidRateGet->saturationI; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_saturation_PID: BT.txFrame.paramData = pidRateGet->saturationPID; BT.txFrame.numberOfFrac = 1; break;
				case ID_control_PID_rate_Dterm_C: BT.txFrame.paramData = pidRateGet->DTermC; BT.txFrame.numberOfFrac = 1; break;
				case ID_gyro_filter_paramC: BT.txFrame.paramData = gyroDataGet->paramC; BT.txFrame.numberOfFrac = 1; break;
				case ID_gyro_kalman_filter_q: BT.txFrame.paramData = gyroDataGet->KF.q; BT.txFrame.numberOfFrac = 3; break;
				case ID_gyro_kalman_filter_r: BT.txFrame.paramData = gyroDataGet->KF.r; BT.txFrame.numberOfFrac = 1; break;
				case ID_acc_filter_paramC: BT.txFrame.paramData = accDataSet->paramC; BT.txFrame.numberOfFrac = 1; break;
				case ID_acc_kalman_filter_q_angle: BT.txFrame.paramData = accDataSet->q_angle; BT.txFrame.numberOfFrac = 6; break;
				case ID_acc_kalman_filter_q_bias: BT.txFrame.paramData = accDataSet->q_bias; BT.txFrame.numberOfFrac = 6; break;
				case ID_acc_kalman_filter_r: BT.txFrame.paramData = accDataSet->r_measure; BT.txFrame.numberOfFrac = 2; break;        
				case ID_spi_acc_offset_x: BT.txFrame.paramData = spi->acc.offset.x; BT.txFrame.numberOfFrac = 2; break;
				case ID_spi_acc_offset_y: BT.txFrame.paramData = spi->acc.offset.y; BT.txFrame.numberOfFrac = 2; break;
				case ID_spi_acc_offset_z: BT.txFrame.paramData = spi->acc.offset.z; BT.txFrame.numberOfFrac = 2; break;
                case ID_control_PID_cascade_P_X: BT.txFrame.paramData = pidCascadseSet->P.x; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_I_X: BT.txFrame.paramData = pidCascadseSet->I.x; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_D_X: BT.txFrame.paramData = pidCascadseSet->D.x; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_P_Y: BT.txFrame.paramData = pidCascadseSet->P.y; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_I_Y: BT.txFrame.paramData = pidCascadseSet->I.y; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_D_Y: BT.txFrame.paramData = pidCascadseSet->D.y; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_P_Z: BT.txFrame.paramData = pidCascadseSet->P.z; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_I_Z: BT.txFrame.paramData = pidCascadseSet->I.z; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_D_Z: BT.txFrame.paramData = pidCascadseSet->D.z; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_saturation_I: BT.txFrame.paramData = pidCascadseSet->saturationI; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_saturation_PID: BT.txFrame.paramData = pidCascadseSet->saturationPID; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_Dterm_C: BT.txFrame.paramData = pidCascadseSet->DTermC; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_FF_X: BT.txFrame.paramData = pidCascadseSet->FFr.x; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_FF_Y: BT.txFrame.paramData = pidCascadseSet->FFr.y; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_FF_DX: BT.txFrame.paramData = pidCascadseSet->FFdr.x; BT.txFrame.numberOfFrac = 1; break;
                case ID_control_PID_cascade_FF_DY: BT.txFrame.paramData = pidCascadseSet->FFdr.y; BT.txFrame.numberOfFrac = 1; break;

                case ID_SDCARD_MAINSTATE: BT.txFrame.paramData = sdcard->MainState; BT.txFrame.numberOfFrac = 0; break;
                case ID_SDCARD_RESET_MEASRUEMENT: BT.txFrame.paramData = ResetMeasurement(); BT.txFrame.numberOfFrac = 0; break;
                case ID_SDCARD_REINIT_SDCARD: BT.txFrame.paramData = ReinitSDCard(); BT.txFrame.numberOfFrac = 0; break;


				//MEAS2SDCARD
				case ID_meas_2_card_sysTime: BT.txFrame.paramData = meas2card->measureSysTime; BT.txFrame.numberOfFrac = 0; break;
                //gyro
				case ID_meas_2_card_gyro_raw_X: BT.txFrame.paramData = meas2card->measureGyroRawX; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_raw_Y: BT.txFrame.paramData = meas2card->measureGyroRawY; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_raw_Z: BT.txFrame.paramData = meas2card->measureGyroRawZ; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_X: BT.txFrame.paramData = meas2card->measureGyroPT1X; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_Y: BT.txFrame.paramData = meas2card->measureGyroPT1Y; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_Z: BT.txFrame.paramData = meas2card->measureGyroPT1Z; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT2_X: BT.txFrame.paramData = meas2card->measureGyroPT2X; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT2_Y: BT.txFrame.paramData = meas2card->measureGyroPT2Y; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT2_Z: BT.txFrame.paramData = meas2card->measureGyroPT2Z; BT.txFrame.numberOfFrac = 0; break;
                //acc
				case ID_meas_2_card_acc_raw_X: BT.txFrame.paramData = meas2card->measureAccRawX; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_raw_Y: BT.txFrame.paramData = meas2card->measureAccRawY; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_raw_Z: BT.txFrame.paramData = meas2card->measureAccRawZ; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_X: BT.txFrame.paramData = meas2card->measureAccPT1X; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_Y: BT.txFrame.paramData = meas2card->measureAccPT1Y; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_Z: BT.txFrame.paramData = meas2card->measureAccPT1Z; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT2_X: BT.txFrame.paramData = meas2card->measureAccPT2X; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT2_Y: BT.txFrame.paramData = meas2card->measureAccPT2Y; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT2_Z: BT.txFrame.paramData = meas2card->measureAccPT2Z; BT.txFrame.numberOfFrac = 0; break;
                //angle
				case ID_meas_2_card_angle_raw_roll: BT.txFrame.paramData = meas2card->measureAngleRawRoll; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_raw_pitch: BT.txFrame.paramData = meas2card->measureAngleRawPitch; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_PT1_roll: BT.txFrame.paramData = meas2card->measureAnglePT1Roll; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_PT1_pitch: BT.txFrame.paramData = meas2card->measureAnglePT1Pitch; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_PT2_roll: BT.txFrame.paramData = meas2card->measureAnglePT2Roll; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_PT2_pitch: BT.txFrame.paramData = meas2card->measureAnglePT2Pitch; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_raw_roll: BT.txFrame.paramData = meas2card->measureAngleKFRawRoll; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_raw_pitch: BT.txFrame.paramData = meas2card->measureAngleKFRawPitch; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT10_roll: BT.txFrame.paramData = meas2card->measureAngleKFPT10Roll; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT10_pitch: BT.txFrame.paramData = meas2card->measureAngleKFPT10Pitch; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT20_roll: BT.txFrame.paramData = meas2card->measureAngleKFPT20Roll; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT20_pitch: BT.txFrame.paramData = meas2card->measureAngleKFPT20Pitch; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT11_roll: BT.txFrame.paramData = meas2card->measureAngleKFPT11Roll; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT11_pitch: BT.txFrame.paramData = meas2card->measureAngleKFPT11Pitch; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT21_roll: BT.txFrame.paramData = meas2card->measureAngleKFPT21Roll; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT21_pitch: BT.txFrame.paramData = meas2card->measureAngleKFPT21Pitch; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT22_roll: BT.txFrame.paramData = meas2card->measureAngleKFPT22Roll; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_angle_KF_PT22_pitch: BT.txFrame.paramData = meas2card->measureAngleKFPT22Pitch; BT.txFrame.numberOfFrac = 0; break;
                //PID sensor
                case ID_meas_2_card_PID_refsig_X: BT.txFrame.paramData = meas2card->measurePIDRefsigX; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_refsig_Y: BT.txFrame.paramData = meas2card->measurePIDRefsigY; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_refsig_Z: BT.txFrame.paramData = meas2card->measurePIDRefsigZ; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_sensor_X: BT.txFrame.paramData = meas2card->measurePIDSensorX; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_sensor_Y: BT.txFrame.paramData = meas2card->measurePIDSensorY; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_sensor_Z: BT.txFrame.paramData = meas2card->measurePIDSensorZ; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Pout_X: BT.txFrame.paramData = meas2card->measurePIDPoutX; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Pout_Y: BT.txFrame.paramData = meas2card->measurePIDPoutY; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Pout_Z: BT.txFrame.paramData = meas2card->measurePIDPoutZ; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Iout_X: BT.txFrame.paramData = meas2card->measurePIDIoutX; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Iout_Y: BT.txFrame.paramData = meas2card->measurePIDIoutY; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Iout_Z: BT.txFrame.paramData = meas2card->measurePIDIoutZ; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Dout_X: BT.txFrame.paramData = meas2card->measurePIDDoutX; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Dout_Y: BT.txFrame.paramData = meas2card->measurePIDDoutY; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Dout_Z: BT.txFrame.paramData = meas2card->measurePIDDoutZ; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_FFout_X: BT.txFrame.paramData = meas2card->measurePIDFFoutX; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_FFout_Y: BT.txFrame.paramData = meas2card->measurePIDFFoutY; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_FFout_Z: BT.txFrame.paramData = meas2card->measurePIDFFoutZ; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_U_X: BT.txFrame.paramData = meas2card->measurePIDUX; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_U_Y: BT.txFrame.paramData = meas2card->measurePIDUY; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_U_Z: BT.txFrame.paramData = meas2card->measurePIDUZ; BT.txFrame.numberOfFrac = 0; break;

				default:BT.txFrame.sendParam = false;
			}
		}
		
		if (CMD_STREAM == BT.rxFrame.cmd)
		{
			switch (BT.rxFrame.id)
			{
				//flagset 1
				case ID_pidRate_sensor_signal_X: SetSteamFlag(ID_pidRate_sensor_signal_X); break;
				case ID_pidRate_sensor_signal_Y: SetSteamFlag(ID_pidRate_sensor_signal_Y); break;
				case ID_pidRate_sensor_signal_Z: SetSteamFlag(ID_pidRate_sensor_signal_Z); break;
				case ID_gyro_signal_X: SetSteamFlag(ID_gyro_signal_X); break;
				case ID_gyro_signal_Y: SetSteamFlag(ID_gyro_signal_Y); break;
				case ID_gyro_signal_Z: SetSteamFlag(ID_gyro_signal_Z); break;
				case ID_motorCommand_FL: SetSteamFlag(ID_motorCommand_FL); break;
				case ID_motorCommand_FR: SetSteamFlag(ID_motorCommand_FR); break;
				case ID_motorCommand_RL: SetSteamFlag(ID_motorCommand_RL); break;
				case ID_motorCommand_RR: SetSteamFlag(ID_motorCommand_RR); break;
				case ID_gyro_PT1_X: SetSteamFlag(ID_gyro_PT1_X); break;
				case ID_gyro_PT1_Y: SetSteamFlag(ID_gyro_PT1_Y); break;
				case ID_gyro_PT1_Z: SetSteamFlag(ID_gyro_PT1_Z); break;
				case ID_gyro_PT2_X: SetSteamFlag(ID_gyro_PT2_X); break;
				case ID_gyro_PT2_Y: SetSteamFlag(ID_gyro_PT2_Y); break;
				case ID_gyro_PT2_Z: SetSteamFlag(ID_gyro_PT2_Z); break;
				case ID_gyro_KALMAN_X: SetSteamFlag(ID_gyro_KALMAN_X); break;
				case ID_gyro_KALMAN_Y: SetSteamFlag(ID_gyro_KALMAN_Y); break;
				case ID_gyro_KALMAN_Z: SetSteamFlag(ID_gyro_KALMAN_Z); break;

				case ID_acc_signal_X: SetSteamFlag(ID_acc_signal_X); break;
				case ID_acc_signal_Y: SetSteamFlag(ID_acc_signal_Y); break;
				case ID_acc_signal_Z: SetSteamFlag(ID_acc_signal_Z); break;
				case ID_acc_PT1_X: SetSteamFlag(ID_acc_PT1_X); break;
				case ID_acc_PT1_Y: SetSteamFlag(ID_acc_PT1_Y); break;
				case ID_acc_PT1_Z: SetSteamFlag(ID_acc_PT1_Z); break;
				case ID_acc_PT2_X: SetSteamFlag(ID_acc_PT2_X); break;
				case ID_acc_PT2_Y: SetSteamFlag(ID_acc_PT2_Y); break;
				case ID_acc_PT2_Z: SetSteamFlag(ID_acc_PT2_Z); break;
				case ID_roll: SetSteamFlag(ID_roll); break;
				case ID_pitch: SetSteamFlag(ID_pitch); break;
				case ID_roll_PT1: SetSteamFlag(ID_roll_PT1); break;
				case ID_pitch_PT1: SetSteamFlag(ID_pitch_PT1); break;
				//flagset 2
				case ID_roll_PT2: SetSteamFlag2(ID_roll_PT2); break;
				case ID_pitch_PT2: SetSteamFlag2(ID_pitch_PT2); break;
				case ID_roll_KF: SetSteamFlag2(ID_roll_KF); break;
				case ID_pitch_KF: SetSteamFlag2(ID_pitch_KF); break;
				case ID_roll_KF_AccPT1: SetSteamFlag2(ID_roll_KF_AccPT1); break;
				case ID_pitch_KF_AccPT1: SetSteamFlag2(ID_pitch_KF_AccPT1); break;
				case ID_roll_KF_AccPT2: SetSteamFlag2(ID_roll_KF_AccPT2); break;
				case ID_pitch_KF_AccPT2: SetSteamFlag2(ID_pitch_KF_AccPT2); break;
				case ID_roll_KF_AccPT1_GyroPT1: SetSteamFlag2(ID_roll_KF_AccPT1_GyroPT1); break;
				case ID_pitch_KF_AccPT1_GyroPT1: SetSteamFlag2(ID_pitch_KF_AccPT1_GyroPT1); break;
				case ID_roll_KF_AccPT2_GyroPT2: SetSteamFlag2(ID_roll_KF_AccPT2_GyroPT2); break;
				case ID_pitch_KF_AccPT2_GyroPT2: SetSteamFlag2(ID_pitch_KF_AccPT2_GyroPT2); break;
				case ID_loop_time_ms: SetSteamFlag2(ID_loop_time_ms); break;

			}
		}
	}
	else 
	{
		BT.rxFrame.cmd = CMD_NONE;
		BT.rxFrame.id = 0;
	}
}

void SetStreamData(const uint16_t dataID, const float dataValue)
{
	//set ID, 2 bytes
	BT.output.vector[BT.output.ctr++] = uint8_t(dataID & 0xFF);
	BT.output.vector[BT.output.ctr++] = uint8_t((dataID & 0xFF00) >> 8);
	//set data value
	CalcCharAndFillOutput(dataValue, 3u);
	//set data ending
	BT.output.vector[BT.output.ctr++] = '!';
}

void SetSteamFlag(const uint16_t steamDataID)
{
	if (BT.input.vector[3] == 'F')  //if OFF
	{
        BT.txFrame.streamDataFlags &= ~(1u << (steamDataID - ID_bitshift_substracter));
	}
	else if (BT.input.vector[3] == 'O')	//if ON
	{
		BT.txFrame.streamDataFlags |= 1u << (steamDataID - ID_bitshift_substracter);
	}
    else
    {
        //do nothing if none of the character received
    }
}

void SetSteamFlag2(const uint16_t steamDataID)
{
	if (BT.input.vector[3] == 'F')  //if OFF
	{
		BT.txFrame.streamDataFlags2 &= ~(1u << (steamDataID - ID_bitshift_substracter2));
	}
	else if (BT.input.vector[3] == 'O')//if ON
	{
		BT.txFrame.streamDataFlags2 |= 1u << (steamDataID - ID_bitshift_substracter2);
	}
	else
	{
		//do nothing if none of the character received
	}
}

bool ConvertStrToBool(volatile buffer_* input)
{
	uint8_t bool_char = input->vector[3];
	bool returnVal{false};

	if ('0' == bool_char)
	{
		returnVal = false;
	}
	else
	{
		returnVal = true;
	}

	return returnVal;
}

uint8_t ConvertStrToUint8(volatile buffer_* input)
{
	int8_t last_char = input->ctr - 2;	// \r\n
	int8_t integer_diff = 0;
	uint8_t returnVal = 0;

	//integer calcu
	for (uint8_t pos = 3; pos < last_char; pos++)
	{
		integer_diff = last_char - pos - 1;
		returnVal += uint8_t(pow(10, integer_diff)) * uint8_t(input->vector[pos] - '0');
	}

	return returnVal;
}

uint16_t ConvertStrToUint16(volatile buffer_* input)
{
	int8_t last_char = input->ctr - 2;	// \r\n
	int8_t integer_diff = 0;
	uint16_t returnVal = 0;

	//integer calcu
	for (uint8_t pos = 3; pos < last_char; pos++)
	{
		integer_diff = last_char - pos - 1;
		returnVal += uint16_t(pow(10, integer_diff)) * uint16_t(input->vector[pos] - '0');
	}

	return returnVal;
}

float ConvertStrToFloat(volatile buffer_* input)
{
	int8_t first_char = 3;
	int8_t last_char = input->ctr - 2; //13-2-1, 2=\r\n 1=because array start with 0
	int8_t dot_place = last_char;
	int8_t integer_diff = 0;
	int8_t fractional_diff = 0;
	bool isNegative = false;
	float returnVal = 0;

	//if its a negative value raise flag and increment first_char
	if ('-' == input->vector[first_char])
	{
		first_char++;
		isNegative = true;
	}

	//calcu number of integer
	for (uint8_t i = first_char; i < last_char; i++)
	{
		if ('.' == input->vector[i])
		{
			dot_place = i;
			break;
		}
	}

	//integer calcu
	for (int8_t a = first_char; a < dot_place; a++)
	{
		integer_diff = dot_place - a - 1;
		returnVal += float(pow(10, integer_diff)) * float(input->vector[a] - '0');
	}
	//fractional calcu
	for (int8_t b = last_char, c = (dot_place + 1); b > dot_place+1; b--, c++)
	{
		fractional_diff = b - last_char - 1;
		returnVal += float(pow(10, fractional_diff)) * float(input->vector[c] - '0');
	}
	//if negative
	if (isNegative) returnVal *= -1;

	return returnVal;
}

bool isFrameEndReceived(void)
{
	return ('\r' == BT.input.vector[BT.input.ctr - 2] && '\n' == BT.input.vector[BT.input.ctr - 1]);
}

void CalcCharAndFillOutput(float value, uint8_t numOfFrac)
{
	uint8_t currentCtr = 0;
	//increment with a small value to solve the numerical accurecy problem, (without this a 0.005 would be 0.0049)
	value += 5*pow(10, -(numOfFrac + 1));

	if (value < 0) //if its negative, append a - sign and make it positive so the rest of the algo is the same
	{
		BT.output.vector[BT.output.ctr++] = '-';
		value *= -1;
	}

	if (value >= 1.0)  //if the value is is not smaller then 1
	{
		while (value >= 10.0)
		{
			value /= 10;
			currentCtr++;
		}

		for (uint8_t i = 0; i <= currentCtr; i++) //creating integer part
		{
			uint8_t currentInt;

			currentInt = (uint8_t)value; //creat the integer part
			BT.output.vector[BT.output.ctr++] = '0' + currentInt; //set correct ASCII char for sending
			value -= (float)currentInt;  //substitute it from the original
			value *= 10;  //multiplie by 10 for the next round
		}

		if (numOfFrac > 0) //if we need fractional part
		{
			BT.output.vector[BT.output.ctr++] = '.'; //append dot

			for (uint8_t f = 0; f < numOfFrac; f++)  //creating fractional part
			{
				uint8_t currentFrac;

				currentFrac = (uint8_t)value;  //creat the integer part
				BT.output.vector[BT.output.ctr++] = '0' + currentFrac; //set correct ASCII char for sending
				value -= (float)currentFrac; //substitute it from the original
				value *= 10;  //multiplie by 10 for the next round
			}
		}
	}
	else  //if the value is small then 1
	{
		BT.output.vector[BT.output.ctr++] = '0'; //append zero

		if (numOfFrac > 0) //if we need fractional part
		{
			BT.output.vector[BT.output.ctr++] = '.'; //append dot

			for (uint8_t f = 0; f < numOfFrac; f++)  //creating fractional part
			{
				uint8_t currentFrac;

				value *= 10;  //multiplie by 10 for the next round
				currentFrac = (uint8_t)value;  //creat the integer part
				BT.output.vector[BT.output.ctr++] = '0' + currentFrac; //set correct ASCII char for sending
				value -= (float)currentFrac; //substitute it from the original
			}
		}

	}
}
