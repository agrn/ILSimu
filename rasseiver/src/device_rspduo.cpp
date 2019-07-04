#include "device_rspduo.hpp"

#include <iostream>

#define RSPDUO_OPERATION(FN, ...)					\
	do {								\
		auto result {static_cast<sdrplay_api_ErrT> (FN(__VA_ARGS__))}; \
		if (result != sdrplay_api_Success) {				\
			std::cerr << #FN "() failed: "			\
				  << sdrplay_api_GetErrorString(result)		\
				  << std::endl;				\
			throw std::runtime_error {sdrplay_api_GetErrorString(result)}; \
		}							\
	} while (0)

RSPDuo::RSPDuo(unsigned int frequency, unsigned int sample_rate) {

	float ver {};

	unsigned int nb;

	// Open API
	RSPDUO_OPERATION(sdrplay_api_Open,);


	// Enable debug logging output
	RSPDUO_OPERATION(sdrplay_api_DebugEnable,nullptr,sdrplay_api_DbgLvl_Verbose);

	// Check API versions match
	RSPDUO_OPERATION(sdrplay_api_ApiVersion,&ver);

	if (ver != SDRPLAY_API_VERSION){
		printf("API version don't match (local=%.2f dll=%.2f)\n", SDRPLAY_API_VERSION, ver);

	}
	// Lock API
	sdrplay_api_LockDeviceApi();

	// Fetch list of available devices
	RSPDUO_OPERATION(sdrplay_api_GetDevices,devs,&nb,sizeof(devs)/(sizeof(sdrplay_api_DeviceT)));


	printf("MaxDevs=%lu NumDevs=%d\n", sizeof(devs) / sizeof(sdrplay_api_DeviceT), nb);
	if (nb> 0)
	{
		for (i = 0; i < static_cast<int>(nb); i++)
		{
			if (devs[i].hwVer == SDRPLAY_RSPduo_ID)
				std::cout
				<< "Dev"
				<< i
				<< ": SerNo="
				<< devs[i].SerNo
				<< " hwVer="
				<< devs[i].hwVer
				<< " tuner=0x"
				<<  devs[i].tuner
				<<  "x rspDuoMode= "
				<< devs[i].rspDuoMode
				<< std::endl;


			else
			printf("Dev%d: SerNo=%s hwVer=%d tuner=0x%.2x\n", i, devs[i].SerNo,
				devs[i].hwVer, devs[i].tuner);
		}



		// Pick first RSPduo
		for (i = 0; i < (int)ndev; i++){
			if (devs[i].hwVer == SDRPLAY_RSPduo_ID)
			{
				chosenIdx = i;
					break;
			}
		}

		// Choose device
		printf("chosenDevice = %d\n", chosenIdx);
		chosenDevice = &devs[chosenIdx];

		// If chosen device is an RSPduo, assign additional fields
		if (chosenDevice->hwVer == SDRPLAY_RSPduo_ID)
		{
		// If master device is available, select device as master
		if (chosenDevice->rspDuoMode & sdrplay_api_RspDuoMode_Master)
		{
		// Select tuner based on user input (or default to TunerA)
		chosenDevice->tuner = sdrplay_api_Tuner_A;
		if (reqTuner == 1)
		chosenDevice->tuner = sdrplay_api_Tuner_B;
		// Set operating mode
		if (!master_slave) // Single tuner mode
		{
		chosenDevice->rspDuoMode = sdrplay_api_RspDuoMode_Single_Tuner;
		printf("Dev%d: selected rspDuoMode=0x%.2x tuner=0x%.2x\n", chosenIdx,
		chosenDevice->rspDuoMode, chosenDevice->tuner);
		}
		else
		{
		chosenDevice->rspDuoMode = sdrplay_api_RspDuoMode_Master;
		// Need to specify sample frequency in master/slave mode
		chosenDevice->rspDuoSampleFreq = 6000000.0;
		printf("Dev%d: selected rspDuoMode=0x%.2x tuner=0x%.2x rspDuoSampleFreq=%.1f\n",
		chosenIdx, chosenDevice->rspDuoMode,
		chosenDevice->tuner, chosenDevice->rspDuoSampleFreq);
		}
		}
		else // Only slave device available
		{
		// Shouldn't change any parameters for slave device
		}
		}
	}




	if (devs[0].hwVer == SDRPLAY_RSPduo_ID)
		std::cout << "Dev:0" << " SerNo=" << devs[0].SerNo << " hwVer=" <<devs[0].hwVer<<
			     " tuner=0x"<< devs[0].tuner<< "x rspDuoMode=0x"<<devs[0].rspDuoMode<< "\n";

	switch (devs[0].rspDuoMode) {

	case  sdrplay_api_RspDuoMode_Slave:
		std::cout << "Mode Slave" << std::endl;break;
	case sdrplay_api_RspDuoMode_Master :
		std::cout << "Mode Master" << std::endl;break;
	case sdrplay_api_RspDuoMode_Dual_Tuner :
		std::cout << "Mode Dual Tuner" << std::endl;break;
	case sdrplay_api_RspDuoMode_Single_Tuner :
		std::cout << "Mode Single Tuner" << std::endl;break;
	case sdrplay_api_RspDuoMode_Unknown :
		std::cout << "Unknown" << std::endl;break;

	default : std::cout << "unknown++" << std::endl;



	}
	sample_rate ++;
	frequency ++;
}

void RSPDuo::init_RSPDuo(unsigned int frequency, unsigned int sample_rate)
{
	(void) frequency;
	(void) sample_rate;

}

void RSPDuo::set_gain(int gain)
{
	gain ++;
}

void RSPDuo::receive(Process<int16_t> &process)
{
	sdrplay_api_ErrT err;
	if ((err = sdrplay_api_Init(chosenDevice->dev, &cbFns, static_cast<void *>(&process) )) != sdrplay_api_Success)
	{printf("sdrplay_api_Init failed %s\n", sdrplay_api_GetErrorString(err));


}
}
void RSPDuo::stop()
{

}

size_t RSPDuo::buffer_size()
{
	size_t t {};
	return t;
}
int RSPDuo::max_value()
{
	int i{};
	return i;
}

bool RSPDuo::is_streaming()
{
	return true;
}

RSPDuo::~RSPDuo()
{

}
/*
Airspy::Airspy(uint64_t serial_num, unsigned int frequency,
	       unsigned int sample_rate, airspy_sample_type sample_type) {
	AIRSPY_OPERATION(airspy_open_sn, &device, serial_num);

	init_airspy(frequency, sample_rate, sample_type);
}

void Airspy::init_airspy(unsigned int frequency, unsigned int sample_rate,
			 airspy_sample_type sample_type) {
	set_frequency(frequency);
	set_sample_rate(sample_rate);
	set_sample_type(sample_type);
	set_gain(1);
}

uint64_t Airspy::get_serial_number() {
	airspy_read_partid_serialno_t partid;
	AIRSPY_OPERATION(airspy_board_partid_serialno_read, device, &partid);

	return static_cast<uint64_t> (partid.serial_no[2]) << 32 |
		partid.serial_no[3];
}

bool Airspy::is_streaming() {
	return airspy_is_streaming(device);
}

void Airspy::set_frequency(unsigned int frequency) {
	AIRSPY_OPERATION(airspy_set_freq, device, frequency);
}

void Airspy::set_sample_rate(unsigned int sample_rate) {
	AIRSPY_OPERATION(airspy_set_samplerate, device, sample_rate);
}

void Airspy::set_sample_type(airspy_sample_type sample_type) {
	AIRSPY_OPERATION(airspy_set_sample_type, device, sample_type);
}

void Airspy::set_gain(int gain) {
	AIRSPY_OPERATION(airspy_set_linearity_gain, device, gain);
}

Airspy::~Airspy() {
	if (device != nullptr) {
		airspy_close(device);
		std::cout << "Closing airspy" << std::endl;
	}
}


 * A callback to wrap the real process to apply.
 *
 * This functions checks if the Airspy is still synced with its clock, and calls
 * the process provided.
 *
 * To check if the Airspy is still synced, the register 0 is dumped.  If the 4th
 * bit is set, the device is still synced.  If not, the device is out of sync,
 * and a warning is printed to stderr.

 * Then, it converts the context parameter to a Process, and calls its method
 * apply().  This call should be inlined when the program is optimised.
 *
 * This function delegates the processing instead of doing itself, because if we
 * wanted to handle another device kind, we would have to duplicate the process
 * in the other callback.  Moving the process makes it agnostic of the
 * underlying device and reusable.  Some parameters are needed (such as the
 * filter used), so it's a class and not a freestanding function.
 *
 * @param transfer The data received from the Airspy.
 */
/*
static int airspy_callback(airspy_transfer_t *transfer) {
	// Dump register and check if the airspy is still synced.
	uint8_t value;
	auto result {static_cast<airspy_error> (
			airspy_si5351c_read(transfer->device, 0, &value))};

	if (result != AIRSPY_SUCCESS) {
		std::cerr << "Error: could not dump register." << std::endl;
		std::cerr << airspy_error_name(result)
			  << std::endl;
	} else if (value & 0x10) {
		std::cerr << "Warning: Airspy out of sync." << std::endl;
	}

	if (transfer->dropped_samples > 0) {
		std::cerr << "Dropped samples" << std::endl;
	}

	// Processing input buffer
	// Get back the process and the samples.
	auto *process {static_cast<Process<int16_t> *> (transfer->ctx)};
	auto *samples {static_cast<int16_t *> (transfer->samples)};

	process->apply(samples, transfer->sample_count * 2);

	return 0;
}

void Airspy::receive(Process<int16_t> &process) {
	AIRSPY_OPERATION(airspy_start_rx, device, airspy_callback,
			 static_cast<void *> (&process));
}

void Airspy::stop() {
	airspy_stop_rx(device);
}

size_t Airspy::buffer_size() {
	// 262144 is from libairspy.  This is the size of the buffer in bytes.
	// Divided by 2 for both IQ channels, then by the size of an int16_t
	// (should be 2).  The final result should be equal to 65536.
	return 262144 / 2 / sizeof(int16_t);
}

int Airspy::max_value() {
	// 2^12
	return 4069;
}*/
