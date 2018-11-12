/* Arduino Includes */

/* ADL Includes */

#include "adl.h"

#include "integer-param.h"

#include "digital-output.h"
#include "adl-oneshot-timer.h"

/* Application Includes */

/* Defines, typedefs, constants */

static const uint8_t RELAY_PINS[] = {3,4,5,6,7};

/* Private Variables */

static DigitalOutput * s_pOutputs[5] = {NULL};
static IntegerParam * s_pOnTime = NULL;
static IntegerParam * s_pProgramRFID = NULL;

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
    // TODO: read RFID and return true on match
    return false;
}

static void run_output(uint8_t i)
{
    if (s_timers[i].is_running())
    {
        if (s_timers[i].check_and_reset())
        {
            s_pOutputs[i]->set(false);
        }
    }
    else
    {
        if (check_rfid(i))
        {
            s_pOutputs[i]->set(true);
            s_timers[i].start(s_pOnTime->get());
        }
    }
}

static void check_program_flag(uint8_t i)
{
    int32_t to_program = s_pProgramRFID->get();
    if (to_program == (i+1))
    {
        // TODO: get and store new RFID for this output
    }
}

/* ADL Functions */

void adl_custom_setup(DeviceBase * pdevices[], int ndevices, ParameterBase * pparams[], int nparams)
{
    (void)nparams; (void)ndevices;

    s_pOutputs[0] = (DigitalOutput*)pdevices[0];
    s_pOutputs[1] = (DigitalOutput*)pdevices[1];
    s_pOutputs[2] = (DigitalOutput*)pdevices[2];
    s_pOutputs[3] = (DigitalOutput*)pdevices[3];
    s_pOutputs[4] = (DigitalOutput*)pdevices[4];

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
