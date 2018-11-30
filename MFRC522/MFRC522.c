#include <tinyara/spi/spi.h>
#include <stdio.h>
#include "mfrc522.h"

typedef unsigned char uchar;

struct spi_dev_s *spi_dev;
int freq = 1000000;
int bits = 8;
int conf = 0;

static char spi_read(int port, int addr)
{
    unsigned char buf[2];
    buf[0] = addr | 0x80;

    if (spi_dev != 0x0)
    {
        SPI_LOCK(spi_dev, true);

        SPI_SETFREQUENCY(spi_dev, freq);
        SPI_SETBITS(spi_dev, bits);
        SPI_SETMODE(spi_dev, conf);

        SPI_SELECT(spi_dev, port, true);
        SPI_RECVBLOCK(spi_dev, buf, 2);
        SPI_SELECT(spi_dev, port, false);

        SPI_LOCK(spi_dev, false);
    }

    return buf[1];
}

static void spi_write(int port, int addr, char value)
{
    unsigned char buf[2];
    buf[0] = addr;
    buf[1] = value;

    if (spi_dev != 0x0)
    {
        SPI_LOCK(spi_dev, true);

        SPI_SETFREQUENCY(spi_dev, freq);
        SPI_SETBITS(spi_dev, bits);
        SPI_SETMODE(spi_dev, conf);

        SPI_SELECT(spi_dev, port, true);
        SPI_SNDBLOCK(spi_dev, buf, 2);
        SPI_SELECT(spi_dev, port, false);

        SPI_LOCK(spi_dev, false);
    }
}

void Write_MFRC522(uchar addr, uchar val)
{
    addr = (addr << 1) & 0x7E;
    spi_write(0, addr, val);
}

uchar Read_MFRC522(uchar addr)
{
    addr = ((addr << 1) & 0x7E) | 0x80;
    return spi_read(0, addr);
}

void SetBitMask(uchar reg, uchar mask)
{
    uchar temp;
    temp = Read_MFRC522(reg);
    Write_MFRC522(reg, temp | mask);
}

void ClearBitMask(uchar reg, uchar mask)
{
    uchar temp;
    temp = Read_MFRC522(reg);
    Write_MFRC522(reg, temp & (~mask));
}

void AntennaOn()
{
    uchar temp;
    temp = Read_MFRC522(TxControlReg);
    if (!(temp & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

void AntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}

void mfrc522_init()
{
    spi_dev = up_spiinitialize(0);
    //reset mfrc522
    Write_MFRC522(CommandReg, PCD_RESETPHASE);

    Write_MFRC522(TModeReg, 0x8D);      //Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    Write_MFRC522(TPrescalerReg, 0x3E); //TModeReg[3..0] + TPrescalerReg
    Write_MFRC522(TReloadRegL, 30);
    Write_MFRC522(TReloadRegH, 0);

    Write_MFRC522(TxAutoReg, 0x40); //100%ASK
    Write_MFRC522(ModeReg, 0x3D);

    AntennaOn();
}

uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
    uchar status = MI_ERR;
    uchar irqEn = 0x00;
    uchar waitIRq = 0x00;
    uchar lastBits;
    uchar n;
    uint i;

    switch (command)
    {
    case PCD_AUTHENT: //verify card password
    {
        irqEn = 0x12;
        waitIRq = 0x10;
        break;
    }
    case PCD_TRANSCEIVE: //send data in the FIFO
    {
        irqEn = 0x77;
        waitIRq = 0x30;
        break;
    }
    default:
        break;
    }

    Write_MFRC522(CommIEnReg, irqEn | 0x80); //Allow interruption
    ClearBitMask(CommIrqReg, 0x80);          //Clear all the interrupt bits
    SetBitMask(FIFOLevelReg, 0x80);          //FlushBuffer=1, FIFO initilizate

    Write_MFRC522(CommandReg, PCD_IDLE); //NO action;cancel current command  ???

    //write data into FIFO
    for (i = 0; i < sendLen; i++)
    {
        Write_MFRC522(FIFODataReg, sendData[i]);
    }

    //procceed it
    Write_MFRC522(CommandReg, command);
    if (command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80); //StartSend=1,transmission of data starts
    }

    //waite receive data is finished
    i = 2000; //i should adjust according the clock, the maxium the waiting time should be 25 ms???
    do
    {
        //CommIrqReg[7..0]
        //Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = Read_MFRC522(CommIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    ClearBitMask(BitFramingReg, 0x80); //StartSend=0

    if (i != 0)
    {
        if (!(Read_MFRC522(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR; //??
            }

            if (command == PCD_TRANSCEIVE)
            {
                n = Read_MFRC522(FIFOLevelReg);
                lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits)
                {
                    *backLen = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *backLen = n * 8;
                }

                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAX_LEN)
                {
                    n = MAX_LEN;
                }

                //read the data from FIFO
                for (i = 0; i < n; i++)
                {
                    backData[i] = Read_MFRC522(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    //SetBitMask(ControlReg,0x80);           //timer stops
    //Write_MFRC522(CommandReg, PCD_IDLE);

    return status;
}

uchar MFRC522_Request(uchar reqMode, uchar *TagType)
{
    uchar status;
    uint backBits; //the data bits that received

    Write_MFRC522(BitFramingReg, 0x07); //TxLastBists = BitFramingReg[2..0] ???

    TagType[0] = reqMode;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

    if ((status != MI_OK) || (backBits != 0x10))
    {
        status = MI_ERR;
    }

    return status;
}

uchar MFRC522_Anticoll(uchar *serNum)
{
    uchar status;
    uchar i;
    uchar serNumCheck = 0;
    uint unLen;

    //ClearBitMask(Status2Reg, 0x08);   //TempSensclear
    //ClearBitMask(CollReg,0x80);     //ValuesAfterColl
    Write_MFRC522(BitFramingReg, 0x00); //TxLastBists = BitFramingReg[2..0]

    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
    {
        //Verify card serial number
        for (i = 0; i < 4; i++)
        {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[i])
        {
            status = MI_ERR;
        }
    }

    //SetBitMask(CollReg, 0x80);    //ValuesAfterColl=1

    return status;
}

void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData)
{
    uchar i, n;

    ClearBitMask(DivIrqReg, 0x04);  //CRCIrq = 0
    SetBitMask(FIFOLevelReg, 0x80); //Clear FIFO pointer
    //Write_MFRC522(CommandReg, PCD_IDLE);

    //Write data into FIFO
    for (i = 0; i < len; i++)
    {
        Write_MFRC522(FIFODataReg, *(pIndata + i));
    }
    Write_MFRC522(CommandReg, PCD_CALCCRC);

    //waite CRC caculation to finish
    i = 0xFF;
    do
    {
        n = Read_MFRC522(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04)); //CRCIrq = 1

    //read CRC caculation result
    pOutData[0] = Read_MFRC522(CRCResultRegL);
    pOutData[1] = Read_MFRC522(CRCResultRegM);
}

void MFRC522_Halt(void)
{
    uchar status;
    uint unLen;
    uchar buff[4];

    buff[0] = PICC_HALT;
    buff[1] = 0;
    CalulateCRC(buff, 2, &buff[2]);

    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}
