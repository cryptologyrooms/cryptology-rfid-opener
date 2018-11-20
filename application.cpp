/* Arduino Includes */

#include <MFRC522.h>

/* ADL Includes */

#include "adl.h"

#include "integer-param.h"
#include "string-param.h"

#include "digital-output.h"
#include "adl-oneshot-timer.h"
#include "rfid-rc522.h"

/* Application Includes */

/* Defines, typedefs, constants */

static const uint8_t RELAY_PINS[] = {3,4,5,6,7};

/* Private Variables */

static DigitalOutput * s_pOutputs[5] = {NULL};
static RFID_RC522 * s_pRFID = NULL;

static IntegerParam * s_pOnTime = NULL;
static IntegerParam * s_pProgramRFID = NULL;

static StringParam * s_pStoredRFIDs[5] = {NULL};

static ADLOneShotTimer s_timers[5] = {
    ADLOneShotTimer(3000),
    ADLOneShotTimer(3000),
    ADLOneShotTimer(3000),
    ADLOneShotTimer(3000),
    ADLOneShotTimer(3000),
};

/* Private Functions */

static bool check_rfid(uint8_t i)
{
    char uuid1[20] = {NULL};
    char uuid2[20] = {NULL};
    int len1, len2;

    len1 = s_pRFID->get(uuid1);

    s_pStoredRFIDs[i]->get(uuid2);
    len2 = strlen(uuid2);

    return strncmp(uuid1, uuid2, max(len1, len2)) == 0;
}

static void run_output(uint8_t i)
{
    if (s_timers[i].is_running())
    {
        if (s_timers[i].check_and_reset())
        {
            adl_logln(LOG_APP, "Timer %d expired", i);
            s_pOutputs[i]->set(false);
        }
    }
    else
    {
        if (check_rfid(i))
        {
            adl_logln(LOG_APP, "Timer %d started", i);
            s_pOutputs[i]->set(true);
            s_timers[i].start(s_pOnTime->get());
        }
    }
}

static void check_program_flag(uint8_t i)
{
    int32_t to_program = s_pProgramRFID->get();
    char uuid[20];
    uint8_t uuid_length = 0;

    if (to_program == (i+1))
    {
        adl_logln(LOG_APP, "Waiting for RFID %d", to_program);
        while(uuid_length == 0)
        {
            uuid_length = s_pRFID->get(uuid);
            if (uuid_length)
            {
                adl_logln(LOG_APP, "Saved RFID %lu: <%s>", to_program, uuid);
                s_pStoredRFIDs[to_program-1]->set(uuid);
                s_pStoredRFIDs[to_program-1]->save();
            }
        }
        s_pProgramRFID->set(0);
    }
}

/* ADL Functions */

void adl_custom_setup(DeviceBase * pdevices[], int ndevices, ParameterBase * pparams[], int nparams)
{
    (void)nparams; (void)ndevices;

    char uuid[20];
    for (uint8_t i=0; i<5; i++)
    {
        s_pOutputs[i] = (DigitalOutput*)pdevices[i];
        s_pStoredRFIDs[i] = (StringParam*)pparams[i+2];
        s_pStoredRFIDs[i]->get(uuid);
        if (strlen(uuid))
        {
            adl_logln(LOG_APP, "Saved RFID %u: <%s>", i+1, uuid);
        }
        else
        {
            adl_logln(LOG_APP, "No saved RFID %u", i+1);
        }
    }

    s_pRFID = (RFID_RC522*)pdevices[5];

    s_pOnTime = (IntegerParam*)pparams[0];
    s_pProgramRFID = (IntegerParam*)pparams[1];

    
}

void adl_custom_loop(DeviceBase * pdevices[], int ndevices, ParameterBase * pparams[], int nparams)
{
    (void)pdevices; (void)pparams; (void)ndevices; (void)nparams;

    for (uint8_t i=0; i<5; i++)
    {
        run_output(i);
        check_program_flag(i);
    }
}
