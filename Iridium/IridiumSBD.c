//
// Created by Claudio Imberlina on 13/11/2018.
//

#include "includes.h"
#include "IridiumSBD.h"

// Timings
int sbdixInterval;
int atTimeout;
int sendReceiveTimeout;

// State variables
int remainingMessages;
int asleep;
int reentrant;
int sleepPin;
int ringPin;
int msstmWorkaroundRequested;
int ringAlertsEnabled;
int ringAsserted;
unsigned long lastPowerOnTime;

// Unsolicited SBDRING filter technology
const char SBDRING[] = "SBDRING\r\n";
const int FILTERTIMEOUT = 10;
const char *head, *tail;
int nextChar;
void filterSBDRING();
int filteredavailable();
int filteredread();

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
/* itoa:  convert n to characters in s */
void sbd_itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}


long milisec (void)
{
    return (long)MSEC_TIMER;
}


void iridium_send(unsigned char *databuff, int lendatabuff)
{
    int i;
    OS_ERR os_err;
    uint32_t timeout;
    uint8_t ch, err;

    GPIO_SetValue(1, 1<<18);

    //vacio el buffer de recepcion
	timeout = MSEC_TIMER + 300;
	while(CommIsEmpty(COMM3) != TRUE)	{
		ch = CommGetChar(COMM3, 1, &err);
		if( MSEC_TIMER > timeout)
			break;
	}

	//OSTimeDlyHMSM(0, 0, 0, 4, OS_OPT_TIME_HMSM_STRICT, &os_err);
    //n = write(port,(const void *)databuff, (size_t )lendatabuff);
    ComSendBuff(COMM3, databuff, lendatabuff);
}

// Wait for response from previous AT command.  This process terminates when "terminator" string is seen or upon timeout.
// If "prompt" string is provided (example "+CSQ:"), then all characters following prompt up to the next CRLF are
// stored in response buffer for later parsing by caller.
int waitForATResponse(char *response, int responseSize, const char *prompt, const char *terminator)
{
    char tmpbuffer[256];
    int tmpptr = 0;
    OS_ERR os_err;

    long start;
    char c;
    enum {LOOKING_FOR_PROMPT, GATHERING_RESPONSE, LOOKING_FOR_TERMINATOR};

    if (response)
        memset(response, 0, responseSize);

    memset(tmpbuffer, 0, responseSize);

    int matchPromptPos = 0; // Matches chars in prompt
    int matchTerminatorPos = 0; // Matches chars in terminator

    int promptState = prompt ? LOOKING_FOR_PROMPT : LOOKING_FOR_TERMINATOR;


    for (start = milisec(); milisec() - start < 1000UL * atTimeout;)
    {
        if (cancelled())
            return FALSE;

        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);

        while (filteredavailable() > 0)
        {
            c = filteredread();
            tmpbuffer[tmpptr++] = c;

            if (prompt)
            {
                switch (promptState)
                {
                    case LOOKING_FOR_PROMPT:
                        if (c == prompt[matchPromptPos])
                        {
                            ++matchPromptPos;
                            if (prompt[matchPromptPos] == '\0')
                                promptState = GATHERING_RESPONSE;
                        }

                        else
                        {
                            matchPromptPos = c == prompt[0] ? 1 : 0;
                        }

                        break;
                    case GATHERING_RESPONSE: // gathering response from end of prompt to first \r
                        if (response)
                        {
                            if (c == '\r' || responseSize < 2)
                            {
                                promptState = LOOKING_FOR_TERMINATOR;
                            }
                            else
                            {
                                *response++ = c;
                                responseSize--;
                            }
                        }
                        break;
                }
            }

            if (c == terminator[matchTerminatorPos])
            {
                ++matchTerminatorPos;
                if (terminator[matchTerminatorPos] == '\0')
                    return TRUE;
            }
            else
            {
                matchTerminatorPos = c == terminator[0] ? 1 : 0;
            }
        } // while (stream.available() > 0)
    } // timer loop
    return FALSE;
}

int filteredavailable(void)
{
    filterSBDRING();
    return head - tail + (nextChar != -1 ? 1 : 0);
}

int filteredread(void)
{
    filterSBDRING();

    // Use up the queue first
    if (head > tail)
    {
        char c = *tail++;
        if (head == tail)
            head = tail = SBDRING;
        return c;
    }

        // Then the "extra" char
    else if (nextChar != -1)
    {
        char c = (char)nextChar;
        nextChar = -1;
        return c;
    }


    return -1;
}

// Read characters until we find one that doesn't match SBDRING
// If nextChar is -1 it means we are still entertaining a possible
// match with SBDRING\r\n.  Once we find a mismatch, stuff it into
// nextChar.
void filterSBDRING(void)
{
    long start;
    ssize_t bytes_read = -1;
    int i, bytes;
    char c;
    char buff[50];
    uint8_t err;


    //ioctl(port, FIONREAD, &bytes);
    bytes = COMMXrdUsed(COMM3);

    while ((bytes > 0) && (nextChar == -1))
    {
        //read(port, buff, 1);
        //c = buff[0];
        c = CommGetChar(COMM3, 100, &err);

        if (*head != 0 && c == *head) {
            ++head;
            if (*head == 0) {
                SBDRINGSeen();
                head = tail = SBDRING;
            } else {
                // Delay no more than 10 milliseconds waiting for next char in SBDRING
                //ioctl(port, FIONREAD, &bytes);
                bytes = COMMXrdUsed(COMM3);
                for (start = milisec(); bytes == 0 && milisec() - start < FILTERTIMEOUT;)
                    //ioctl(port, FIONREAD, &bytes);
                    bytes = COMMXrdUsed(COMM3);

                // If there isn't one, assume this ISN'T an unsolicited SBDRING
                //ioctl(port, FIONREAD, &bytes);
                bytes = COMMXrdUsed(COMM3);
                if (bytes == 0) // pop the character back into nextChar
                {
                    --head;
                    nextChar = c;
                }
            }
        } else {
            nextChar = c;
        }
        //ioctl(port, FIONREAD, &bytes);
        bytes = COMMXrdUsed(COMM3);
    }
}

void SBDRINGSeen(void)
{
    ringAsserted = TRUE;
}

int cancelled(void)
{

    if (&ISBDCallback != NULL)
        return !ISBDCallback();

    return FALSE;
}

int ISBDCallback(void)
{

    return TRUE;
}

int internalSendReceiveSBD(const char *txTxtMessage, const u_int8_t *txData, int txDataSize, u_int8_t *rxBuffer, int *prxBufferSize)
{

    uint16_t checksum = 0;
    uint8_t tmpdata[8];
    long start;
    int okToProceed, ret;
    uint16_t moCode, moMSN, mtCode, mtMSN, mtLen, mtRemaining;

    OS_ERR os_err;

    if (asleep)
        return ISBD_IS_ASLEEP;

    // Binary transmission?
    if (txData && txDataSize)
    {
        if (txDataSize > ISBD_MAX_MESSAGE_LENGTH)
            return ISBD_MSG_TOO_LONG;

        iridium_send((unsigned char *)"AT+SBDWB=", 9);
        sbd_itoa(txDataSize, (char *)tmpdata);
        iridium_send(tmpdata, strlen((char *)tmpdata));
        iridium_send((unsigned char *)"\r", 1);
        if (!waitForATResponse(NULL, 0, NULL, "READY\r\n"))
            return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;


        for (size_t i=0; i<txDataSize; ++i)
        {
            //write(port,(const void *)(txData + i), 1);
            ComSendBuff(COMM3, (CPU_INT08U *)(txData + i), 1);
            checksum += (uint16_t)txData[i];
        }

        tmpdata[0] = checksum >> 8;
        //write(port,(const void *)(tmpdata), 1);
        ComSendBuff(COMM3, (tmpdata), 1);

        tmpdata[0] = (checksum & 0xFF);
        //write(port,(const void *)(tmpdata), 1);
        ComSendBuff(COMM3, (tmpdata), 1);

        if (!waitForATResponse(NULL, 0, NULL, "0\r\n\r\nOK\r\n"))
            return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
    }

    else // Text transmission
    {
#if TRUE // use long string implementation
        if (txTxtMessage == NULL) // It's ok to have a NULL txtTxtMessage if the transaction is RX only
        {
            iridium_send((unsigned char *)"AT+SBDWT=\r", 10);
            if (!waitForATResponse(NULL, 0, NULL, "OK\r\n"))
                return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
        }
        else
        {
            // remove any embedded \r
            char *p = strchr(txTxtMessage, '\r');
            if (p) *p = 0;
            if (strlen(txTxtMessage) > ISBD_MAX_MESSAGE_LENGTH)
                return ISBD_MSG_TOO_LONG;
            iridium_send((unsigned char *)"AT+SBDWT\r", 9);
            if (!waitForATResponse(NULL, 0, NULL, "READY\r\n"))
                return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
            iridium_send((unsigned char *)txTxtMessage, (int)strlen(txTxtMessage));
            iridium_send((unsigned char *)"\r", 1);
            if (!waitForATResponse(NULL, 0, NULL, "0\r\n\r\nOK\r\n"))
                return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
        }
#else
        iridium_send((unsigned char *)"AT+SBDWT=", 9);
        if (txTxtMessage != NULL) // It's ok to have a NULL txtTxtMessage if the transaction is RX only
            iridium_send((unsigned char *)txTxtMessage, (int)strlen(txTxtMessage));
        iridium_send((unsigned char *)"\r", 1);
        if (!waitForATResponse(NULL, 0, NULL, "OK\r\n"))
            return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
#endif
    }

    // Long SBDIX loop begins here
    for ( start = milisec(); milisec() - start < 1000UL * sendReceiveTimeout;)
    {
        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
        okToProceed = TRUE;
        if (msstmWorkaroundRequested)
        {
            okToProceed = FALSE;
            ret = internalMSSTMWorkaround(&okToProceed);
            if (ret != ISBD_SUCCESS)
                return ret;
        }

        if (okToProceed)
        {
            moCode = 0; moMSN = 0; mtCode = 0; mtMSN = 0; mtLen = 0; mtRemaining = 0;
            ret = doSBDIX(&moCode, &moMSN, &mtCode, &mtMSN, &mtLen, &mtRemaining);
            if (ret != ISBD_SUCCESS)
                return ret;

            if (moCode >= 0 && moCode <= 4) // this range indicates successful return!
            {
                remainingMessages = mtRemaining;
                if (mtCode == 1 && rxBuffer) // retrieved 1 message
                {
                    return doSBDRB(rxBuffer, prxBufferSize);
                }

                else
                {
                    // No data returned
                    if (prxBufferSize)
                        *prxBufferSize = 0;
                }
                return ISBD_SUCCESS;
            }

            else if ((moCode == 12) || (moCode == 14 )|| (moCode == 16)) // fatal failure: no retry
            {
                return ISBD_SBDIX_FATAL_ERROR;
            }

            else // retry
            {
                if (!noBlockWait(sbdixInterval))
                    return ISBD_CANCELLED;
            }
        }

        else // MSSTM check fail
        {
            if (!noBlockWait(ISBD_MSSTM_RETRY_INTERVAL))
                return ISBD_CANCELLED;
        }
    } // big wait loop

    return ISBD_SENDRECEIVE_TIMEOUT;
}

int doSBDIX(uint16_t *moCode, uint16_t *moMSN, uint16_t *mtCode, uint16_t *mtMSN, uint16_t *mtLen, uint16_t *mtRemaining)
{
    char *p;
    int i;
    uint16_t *values[6];
    OS_ERR os_err;

    // Returns xx,xxxxx,xx,xxxxx,xx,xxx
    char sbdixResponseBuf[256];
    iridium_send((unsigned char *)"AT+SBDIX\r", 9);
    OSTimeDlyHMSM(0, 0, 15, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
    //sleep(10);
    if (!waitForATResponse(sbdixResponseBuf, sizeof(sbdixResponseBuf), "+SBDIX: ", "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    //values[6] = { moCode, moMSN, mtCode, mtMSN, mtLen, mtRemaining };
    values[0] = moCode;
    values[1] = moMSN;
    values[2] = mtCode;
    values[3] = mtMSN;
    values[4] = mtLen;
    values[5] = mtRemaining;

    for (i=0; i<6; ++i)
    {
        p = strtok(i == 0 ? sbdixResponseBuf : NULL, ", ");
        if (p == NULL)
            return ISBD_PROTOCOL_ERROR;
        *values[i] = (uint16_t)atol(p);
    }
    return ISBD_SUCCESS;
}

int doSBDRB(uint8_t *rxBuffer, int *prxBufferSize)
{
    int rxOverflow = FALSE;
    long start;
    int bytes;
    unsigned char byte0, byte1;
    uint16_t size;
    uint16_t bytesRead;
    uint16_t checksum;
    uint8_t err;

    OS_ERR os_err;

    iridium_send((unsigned char *)"AT+SBDRB\r", 9);
    if (!waitForATResponse(NULL, 0, NULL, "AT+SBDRB\r")) // waits for its own echo
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    // Time to read the binary data: size[2], body[size], checksum[2]
    start = milisec();
    while (milisec() - start < 1000UL * atTimeout)
    {
        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
        if (cancelled())
            return ISBD_CANCELLED;

        //ioctl(port, FIONREAD, &bytes);
        bytes = COMMXrdUsed(COMM3);
        if (bytes >= 2)
            break;
    }

    //ioctl(port, FIONREAD, &bytes);
    bytes = COMMXrdUsed(COMM3);
    if (bytes < 2)
        return ISBD_SENDRECEIVE_TIMEOUT;

    //read(port, &byte0, 1);
    //read(port, &byte1, 1);

    byte0 = CommGetChar(COMM3, 100, &err);
    byte1 = CommGetChar(COMM3, 100, &err);
    size = (256 * byte0) + byte1;


    for (bytesRead = 0; bytesRead < size;)
    {
        if (cancelled())
            return ISBD_CANCELLED;

        //ioctl(port, FIONREAD, &bytes);
        bytes = COMMXrdUsed(COMM3);
        if (bytes > 0)
        {
            //read(port, &byte0, 1);
            byte0 = CommGetChar(COMM3, 100, &err);
            bytesRead++;
            if (rxBuffer && prxBufferSize)
            {
                if (*prxBufferSize > 0)
                {
                    *rxBuffer++ = byte0;
                    (*prxBufferSize)--;
                }
                else
                {
                    rxOverflow = TRUE;
                }
            }
        }

        if (milisec() - start >= 1000UL * atTimeout)
            return ISBD_SENDRECEIVE_TIMEOUT;
    }

    while (milisec() - start < 1000UL * atTimeout)
    {
        if (cancelled())
            return ISBD_CANCELLED;
        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
        //ioctl(port, FIONREAD, &bytes);
        bytes = COMMXrdUsed(COMM3);
        if (bytes >= 2)
            break;
    }

    //ioctl(port, FIONREAD, &bytes);
    bytes = COMMXrdUsed(COMM3);
    if (bytes < 2)
        return ISBD_SENDRECEIVE_TIMEOUT;

    //read(port, &byte0, 1);
    //read(port, &byte1, 1);
    byte0 = CommGetChar(COMM3, 100, &err);
    byte1 = CommGetChar(COMM3, 100, &err);
    checksum = (256 * byte0) + byte1;



    // Return actual size of returned buffer
    if (prxBufferSize)
        *prxBufferSize = (size_t)size;

    // Wait for final OK
    if (!waitForATResponse(NULL, 0, NULL, "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    return rxOverflow ? ISBD_RX_OVERFLOW : ISBD_SUCCESS;
}

int noBlockWait(int seconds)
{
    long start;
    OS_ERR os_err;

    for (start = milisec(); milisec() - start < 1000UL * seconds;)  {
        if (cancelled())
            return FALSE;
        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }

    return TRUE;
}

int internalMSSTMWorkaround(int *okToProceed)
{
    /*
    According to Iridium 9602 Product Bulletin of 7 May 2013, to overcome a system erratum:

    "Before attempting any of the following commands: +SBDDET, +SBDREG, +SBDI, +SBDIX, +SBDIXA the field application
    should issue the AT command AT-MSSTM to the transceiver and evaluate the response to determine if it is valid or not:

    Valid Response: "-MSSTM: XXXXXXXX" where XXXXXXXX is an eight-digit hexadecimal number.

    Invalid Response: "-MSSTM: no network service"

    If the response is invalid, the field application should wait and recheck system time until a valid response is
    obtained before proceeding.

    This will ensure that the Iridium SBD transceiver has received a valid system time before attempting SBD communication.
    The Iridium SBD transceiver will receive the valid system time from the Iridium network when it has a good link to the
    satellite. Ensuring that the received signal strength reported in response to AT command +CSQ and +CIER is above 2-3 bars
    before attempting SBD communication will protect against lockout.
    */
    char msstmResponseBuf[24];

    iridium_send((unsigned char *)"AT-MSSTM\r", 9);
    if (!waitForATResponse(msstmResponseBuf, sizeof(msstmResponseBuf), "-MSSTM: ", "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    // Response buf now contains either an 8-digit number or the string "no network service"
    *okToProceed = isxdigit(msstmResponseBuf[0]);
    return ISBD_SUCCESS;
}


int internalBegin( void )
{
    int modemAlive, ret;
    unsigned long start;
    char version[8];
    OS_ERR os_err;

    if (asleep)
        return ISBD_ALREADY_AWAKE;

    power(TRUE); // power on

    modemAlive = FALSE;

    unsigned long startupTime = 500; //ms
    for (start = milisec(); milisec() - start < startupTime;)   {
        if (cancelled())
            return ISBD_CANCELLED;
        OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_HMSM_STRICT, &os_err);
        
    }

    // Turn on modem and wait for a response from "AT" command to begin
    for (start = milisec(); !modemAlive && milisec() - start < 1000UL * ISBD_STARTUP_MAX_TIME;)
    {
        iridium_send((unsigned char *)("AT\r\n"), 4);
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
        modemAlive = waitForATResponse(NULL, 0, NULL, "OK\r\n");
        if(modemAlive)
            break;
        if (cancelled())
            return ISBD_CANCELLED;
    }

    if (!modemAlive)
    {
        return ISBD_NO_MODEM_DETECTED;
    }
    IRIDIUM_flag |= IRI_MDMALIVE_FLAG;
    // The usual initialization sequence
    iridium_send((unsigned char *)"ATE1\r", 5);
    if (!waitForATResponse(NULL, 0, NULL, "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
    iridium_send((unsigned char *)"AT&D0\r", 6);
    if (!waitForATResponse(NULL, 0, NULL, "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
    iridium_send((unsigned char *)"AT&K0\r", 6);
    if (!waitForATResponse(NULL, 0, NULL, "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    // Enable or disable RING alerts as requested by user
    // By default they are on if a RING pin was supplied on constructor
    if(ringAlertsEnabled)
        iridium_send((unsigned char *)"AT+SBDMTA=1\r", 12);
    else
        iridium_send((unsigned char *)"AT+SBDMTA=0\r", 12);
    if (!waitForATResponse(NULL, 0, NULL, "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    // Decide whether the internal MSSTM workaround should be enforced on TX/RX
    // By default it is unless the firmware rev is >= TA13001

    ret = getFirmwareVersion(version, sizeof(version));
    if (ret != ISBD_SUCCESS)
    {
        msstmWorkaroundRequested = TRUE;
    }
    else
    {
        if (version[0] == 'T' && version[1] == 'A')
        {
            unsigned long ver = strtoul(version + 2, NULL, 10);
            msstmWorkaroundRequested = ver < ISBD_MSSTM_WORKAROUND_FW_VER;
        }
    }

    // Done!
    return ISBD_SUCCESS;
}

void power(int on)
{
    unsigned long elapsed;
    OS_ERR os_err;

    asleep = !on;

    if (sleepPin == -1)
        return;

    //pinMode(this->sleepPin, OUTPUT);

    if (on)
    {

        //digitalWrite(this->sleepPin, HIGH); // HIGH = awake
        lastPowerOnTime = milisec();
    }

    else
    {
        // Best Practices Guide suggests waiting at least 2 seconds
        // before powering off again
        elapsed = milisec() - lastPowerOnTime;
        if (elapsed < 2000UL)
            //usleep(2000000UL - elapsed*1000);
            OSTimeDlyHMSM(0, 0, 0, (2000UL - elapsed), OS_OPT_TIME_HMSM_STRICT, &os_err);

    }
}

int getFirmwareVersion(char *version, int bufferSize)
{
    OS_ERR os_err;
    
    if (bufferSize < 8)
        return ISBD_RX_OVERFLOW;

    iridium_send((unsigned char *)"AT+CGMR\r", 8);
    OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_HMSM_STRICT, &os_err);
    if (!waitForATResponse(version, bufferSize, "Call Processor Version: ", "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    return ISBD_SUCCESS;
}

int IridiumSBD_begin()
{
    int ret, count;
    OS_ERR os_err;

    if (reentrant)
        return ISBD_REENTRANT;

    count = 0;
    while( count < 3)   {
        reentrant = TRUE;
        ret = internalBegin();
        reentrant = FALSE;
        count++;
        if (ret == ISBD_SUCCESS)
            break;

        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }
    

    // Absent a successful startup, keep the device turned off
    if (ret != ISBD_SUCCESS)   {
        power(FALSE);
        ;
    } else
    {
        IRIDIUM_flag |= IRI_IRIDIUMOK_FLAG;
    }
    

    return ret;
}

// Transmit a binary message
int sendSBDBinary(const uint8_t *txData, int txDataSize)
{
    int ret;

    if (reentrant)
        return ISBD_REENTRANT;

    reentrant = TRUE;
    ret = internalSendReceiveSBD(NULL, txData, txDataSize, NULL, NULL);
    reentrant = FALSE;
    return ret;
}

// Transmit and receive a binary message
int sendReceiveSBDBinary(const uint8_t *txData, int txDataSize, uint8_t *rxBuffer, int *rxBufferSize)
{
    int ret;

    if (reentrant)
        return ISBD_REENTRANT;

    reentrant = TRUE;
    ret = internalSendReceiveSBD(NULL, txData, txDataSize, rxBuffer, rxBufferSize);
    reentrant = FALSE;
    return ret;
}

// Transmit a text message
int sendSBDText(const char *message)
{
    int ret;

    if (reentrant)
        return ISBD_REENTRANT;

    reentrant = TRUE;
    ret = internalSendReceiveSBD(message, NULL, 0, NULL, NULL);
    reentrant = FALSE;
    return ret;
}

// Transmit a text message and receive reply
int sendReceiveSBDText(const char *message, uint8_t *rxBuffer, int *rxBufferSize)
{
    int ret;

    if (reentrant)
        return ISBD_REENTRANT;

    reentrant = TRUE;
    ret = internalSendReceiveSBD(message, NULL, 0, rxBuffer, rxBufferSize);
    reentrant = FALSE;
    return ret;
}

// High-level wrapper for AT+CSQ
int getSignalQuality(int *quality)
{
    int ret;

    if (reentrant)
        return ISBD_REENTRANT;

    reentrant = TRUE;
    ret = internalGetSignalQuality(quality);
    reentrant = FALSE;
    return ret;
}

int internalGetSignalQuality(int *quality)
{
    OS_ERR os_err;
    char csqResponseBuf[2];

    if (asleep)
        return ISBD_IS_ASLEEP;

    iridium_send((unsigned char *)"AT+CSQ\r", 7);
    //usleep(10000000);
    OSTimeDlyHMSM(0, 0, 10, 0, OS_OPT_TIME_HMSM_STRICT, &os_err);
    if (!waitForATResponse(csqResponseBuf, sizeof(csqResponseBuf), "+CSQ:", "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    if (isdigit(csqResponseBuf[0]))
    {
        *quality = atoi(csqResponseBuf);
        return ISBD_SUCCESS;
    }

    return ISBD_PROTOCOL_ERROR;
}

int internalSleep( void )
{
    if (asleep)
        return ISBD_IS_ASLEEP;

#if false // recent research suggest this is not what you should do when just sleeping
    // Best Practices Guide suggests this before shutdown
   iridium_send((unsigned char *)"AT*F\r", 5);

   if (!waitForATResponse())
      return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
#endif

    return ISBD_SUCCESS;
}

// Gracefully put device to lower power mode (if sleep pin provided)
int IridiumSBD_sleep(void)
{
    int ret;

    if (reentrant)
        return ISBD_REENTRANT;

    if (sleepPin == -1)
        return ISBD_NO_SLEEP_PIN;

    reentrant = TRUE;
    ret = internalSleep();
    reentrant = FALSE;

    if (ret == ISBD_SUCCESS)
        power(TRUE); // power off
    return ret;
}

// Return sleep state
int isAsleep( void )
{
    return asleep;
}

// Return number of pending messages
int getWaitingMessageCount( void )
{
    return remainingMessages;
}

int getSystemTime(struct tm *tmptr)
{
    char msstmResponseBuf[24];
    struct tm epoch_start;
    unsigned long ticks_since_epoch;
    unsigned long secs_since_epoch;
    unsigned long small_ticks;
    time_t epoch_time;


    iridium_send((unsigned char *)"AT-MSSTM\r", 9);

    if (!waitForATResponse(msstmResponseBuf, sizeof(msstmResponseBuf), "-MSSTM: ", "OK\r\n"))
        return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

    if (!isxdigit(msstmResponseBuf[0]))
        return ISBD_NO_NETWORK;

    // Latest epoch began at May 11, 2014, at 14:23:55 UTC.

    epoch_start.tm_year = 2014 - 1900;
    epoch_start.tm_mon = 5 - 1;
    epoch_start.tm_mday = 11;
    epoch_start.tm_hour = 14;
    epoch_start.tm_min = 23;
    epoch_start.tm_sec = 55;

    ticks_since_epoch = strtoul(msstmResponseBuf, NULL, 16);

    /* Strategy: we'll convert to seconds by finding the largest number of integral
    seconds less than the equivalent ticks_since_epoch. Subtract that away and
    we'll be left with a small number that won't overflow when we scale by 90/1000.

    Many thanks to Scott Weldon for this suggestion.
    */
    secs_since_epoch = (ticks_since_epoch / 1000) * 90;
    small_ticks = ticks_since_epoch - (secs_since_epoch / 90) * 1000;
    secs_since_epoch += small_ticks * 90 / 1000;

    epoch_time = mktime(&epoch_start);
    time_t now = epoch_time + secs_since_epoch;
    memcpy(tmptr, localtime(&now), sizeof (struct tm));

    return ISBD_SUCCESS;
}

// Define capacitor recharge times
void setPowerProfile(POWERPROFILE profile) // 0 = direct connect (default), 1 = USB
{
    switch(profile)
    {
        case DEFAULT_POWER_PROFILE:
            sbdixInterval = ISBD_DEFAULT_SBDIX_INTERVAL;
            break;

        case USB_POWER_PROFILE:
            sbdixInterval = ISBD_USB_SBDIX_INTERVAL;
            break;
    }
}

// Tweak AT timeout
void adjustATTimeout(int seconds)
{
    atTimeout = seconds;
}

// Tweak Send/Receive SBDIX process timeout
void adjustSendReceiveTimeout(int seconds)
{
    sendReceiveTimeout = seconds;
}

void useMSSTMWorkaround(int useWorkAround) // true to use workaround from Iridium Alert 5/7
{
    msstmWorkaroundRequested = useWorkAround;
}

void enableRingAlerts(int enable) // true to enable SBDRING alerts and RING signal pin
{
    ringAlertsEnabled = enable;
    if (enable)
        ringAsserted = FALSE;
}

int hasRingAsserted(void)
{
    int ret;

    if (!ringAlertsEnabled)
        return FALSE;

    if (!reentrant)
    {
        // It's possible that the SBDRING message comes while we're not doing anything
        filterSBDRING();
    }

    ret = ringAsserted;
    ringAsserted = FALSE;
    return ret;
}


uint32_t    HBIRI_timer, hbiri_time, iphbiri_timer;
uint32_t    IRIDIUM_flag;
uint8_t     FSMIRI_state;
uint8_t     IRI_mode;

void fsm_alrm_iridium( void )
{
    static int count_OK, count_NG, retval, signalquality, count_signal_NG, modemAlive;
    static int count_retries;
    static uint32_t FSMIRI_timer, modemAlive_timer;

    //-----------------------------------------------------------------------
    //Chequeo de condiciones para tirar alarmas
    IRIDIUM_flag &= ~IRI_SNDCONDOK_FLAG;
    switch(IRI_mode)    {
        case 0:
            IRIDIUM_flag &= ~IRI_SNDCONDOK_FLAG;
            break;
        case 1:
            if(IRIDIUM_flag & IRI_IPNG_FLAG)    {
                IRIDIUM_flag |= IRI_SNDCONDOK_FLAG;
            }
            break;
        case 2:
            if((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX))  {
                IRIDIUM_flag |= IRI_SNDCONDOK_FLAG;
            }
            break;
        case 3:
            if(((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)) && (IRIDIUM_flag & IRI_IPNG_FLAG))  {
                IRIDIUM_flag |= IRI_SNDCONDOK_FLAG;
            }
            break;
        case 4:
            IRIDIUM_flag |= IRI_SNDCONDOK_FLAG;
            break;
        case 5:
            if(((SysFlag1 & PREVE_CENTRAL_TX) || (SysFlag1 & PREVE_CENTRAL_RX)) && (IRIDIUM_flag & IRI_IPNG_FLAG) && (IRIDIUM_flag & IRI_GPRSNG_FLAG))  {
                IRIDIUM_flag |= IRI_SNDCONDOK_FLAG;
            }
            break;
        default:
            IRIDIUM_flag &= ~IRI_SNDCONDOK_FLAG;
            break;
    }
    //-----------------------------------------------------------------------

    //--------------------------------------
    // para prueba con RVB
    IRIDIUM_flag |= IRI_ALWAYSHAB_FLAG;
    IRIDIUM_flag |= IRI_SNDCONDOK_FLAG;
    //--------------------------------------

    switch(FSMIRI_state)    {
        case FSMIRI_IDLE:
            if((IRIDIUM_flag & IRI_SENDALRM_FLAG) && (IRIDIUM_flag & IRI_USEIRIDIUM_FLAG))   {
                IRIDIUM_flag &= ~IRI_SENDALRM_FLAG;
                if(IRIDIUM_flag & IRI_SNDCONDOK_FLAG)   {
                    if((IRIDIUM_flag & IRI_ALWAYSHAB_FLAG) || (IRIDIUM_flag & IRI_RFDWNHAB_FLAG) || (IRIDIUM_flag & IRI_IPDWNHAB_FLAG) || ((IRIDIUM_flag & IRI_BOTHDWNHAB_FLAG))) {
                        FSMIRI_state = FSMIRI_PRESENDING;
                        FSMIRI_timer = SEC_TIMER - 1;
                        count_OK = 0;
                        count_NG = 0;
                        count_retries = 0;
                        IRIDIUM_flag &= ~IRI_SENDNG_FLAG;
                        IRIDIUM_flag &= ~IRI_SENDOK_FLAG;
                        IRIDIUM_flag |= IRI_SENDING_FLAG;
                        IRIDIUM_flag |= IRI_PENDING_FLAG;
                        //SendIRI_ALRM_packet();
                    }
                }
            } else
            if((IRIDIUM_flag & IRI_SENDHB_FLAG) && (IRIDIUM_flag & IRI_USEIRIDIUM_FLAG))  {
                IRIDIUM_flag &= ~IRI_SENDHB_FLAG;
                FSMIRI_state = FSMIRI_PRESENDING;
                FSMIRI_timer = SEC_TIMER - 1;
                count_OK = 0;
                count_NG = 0;
                count_retries = 0;
                IRIDIUM_flag &= ~IRI_SENDNG_FLAG;
                IRIDIUM_flag &= ~IRI_SENDOK_FLAG;
                IRIDIUM_flag |= IRI_SENDING_FLAG;
                //SendIRI_ALRM_packet();
            } else
            if((HBIRI_timer < SEC_TIMER) && (IRIDIUM_flag & IRI_USEIRIDIUM_FLAG)) {
                hbiri_time = 47;
                HBIRI_timer = SEC_TIMER + hbiri_time;
                FSMIRI_state = FSMIRI_CHKIRIOK;
                modemAlive_timer = SEC_TIMER + 1;
                iridium_send((unsigned char *)("AT\r\n"), 4);
            }
            // else if( (iphbiri_timer < SEC_TIMER)  && (IRIDIUM_flag & IRI_USEIRIDIUM_FLAG))  {
            //     FSMIRI_state = FSMIRI_CHKSIGQTY;
            //     iphbiri_timer = SEC_TIMER + 15*60;
            //     signalquality = -1;
            //     count_OK = 0;
            //     count_NG = 0;
            //     retval = getSignalQuality(&signalquality);
            // }
            break;
        //----------------------------------------------------------------------------
        case FSMIRI_PRESENDING:
            if(SEC_TIMER > FSMIRI_timer)    {
                FSMIRI_timer = SEC_TIMER + 10;
                signalquality = -1;
                retval = getSignalQuality(&signalquality);
                if(signalquality >= 2) {
                    SendIRI_ALRM_packet();
                    FSMIRI_state = FSMIRI_SENDING;
                    FSMIRI_timer = SEC_TIMER + 20;
                } else {
                    count_retries++;
                }
            } else
            if(count_retries > 60)  {   //lo intentamos durante 10 minutos
                logCidEvent(account, 1, 997, 0, 1);         //problema enviando paquete iridium
                FSMIRI_state = FSMIRI_IDLE;
            }
            break;
        case FSMIRI_SENDING:
            if(SEC_TIMER > FSMIRI_timer)    {
                FSMIRI_timer = SEC_TIMER - 1;
                FSMIRI_state = FSMIRI_PRESENDING;
            } else
            if(IRIDIUM_flag & IRI_SENDOK_FLAG)  {
                IRIDIUM_flag &= ~IRI_SENDOK_FLAG;
                count_OK++;
                if(count_OK < 2)   {
                    FSMIRI_timer = SEC_TIMER - 1;
                    FSMIRI_state = FSMIRI_PRESENDING;
                } else  {
                    logCidEvent(account, 1, 998, 0, count_retries); 
                    FSMIRI_state = FSMIRI_IDLE;
                    IRIDIUM_flag &= ~IRI_SENDNG_FLAG;
                    IRIDIUM_flag &= ~IRI_PENDING_FLAG;
                } 
            } else if(IRIDIUM_flag & IRI_SENDNG_FLAG)   {
                IRIDIUM_flag &= ~IRI_SENDNG_FLAG;
                count_NG++;
                FSMIRI_timer = SEC_TIMER - 1;
                FSMIRI_state = FSMIRI_PRESENDING;
            }
            break;
        //----------------------------------------------------------------------------
        // case FSMIRI_SENDING:
        //     if( (SEC_TIMER > FSMIRI_timer) && (!(IRIDIUM_flag & IRI_SENDING_FLAG)))   {
        //         IRIDIUM_flag &= ~IRI_SENDNG_FLAG;
        //         IRIDIUM_flag &= ~IRI_SENDOK_FLAG;
        //         IRIDIUM_flag |= IRI_SENDING_FLAG;
        //         SendIRI_ALRM_packet();
        //     }
        //     if(IRIDIUM_flag & IRI_SENDOK_FLAG)  {
        //         IRIDIUM_flag &= ~IRI_SENDOK_FLAG;
        //         IRIDIUM_flag &= ~IRI_SENDING_FLAG;
        //         FSMIRI_timer = SEC_TIMER + 60;
        //         count_OK++;
        //     } else if(IRIDIUM_flag & IRI_SENDNG_FLAG)   {
        //         IRIDIUM_flag &= ~IRI_SENDNG_FLAG;
        //         IRIDIUM_flag &= ~IRI_SENDING_FLAG;
        //         FSMIRI_timer = SEC_TIMER + 60;
        //         count_NG++;
        //     } else 
        //     if((HBIRI_timer < SEC_TIMER) && (IRIDIUM_flag & IRI_USEIRIDIUM_FLAG)) {
        //         hbiri_time = 47;
        //         HBIRI_timer = SEC_TIMER + hbiri_time;
        //         FSMIRI_state = FSMIRI_CHKIRIOK2;
        //         modemAlive_timer = SEC_TIMER + 1;
        //         iridium_send((unsigned char *)("AT\r\n"), 4);
        //     }
        //     if( (count_OK >= 2) && (count_NG < 10)) {
        //         FSMIRI_state = FSMIRI_IDLE;
        //     }
        //     if(count_NG >= 10)  { 
        //         logCidEvent(account, 1, 997, 0, 1);         //problema enviando paquete iridium
        //         FSMIRI_state = FSMIRI_IDLE;
        //     }
        //     break;
        case FSMIRI_CHKSIGQTY:
            if(count_NG > 4)    {
                logCidEvent(account, 1, 997, 0, 2);         //problema de comunicacion con el iridium
                FSMIRI_state = FSMIRI_IDLE;
            }
            if(retval == ISBD_SUCCESS)  {
                if(signalquality < 2)   {
                    count_signal_NG++;
                    if(count_signal_NG > 4) {
                        logCidEvent(account, 1, 997, 0, 3);     //nivel de señal bajo
                        FSMIRI_state = FSMIRI_IDLE;
                        count_signal_NG = 0;
                    } else {
                        FSMIRI_state = FSMIRI_CHKSIGQTYWAIT;
                        FSMIRI_timer = SEC_TIMER + 5*60;
                    }
                } else {
                    count_signal_NG = 0;
                    FSMIRI_state = FSMIRI_IDLE;
                }

            } else {
                count_NG++;
                signalquality = -1;
                retval = getSignalQuality(&signalquality);
            }
            break;
        case FSMIRI_CHKSIGQTYWAIT:
            if(SEC_TIMER > FSMIRI_timer)    {
                FSMIRI_state = FSMIRI_CHKSIGQTY;
                signalquality = -1;
                retval = getSignalQuality(&signalquality);
            }
            break;
        case FSMIRI_CHKIRIOK:
            if(SEC_TIMER > modemAlive_timer)    {
                modemAlive = waitForATResponse(NULL, 0, NULL, "OK\r\n");
                if(modemAlive)  {
                    IRIDIUM_flag |= IRI_MDMALIVE_FLAG;
                } else {
                    IRIDIUM_flag &= ~IRI_MDMALIVE_FLAG;
                }
                FSMIRI_state = FSMIRI_IDLE;
                hbiri_time = 47;
                HBIRI_timer = SEC_TIMER + hbiri_time;
            }
            break;
        case FSMIRI_CHKIRIOK2:
            if(SEC_TIMER > modemAlive_timer)    {
                modemAlive = waitForATResponse(NULL, 0, NULL, "OK\r\n");
                if(modemAlive)  {
                    IRIDIUM_flag |= IRI_MDMALIVE_FLAG;
                } else {
                    IRIDIUM_flag &= ~IRI_MDMALIVE_FLAG;
                }
                FSMIRI_state = FSMIRI_SENDING;
                hbiri_time = 47;
                HBIRI_timer = SEC_TIMER + hbiri_time;
            }
            break;
        default:
            FSMIRI_state = FSMIRI_IDLE;
            break;
    }
}

void SendIRI_ALRM_packet( void )
{
    int i, retval;
    uint32_t checksum;
    uint8_t binarydata[16];

    binarydata[0] = (uint8_t)((account >> 8) & 0x00FF);
    binarydata[1] = (uint8_t)(account & 0x00FF);
    binarydata[2] = BaseAlarmPkt_numabo;
    binarydata[3] = BaseAlarmPkt_alarm;
    binarydata[4] = (BaseAlarmPkt_estado_dispositivos & 0xF8);
    binarydata[5] = BaseAlarmPkt_memoria_dispositivos;

    binarydata[6] = 0x00;
    if(SysFlag1 & PREVE_CENTRAL_TX)
        binarydata[6] = 0xAA;
    else if(SysFlag1 & PREVE_CENTRAL_RX)
        binarydata[6] = 0xAB;
    else if((SysFlag1 & PREVE_CENTRAL_TX) && (SysFlag1 & PREVE_CENTRAL_RX))
        binarydata[6] = 0xAC;


    checksum = 0;
    for(i = 0; i < 7; i++)  {
        checksum += binarydata[i];
    }
    binarydata[7] = (uint8_t)( checksum & 0x00FF);

    retval = sendSBDBinary(binarydata, 8);

    if (retval != ISBD_SUCCESS) {
        IRIDIUM_flag |= IRI_SENDNG_FLAG;
        IRIDIUM_flag &= ~IRI_SENDOK_FLAG;
    } else  {
        IRIDIUM_flag |= IRI_SENDOK_FLAG;
        IRIDIUM_flag &= ~IRI_SENDNG_FLAG;
    }
}