#include "LoRa.h"

#include "lora.h"

// We're calling C++ functions from pure C LoRaWAN
// stack. So we need to create a bridge to store the _this
static void *this_ptr = nullptr;

static LoRaMainCallback_t LoRaMainCallbacks = {
	HW_GetBatteryLevel, HW_GetTemperatureLevel,
	HW_GetUniqueId,		HW_GetRandomSeed,
	LORA_RxData,		LORA_HasJoined,
	LORA_ConfirmClass};

/* !
 *Initialises the Lora Parameters
 */
static LoRaParam_t LoRaParamInit = {LORAWAN_ADR_STATE,
									LORAWAN_DEFAULT_DATA_RATE,
									LORAWAN_PUBLIC_NETWORK, JOINREQ_NBTRIALS};

LoRa::LoRa() { this_ptr = this; }

bool LoRa::init() {
	/* Configure the Lora Stack*/
	LORA_Init(&LoRaMainCallbacks, &LoRaParamInit);
	return true;
}

bool LoRa::sendPayload(void) { return true; }

void LORA_ConfirmClass(DeviceClass_t Class) {
	PRINTF("switch to class %c done\n\r", "ABC"[Class]);

	/* Optionnal */
	/* informs the server that switch has occurred ASAP*/
	AppData.BuffSize = 0;
	AppData.Port = LORAWAN_APP_PORT;

	LORA_send(&AppData, LORAWAN_UNCONFIRMED_MSG);
}
