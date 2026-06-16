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

#define TRANSMIT_DELAY 115500 // 11ms in ticks

//#define DEBUG_BT
//#define DEBUG_BT_INTERRUPT

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

void RunBT()
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
	
	BTTransmit();


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
#ifdef DEBUG_BT_INTERRUPT
	SerialUSB.println(usart_status);
#endif

	if (usart_status & US_CSR_ENDRX)
	{
		if (WAITING_FOR_NUMBEROFBYTES == BT.rxDataState)
		{
			BT.input.ctr = BT.input.vector[0];
			USART1->US_RPR = (uint32_t)BT.input.vector;
			USART1->US_RCR = BT.input.ctr;
			USART1->US_PTCR |= US_PTCR_RXTEN; //start pdc receive

			BT.rxDataState = RECEVING_DATA_BYTES;
			
#ifdef DEBUG_BT_INTERRUPT
		SerialUSB.println("c");
#endif
		}
		else if (RECEVING_DATA_BYTES == BT.rxDataState)
		{
			//enable receive
			USART1->US_CR |= US_CR_RXDIS;
			BT.rxDataState = FRAME_RECEIVED;
			USART1->US_RCR = 1; //counter set to 1 to unset interrupt flag
			
#ifdef DEBUG_BT_INTERRUPT
		SerialUSB.println("d");
#endif
		}
		
	}
}

void BTReceive()
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

#ifdef DEBUG_BT
		SerialUSB.print("BTReceive: cmd - "); SerialUSB.print(BT.rxFrame.cmd);
		SerialUSB.print(" ,id - "); SerialUSB.print(BT.rxFrame.id);
		SerialUSB.print(" ,paramData - "); SerialUSB.print(BT.txFrame.paramData);
		SerialUSB.print(" ,sendParam - "); SerialUSB.print(BT.txFrame.sendParam);
		SerialUSB.println();
#endif

	}
}

void BTTransmit()
{
	bool triggerTx{ false };
	BT.output.ctr = 2;

    //only enter if at least 10ms elapsed since previous data send
    if (getSysTick() - BT.lastTimeTx > TRANSMIT_DELAY)
    {
		BT.lastTimeTx = getSysTick();

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
            if ((BT.txFrame.streamDataFlags & (1 << (ID_pidRate_sensor_signal_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_pidRate_sensor_signal_X, pidRate->sensor.signalPT1.x);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_pidRate_sensor_signal_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_pidRate_sensor_signal_Y, pidRate->sensor.signalPT1.y);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_pidRate_sensor_signal_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_pidRate_sensor_signal_Z, pidRate->sensor.signalPT1.z);

            if ((BT.txFrame.streamDataFlags & (1 << (ID_motorCommand_FL - ID_bitshift_substracter))) > 0) SetStreamData(ID_motorCommand_FL, motorSpeeds.FL_tick);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_motorCommand_FR - ID_bitshift_substracter))) > 0) SetStreamData(ID_motorCommand_FR, motorSpeeds.FR_tick);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_motorCommand_RL - ID_bitshift_substracter))) > 0) SetStreamData(ID_motorCommand_RL, motorSpeeds.RL_tick);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_motorCommand_RR - ID_bitshift_substracter))) > 0) SetStreamData(ID_motorCommand_RR, motorSpeeds.RR_tick);

            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT1_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT1_X, gyroData->PT1.signalPT1.x);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT1_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT1_Y, gyroData->PT1.signalPT1.y);
            if ((BT.txFrame.streamDataFlags & (1 << (ID_gyro_PT1_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_gyro_PT1_Z, gyroData->PT1.signalPT1.z);

			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT1_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT1_X, accData->PT1.signalPT1.x);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT1_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT1_Y, accData->PT1.signalPT1.y);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT1_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT1_Z, accData->PT1.signalPT1.z);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT2_X - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT2_X, accData->PT2.signalPT1.x);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT2_Y - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT2_Y, accData->PT2.signalPT1.y);
			if ((BT.txFrame.streamDataFlags & (1 << (ID_acc_PT2_Z - ID_bitshift_substracter))) > 0) SetStreamData(ID_acc_PT2_Z, accData->PT2.signalPT1.z);

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
			if ((BT.txFrame.streamDataFlags2 & (1 << (ID_loop_tick - ID_bitshift_substracter2))) > 0) SetStreamData(ID_loop_tick, (float)getSysTick()/10.5);

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
						
#ifdef DEBUG_BT
		SerialUSB.println("BTTransmit - parameter: "); SerialUSB.print(BT.txFrame.paramData);
#endif
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
        }
    }
}

void ProcessRxFrame()
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
				//case ID_control_PID_rate_P_X: pidRateSet->P.x = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_I_X: pidRateSet->I.x = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_D_X: pidRateSet->D.x = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_P_Y: pidRateSet->P.y = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_I_Y: pidRateSet->I.y = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_D_Y: pidRateSet->D.y = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_P_Z: pidRateSet->P.z = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_I_Z: pidRateSet->I.z = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_D_Z: pidRateSet->D.z = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_FF_X: pidRateSet->FFr.x = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_FF_Y: pidRateSet->FFr.y = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_FF_DX: pidRateSet->FFdr.x = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_FF_DY: pidRateSet->FFdr.y = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_saturation_I: pidRateSet->saturationI = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_saturation_PID: pidRateSet->saturationPID = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_Dterm_C: pidRateSet->DTermC = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_FF_Dterm_C: pidRateSet->FFDTermC = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_I_relax_ref_threshhold: pidRateSet->iRelaxRefThreshhold = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_I_relax_error_threshhold: pidRateSet->iRelaxErrThreshhold = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_D_max_ref_threshhold: pidRateSet->dMaxRefThreshhold = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_D_max_error_threshhold: pidRateSet->dMaxErrThreshhold = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_D_max_X: pidRateSet->Dmax.x = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_D_max_Y: pidRateSet->Dmax.y = ConvertStrToDouble(&BT.input); break;
				//case ID_control_PID_rate_D_max_Z: pidRateSet->Dmax.z = ConvertStrToDouble(&BT.input); break;
				case ID_complementary_filter_alpha: accDataSet->alpha = ConvertStrToDouble(&BT.input); break;
				case ID_acc_kalman_filter_q_angle: accDataSet->q_angle = ConvertStrToDouble(&BT.input);	break;
				case ID_acc_kalman_filter_q_bias: accDataSet->q_bias = ConvertStrToDouble(&BT.input);	break;
				case ID_acc_kalman_filter_r: accDataSet->r_measure = ConvertStrToDouble(&BT.input);	break;
				case ID_spi_acc_offset_int_x: spi->acc.offset.x = ConvertStrToInt32(&BT.input);	break;
				case ID_spi_acc_offset_int_y: spi->acc.offset.y = ConvertStrToInt32(&BT.input);	break;
				case ID_spi_acc_offset_int_z: spi->acc.offset.z = ConvertStrToInt32(&BT.input);	break;
				case ID_control_PID_rate_P_X_int: pidRateSet->P_i.x = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_I_X_int: pidRateSet->I_i.x = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_D_X_int: pidRateSet->D_i.x = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_P_Y_int: pidRateSet->P_i.y = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_I_Y_int: pidRateSet->I_i.y = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_D_Y_int: pidRateSet->D_i.y = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_P_Z_int: pidRateSet->P_i.z = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_I_Z_int: pidRateSet->I_i.z = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_D_Z_int: pidRateSet->D_i.z = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_FF_X_int: pidRateSet->FFr_i.x = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_FF_Y_int: pidRateSet->FFr_i.y = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_FF_DX_int: pidRateSet->FFdr_i.x = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_FF_DY_int: pidRateSet->FFdr_i.y = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_sat_I_int: pidRateSet->satI_i = ConvertStrToInt32(&BT.input); break;
				case ID_control_PID_rate_sat_PID_int: pidRateSet->satPID_i = ConvertStrToInt32(&BT.input); break;


				//
				case ID_update_global_time: setGlobalTime(ConvertStrToGlobalTime(&BT.input), getSysTick()); break;
				case ID_update_global_date: setGlobalDate(ConvertStrToGlobalDate(&BT.input)); break;

				// meas 2 sdcard
				case ID_meas_2_card_sysTick: meas2card->measureSysTick = ConvertStrToBool(&BT.input); break;
                //gyro
				case ID_meas_2_card_gyro_raw_X: meas2card->measureGyroRawX = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_raw_Y: meas2card->measureGyroRawY = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_raw_Z: meas2card->measureGyroRawZ = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_X: meas2card->measureGyroPT1X = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_Y: meas2card->measureGyroPT1Y = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_Z: meas2card->measureGyroPT1Z = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_raw_X_int: meas2card->measureGyroRealX = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_raw_Y_int: meas2card->measureGyroRealY = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_raw_Z_int: meas2card->measureGyroRealZ = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_X_int: meas2card->measureGyroRealPT1X = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_Y_int: meas2card->measureGyroRealPT1Y = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_gyro_PT1_Z_int: meas2card->measureGyroRealPT1Z = ConvertStrToBool(&BT.input); break;
                //acc
				case ID_meas_2_card_acc_raw_X: meas2card->measureAccRawX = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_raw_Y: meas2card->measureAccRawY = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_raw_Z: meas2card->measureAccRawZ = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_X: meas2card->measureAccPT1X = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_Y: meas2card->measureAccPT1Y = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_Z: meas2card->measureAccPT1Z = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_raw_X_int: meas2card->measureAccRealX = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_raw_Y_int: meas2card->measureAccRealY = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_raw_Z_int: meas2card->measureAccRealZ = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_X_int: meas2card->measureAccRealPT1X = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_Y_int: meas2card->measureAccRealPT1Y = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_acc_PT1_Z_int: meas2card->measureAccRealPT1Z = ConvertStrToBool(&BT.input); break;
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
                case ID_meas_2_card_angle_CF_raw_roll: meas2card->measureAngleCFRawRoll = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_angle_CF_raw_pitch: meas2card->measureAngleCFRawPitch = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_CF_PT10_roll: meas2card->measureAngleCFPT10Roll = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_CF_PT10_pitch: meas2card->measureAngleCFPT10Pitch = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_CF_PT11_roll: meas2card->measureAngleCFPT11Roll = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_CF_PT11_pitch: meas2card->measureAngleCFPT11Pitch = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_CF_weighted_raw_roll: meas2card->measureAngleCFWeightedRawRoll = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_CF_weighted_raw_pitch: meas2card->measureAngleCFWeightedRawPitch = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_CF_weighted_PT01_roll: meas2card->measureAngleCFWeightedPT01Roll = ConvertStrToBool(&BT.input); break;
				case ID_meas_2_card_angle_CF_weighted_PT01_pitch: meas2card->measureAngleCFWeightedPT01Pitch = ConvertStrToBool(&BT.input); break;
                //PID control
                case ID_meas_2_card_PID_refsig_X: meas2card->measurePIDRefsigX_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_refsig_Y: meas2card->measurePIDRefsigY_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_refsig_Z: meas2card->measurePIDRefsigZ_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_sensor_X: meas2card->measurePIDSensorX_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_sensor_Y: meas2card->measurePIDSensorY_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_sensor_Z: meas2card->measurePIDSensorZ_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Pout_X: meas2card->measurePIDPoutX_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Pout_Y: meas2card->measurePIDPoutY_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Pout_Z: meas2card->measurePIDPoutZ_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Iout_X: meas2card->measurePIDIoutX_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Iout_Y: meas2card->measurePIDIoutY_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Iout_Z: meas2card->measurePIDIoutZ_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Dout_X: meas2card->measurePIDDoutX_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Dout_Y: meas2card->measurePIDDoutY_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_Dout_Z: meas2card->measurePIDDoutZ_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_FFout_X: meas2card->measurePIDFFoutX_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_FFout_Y: meas2card->measurePIDFFoutY_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_FFout_Z: meas2card->measurePIDFFoutZ_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_U_X: meas2card->measurePIDUX_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_U_Y: meas2card->measurePIDUY_i = ConvertStrToBool(&BT.input); break;
                case ID_meas_2_card_PID_U_Z: meas2card->measurePIDUZ_i = ConvertStrToBool(&BT.input); break;

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
				//case ID_control_PID_rate_P_X: BT.txFrame.paramData = pidRateGet->P.x; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_I_X: BT.txFrame.paramData = pidRateGet->I.x; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_D_X: BT.txFrame.paramData = pidRateGet->D.x; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_P_Y: BT.txFrame.paramData = pidRateGet->P.y; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_I_Y: BT.txFrame.paramData = pidRateGet->I.y; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_D_Y: BT.txFrame.paramData = pidRateGet->D.y; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_P_Z: BT.txFrame.paramData = pidRateGet->P.z; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_I_Z: BT.txFrame.paramData = pidRateGet->I.z; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_D_Z: BT.txFrame.paramData = pidRateGet->D.z; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_FF_X: BT.txFrame.paramData = pidRateGet->FFr.x; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_FF_Y: BT.txFrame.paramData = pidRateGet->FFr.y; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_FF_DX: BT.txFrame.paramData = pidRateGet->FFdr.x; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_FF_DY: BT.txFrame.paramData = pidRateGet->FFdr.y; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_saturation_I: BT.txFrame.paramData = pidRateGet->saturationI; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_saturation_PID: BT.txFrame.paramData = pidRateGet->saturationPID; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_Dterm_C: BT.txFrame.paramData = pidRateGet->DTermC; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_FF_Dterm_C: BT.txFrame.paramData = pidRateGet->FFDTermC; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_I_relax_ref_threshhold: BT.txFrame.paramData = pidRateGet->iRelaxRefThreshhold; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_I_relax_error_threshhold: BT.txFrame.paramData = pidRateGet->iRelaxErrThreshhold; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_D_max_ref_threshhold: BT.txFrame.paramData = pidRateGet->dMaxRefThreshhold; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_D_max_error_threshhold: BT.txFrame.paramData = pidRateGet->dMaxErrThreshhold; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_D_max_X: BT.txFrame.paramData = pidRateGet->Dmax.x; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_D_max_Y: BT.txFrame.paramData = pidRateGet->Dmax.y; BT.txFrame.numberOfFrac = 1; break;
				//case ID_control_PID_rate_D_max_Z: BT.txFrame.paramData = pidRateGet->Dmax.z; BT.txFrame.numberOfFrac = 1; break;
				case ID_complementary_filter_alpha: BT.txFrame.paramData = accDataSet->alpha; BT.txFrame.numberOfFrac = 3; break;
				case ID_acc_kalman_filter_q_angle: BT.txFrame.paramData = accDataSet->q_angle; BT.txFrame.numberOfFrac = 6; break;
				case ID_acc_kalman_filter_q_bias: BT.txFrame.paramData = accDataSet->q_bias; BT.txFrame.numberOfFrac = 6; break;
				case ID_acc_kalman_filter_r: BT.txFrame.paramData = accDataSet->r_measure; BT.txFrame.numberOfFrac = 2; break;

                case ID_SDCARD_MAINSTATE: BT.txFrame.paramData = sdcard->MainState; BT.txFrame.numberOfFrac = 0; break;
                case ID_SDCARD_RESET_MEASRUEMENT: BT.txFrame.paramData = ResetMeasurement(); BT.txFrame.numberOfFrac = 0; break;
                case ID_SDCARD_REINIT_SDCARD: BT.txFrame.paramData = ReinitSDCard(); BT.txFrame.numberOfFrac = 0; break;
                case ID_spi_acc_offset_int_x: BT.txFrame.paramData = spi->acc.offset.x; BT.txFrame.numberOfFrac = 0; break;
                case ID_spi_acc_offset_int_y: BT.txFrame.paramData = spi->acc.offset.y; BT.txFrame.numberOfFrac = 0; break;
                case ID_spi_acc_offset_int_z: BT.txFrame.paramData = spi->acc.offset.z; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_P_X_int: BT.txFrame.paramData = pidRateGet->P_i.x; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_I_X_int: BT.txFrame.paramData = pidRateGet->I_i.x; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_D_X_int: BT.txFrame.paramData = pidRateGet->D_i.x; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_P_Y_int: BT.txFrame.paramData = pidRateGet->P_i.y; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_I_Y_int: BT.txFrame.paramData = pidRateGet->I_i.y; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_D_Y_int: BT.txFrame.paramData = pidRateGet->D_i.y; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_P_Z_int: BT.txFrame.paramData = pidRateGet->P_i.z; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_I_Z_int: BT.txFrame.paramData = pidRateGet->I_i.z; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_D_Z_int: BT.txFrame.paramData = pidRateGet->D_i.z; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_FF_X_int: BT.txFrame.paramData = pidRateGet->FFr_i.x; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_FF_Y_int: BT.txFrame.paramData = pidRateGet->FFr_i.y; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_FF_DX_int: BT.txFrame.paramData = pidRateGet->FFdr_i.x; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_FF_DY_int: BT.txFrame.paramData = pidRateGet->FFdr_i.y; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_sat_I_int: BT.txFrame.paramData = pidRateGet->satI_i; BT.txFrame.numberOfFrac = 0; break;
				case ID_control_PID_rate_sat_PID_int: BT.txFrame.paramData = pidRateGet->satPID_i; BT.txFrame.numberOfFrac = 0; break;

				//
				case ID_update_global_time: BT.txFrame.paramData = ConvertGlobalTime(&sdcard->globalDateAndTime); BT.txFrame.numberOfFrac = 0; break;
				case ID_update_global_date: BT.txFrame.paramData = ConvertGlobalDate(&sdcard->globalDateAndTime); BT.txFrame.numberOfFrac = 0; break;

				//MEAS2SDCARD
				case ID_meas_2_card_sysTick: BT.txFrame.paramData = meas2card->measureSysTick; BT.txFrame.numberOfFrac = 0; break;
                //gyro
				case ID_meas_2_card_gyro_raw_X: BT.txFrame.paramData = meas2card->measureGyroRawX; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_raw_Y: BT.txFrame.paramData = meas2card->measureGyroRawY; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_raw_Z: BT.txFrame.paramData = meas2card->measureGyroRawZ; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_X: BT.txFrame.paramData = meas2card->measureGyroPT1X; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_Y: BT.txFrame.paramData = meas2card->measureGyroPT1Y; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_Z: BT.txFrame.paramData = meas2card->measureGyroPT1Z; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_raw_X_int: BT.txFrame.paramData = meas2card->measureGyroRealX; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_raw_Y_int: BT.txFrame.paramData = meas2card->measureGyroRealY; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_raw_Z_int: BT.txFrame.paramData = meas2card->measureGyroRealZ; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_X_int: BT.txFrame.paramData = meas2card->measureGyroRealPT1X; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_Y_int: BT.txFrame.paramData = meas2card->measureGyroRealPT1Y; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_gyro_PT1_Z_int: BT.txFrame.paramData = meas2card->measureGyroRealPT1Z; BT.txFrame.numberOfFrac = 0; break;
                //acc
				case ID_meas_2_card_acc_raw_X: BT.txFrame.paramData = meas2card->measureAccRawX; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_raw_Y: BT.txFrame.paramData = meas2card->measureAccRawY; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_raw_Z: BT.txFrame.paramData = meas2card->measureAccRawZ; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_X: BT.txFrame.paramData = meas2card->measureAccPT1X; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_Y: BT.txFrame.paramData = meas2card->measureAccPT1Y; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_Z: BT.txFrame.paramData = meas2card->measureAccPT1Z; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_raw_X_int: BT.txFrame.paramData = meas2card->measureAccRealX; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_raw_Y_int: BT.txFrame.paramData = meas2card->measureAccRealY; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_raw_Z_int: BT.txFrame.paramData = meas2card->measureAccRealZ; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_X_int: BT.txFrame.paramData = meas2card->measureAccRealPT1X; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_Y_int: BT.txFrame.paramData = meas2card->measureAccRealPT1Y; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_acc_PT1_Z_int: BT.txFrame.paramData = meas2card->measureAccRealPT1Z; BT.txFrame.numberOfFrac = 0; break;
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
				case ID_meas_2_card_angle_CF_raw_roll: BT.txFrame.paramData = meas2card->measureAngleCFRawRoll; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_raw_pitch: BT.txFrame.paramData = meas2card->measureAngleCFRawPitch; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_PT10_roll: BT.txFrame.paramData = meas2card->measureAngleCFPT10Roll; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_PT10_pitch: BT.txFrame.paramData = meas2card->measureAngleCFPT10Pitch; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_PT11_roll: BT.txFrame.paramData = meas2card->measureAngleCFPT11Roll; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_PT11_pitch: BT.txFrame.paramData = meas2card->measureAngleCFPT11Pitch; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_weighted_raw_roll: BT.txFrame.paramData = meas2card->measureAngleCFWeightedRawRoll; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_weighted_raw_pitch: BT.txFrame.paramData = meas2card->measureAngleCFWeightedRawPitch; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_weighted_PT01_roll: BT.txFrame.paramData = meas2card->measureAngleCFWeightedPT01Roll; BT.txFrame.numberOfFrac = 0; break;
				case ID_meas_2_card_angle_CF_weighted_PT01_pitch: BT.txFrame.paramData = meas2card->measureAngleCFWeightedPT01Pitch; BT.txFrame.numberOfFrac = 0; break;
                //PID sensor
                case ID_meas_2_card_PID_refsig_X: BT.txFrame.paramData = meas2card->measurePIDRefsigX_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_refsig_Y: BT.txFrame.paramData = meas2card->measurePIDRefsigY_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_refsig_Z: BT.txFrame.paramData = meas2card->measurePIDRefsigZ_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_sensor_X: BT.txFrame.paramData = meas2card->measurePIDSensorX_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_sensor_Y: BT.txFrame.paramData = meas2card->measurePIDSensorY_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_sensor_Z: BT.txFrame.paramData = meas2card->measurePIDSensorZ_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Pout_X: BT.txFrame.paramData = meas2card->measurePIDPoutX_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Pout_Y: BT.txFrame.paramData = meas2card->measurePIDPoutY_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Pout_Z: BT.txFrame.paramData = meas2card->measurePIDPoutZ_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Iout_X: BT.txFrame.paramData = meas2card->measurePIDIoutX_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Iout_Y: BT.txFrame.paramData = meas2card->measurePIDIoutY_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Iout_Z: BT.txFrame.paramData = meas2card->measurePIDIoutZ_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Dout_X: BT.txFrame.paramData = meas2card->measurePIDDoutX_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Dout_Y: BT.txFrame.paramData = meas2card->measurePIDDoutY_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_Dout_Z: BT.txFrame.paramData = meas2card->measurePIDDoutZ_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_FFout_X: BT.txFrame.paramData = meas2card->measurePIDFFoutX_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_FFout_Y: BT.txFrame.paramData = meas2card->measurePIDFFoutY_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_FFout_Z: BT.txFrame.paramData = meas2card->measurePIDFFoutZ_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_U_X: BT.txFrame.paramData = meas2card->measurePIDUX_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_U_Y: BT.txFrame.paramData = meas2card->measurePIDUY_i; BT.txFrame.numberOfFrac = 0; break;
                case ID_meas_2_card_PID_U_Z: BT.txFrame.paramData = meas2card->measurePIDUZ_i; BT.txFrame.numberOfFrac = 0; break;

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
				case ID_loop_tick: SetSteamFlag2(ID_loop_tick); break;

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

int32_t ConvertStrToInt32(volatile buffer_* input)
{
	int8_t first_char = 3;
	int8_t last_char = input->ctr - 2;	// \r\n
	int8_t integer_diff = 0;
	int32_t returnVal = 0;
	bool isNegative = false;

	//if its a negative value raise flag and increment first_char
	if ('-' == input->vector[first_char])
	{
		first_char++;
		isNegative = true;
	}

	//integer calcu
	for (uint8_t pos = first_char; pos < last_char; pos++)
	{
		integer_diff = last_char - pos - 1;
		returnVal += int32_t(pow(10, integer_diff)) * int32_t(input->vector[pos] - '0');
	}

	//if negative
	if (isNegative) returnVal *= -1;

	return returnVal;
}

float ConvertStrToDouble(volatile buffer_* input)
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

date ConvertStrToGlobalTime(const volatile buffer_* input)
{
	date globalTime{};

	globalTime.hour =  (input->vector[3] - '0') * 10 + input->vector[4] - '0';
	globalTime.min =  (input->vector[5] - '0') * 10 + input->vector[6] - '0';
	globalTime.sec =  (input->vector[7] - '0') * 10 + input->vector[8] - '0';

	return globalTime;
}

date ConvertStrToGlobalDate(const volatile buffer_* input)
{
	date globalTime{};

	globalTime.year = (input->vector[3] - '0') * 10 + input->vector[4] - '0';
	globalTime.month = (input->vector[5] - '0') * 10 + input->vector[6] - '0';
	globalTime.day = (input->vector[7] - '0') * 10 + input->vector[8] - '0';

	return globalTime;
}

double ConvertGlobalTime(const date* globalTime)
{
	double output{0.0};

	output  = double(globalTime->sec);
	output += double(globalTime->min)   * 100.0;
	output += double(globalTime->hour)  * 10000.0;

	return output;
}

double ConvertGlobalDate(const date* globalTime)
{
	double output{ 0.0 };

	output = double(globalTime->day);
	output += double(globalTime->month) * 100.0;
	output += double(globalTime->year) * 10000.0;

	return output;
}

bool isFrameEndReceived(void)
{
	return ('\r' == BT.input.vector[BT.input.ctr - 2] && '\n' == BT.input.vector[BT.input.ctr - 1]);
}

void CalcCharAndFillOutput(double value, uint8_t numOfFrac)
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
    // SerialUSB.println(currentCtr);

		for (uint8_t i = 0; i <= currentCtr; i++) //creating integer part
		{
			uint8_t currentInt;

			currentInt = (uint8_t)value; //creat the integer part
      // SerialUSB.println(currentInt);
			BT.output.vector[BT.output.ctr++] = '0' + currentInt; //set correct ASCII char for sending
			value -= (double)currentInt;  //substitute it from the original
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
				value -= (double)currentFrac; //substitute it from the original
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
				value -= (double)currentFrac; //substitute it from the original
			}
		}

	}
}
