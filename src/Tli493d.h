/**
 *  @page The Arduino Library for Infineon's Magnetic 3D Sensor TLI493D-W2BW
 *  @section introduction Introduction
 * 	The TLI493D-W2B6 sensor family comes with I2C interface and wake-up function. This sensor family TLI493D offers
 * 	accurate three dimensional sensing with extremely low-power consumption.
 *
 *	@section main_features Main Features
 *
 *	@subsection interrupts Interrupts
 * 	Interrupts can be sent from the sensor to the microcontroller to notify the completion of a measurement and its ADC
 * 	conversion. Values read directly after interrupts are guaranteed to be consistent. Without interrupts values read might
 * 	be stale.
 *
 * 	@subsection collision_avoidance Collision Avoidance and Clock Stretching
 *	In case of a collision, the sensor interrupt disturbs the I2C clock, causing an additional SCL pulse which shifts the
 *	data read out by one bit. If collision avoidance is activated, the sensor monitors the start/stop conditions, and suppresses
 *	interrupts in between.
 *
 *	When interrupts are disabled, this feature becomes clock stretching, that is, the data read out only starts after the ADC conversion is
 *	finished. Thus it can be avoided that during an ADC conversion old or corrupted measurement results are read out, which
 *	may occur when the ADC is writing to a register while this is being read out by the microcontroller. When clock stretching
 *	is enabled, the sensor pulls the SCL line down during ongoing ADC conversions, reading of sensor registers or the transmission
 *	of valid ACKs.
 *  For the W2BW-type clock streching only works, if SCL- and /INT-pins are shorted together, as the sensor uses the output-driver of 
 *  the /INT-pin to keep the line LOW until measurements are finished.
 *
 *	Two register bits (CA and INT) work together for different configurations.
 *
 *  @subsection wake_up Wake Up Mode
 *	Wake up mode is intended to be used with low power mode or fast mode. This mode disables interrupts within a user-specified
 *	range, so that interrupts are generated only when relevant data are available.
 */

#ifndef TLI493D_H_INCLUDED
#define TLI493D_H_INCLUDED

#include <Arduino.h>
#include <Wire.h>
#include "./util/BusInterface.h"
#include "./util/Tli493d_conf.h"

#define NO_POWER_PIN -1

typedef enum Tli493d_Error
{
	TLI493D_NO_ERROR = 0,
	TLI493D_BUS_ERROR = 1,
	TLI493D_FRAME_ERROR = 2
} Tli493d_Error_t;

class Tli493d
{
  public:
	enum TypeAddress_e
	{
		TLI493D_A0 = 0x35,
		TLI493D_A1 = 0x22,
		TLI493D_A2 = 0x78,
		TLI493D_A3 = 0x44
	};

	/**
	 * @enum TypeAddress_e Defines four types of the sensor family which are supported by this library and their corresponding addresses
	 * 		 The addresses can be concatenated with 0 or 1 for reading or writing
	 */

	enum AccessMode_e
	{
		LOWPOWERMODE = 0,
		MASTERCONTROLLEDMODE = 1,
		FASTMODE = 3,
	};
	/**
	 * @enum AccessMode_e Enumerates the three available modes; number 2 is reserved
	 *		 In low power mode cyclic measurements and ADC-conversions are carried out with a update rate; the wake-up function is already configured for this mode, so that
	 *		 the sensor can continue making magnetic field measurements. With this configuration the microcontroller will only consume power and access the sensor
	 *		 if relevant measurement data is available.
	 *		 In master controlled mode the sensor powered down if when it is not triggered. This library configures to ADC start before sending first MSB of data registers
	 *		 In fast mode the measurements and ADC-conversions are running continuously.
	 */
	 
	enum Range_e
	{
		FULL = 0,
		SHORT = 1,
		EXTRASHORT = 3,
	};
	/**
	 * @enum Range_e Enumerates the three available ranges; number 2 is reserved
	 *		The full range is from -160mT to 160mT with a sensitivity of 7.7 LSB/mT.
	 *		The short range is from -100mT to 100mT with a sensitivity of 15.4 LSB/mT.
	 *		The extra short range is from -50mT to 50mT with a sensitivity of 30.8 LSB/mT. This is a special feature of the W2BW type.
	 */

	
	/**
	 * @brief Constructor of the sensor class.
	 * @param mode Operating mode of the sensor; default is the master controlled mode
	 * @param productType The library supports product types from A0 to A3; default is type A0
	 */
	Tli493d(AccessMode_e mode = MASTERCONTROLLEDMODE,
			TypeAddress_e productType = TLI493D_A0,
			int resetPin = NO_POWER_PIN,
			bool powerLevel = HIGH);
	
	/**
	 * @brief Constructor of the sensor class.
	 * @param mode Operating mode of the sensor; default is the master controlled mode
	 * @param productType The library supports product types from A0 to A3; default is type A0
	 */
	Tli493d(int resetPin, 
			bool powerLevel = HIGH, 
			AccessMode_e mode = MASTERCONTROLLEDMODE,
			TypeAddress_e productType = TLI493D_A0);

	/**
	 * @brief Destructor
	 */
	~Tli493d(void);
	/**
	 * @brief Starts the sensor
	 */
	void begin(void);
	
	/**
	 * @brief Starts the sensor
	 */
	void begin(bool);

	/**
	 * @brief Starts the sensor
	 * @param bus The I2C bus
	 * @param slaveAddress The 7-bit slave address as defined in @ref Tli493d_Type
	 * @param reset If a reset should be initiated before starting the sensor
	 * @param oneByteRead If one-byte read protocol should be used. Otherwise the two-byte protocol is available
	 */
	void begin(TwoWire &bus, TypeAddress_e slaveAddress, bool reset,
			   uint8_t oneByteRead);

	/**
	 * @brief Sets the operating mode of the sensor
	 * @param mode MASTERCONTROLLEDMODE,LOWPOWERMODE or FASTMODE, default is MASTERCONTROLLEDMODE
	 * @return true if configuration was successfull, otherwise false
	 */
	bool setAccessMode(AccessMode_e mode);
	
	/**
	 * @brief Sets when new measurements are triggered in MASTERCONTROLLEDMODE. If invalid argument or sensor mode is LOWPOWERMODE this function returns without having any effect.
	 * @param trigger: 0 = no measurements, 1 = measurements on read before first MSB, 2 = measurements on read after register 0x05
	 */
	void setTrigger(uint8_t trigger);
	
	/**
	 * @brief Enables temperature measurement; by default already enabled
	 */
	void enableTemp(void);

	/**
	 * @brief Disables temperature measurement to reduce power consumption
	 */
	void disableTemp(void);
	
	/**
	 * @brief Enables BZ measurement; by default already enabled
	 */
	void enableBz(void);

	/**
	 * @brief Disables Bz measurement to reduce power consumption. This only works, when temperature measurement is disabled as well.
	 */
	void disableBz(void);
	
	/**
	 * @brief The Wake Up threshold range disabling /INT pulses between upper threshold and lower threshold is limited
	 *  	  to a window of the half output range. Here the adjustable range can be set with a ratio of size [-1,1]
	 * 		  When all the measurement values Bx, By and Bz are within this range INT is disabled.
	 * 		  If the arguments are out of range or any upper threshold is smaller than the lower one, the function
	 * 		  returns false without taking effect.
	 * 		  If any of the ranges xh-xl, yh-yl or zh-zl is greater than half output range, false will be returned even thought the values are written to the wake-up regeisters.
	 * @param xh Upper threshold in x direction [-1,1]
	 * @param xl Lower threshold in x direction [-1,1]
	 * @param yh Upper threshold in y direction [-1,1]
	 * @param yl Lower threshold in y direction [-1,1]
	 * @param zh Upper threshold in z direction [-1,1]
	 * @param zl Lower threshold in z direction [-1,1]
	 * @return true if configuration was successfull and interrupts will be sent, otherwise false.
	 */
    bool setWakeUpThreshold(float xh, float xl, float yh, float yl, float zh,
                            float zl);
							
	/**
	 * @brief The Wake Up threshold range disabling /INT pulses between upper threshold and lower threshold is limited
	 *  	  to a window of the half output range. Here the adjustable range can be set in LSB [-2048,2047].
	 * 		  When all the measurement values Bx, By and Bz are within this range INT is disabled.
	 * 		  If the arguments are out of range or any upper threshold is smaller than the lower one, the function
	 * 		  returns false without taking effect.
	 * 		  If any of the ranges xh-xl, yh-yl or zh-zl is greater than half output range, false will be returned, even thought the values are written to the wake-up regeisters.
	 * @param xh Upper threshold in x direction [-2048,2047]
	 * @param xl Lower threshold in x direction [-2048,2047]
	 * @param yh Upper threshold in y direction [-2048,2047]
	 * @param yl Lower threshold in y direction [-2048,2047]
	 * @param zh Upper threshold in z direction [-2048,2047]
	 * @param zl Lower threshold in z direction [-2048,2047]
	 * @return true if configuration was successfull and interrupts will be sent, otherwise false.
	 */
    bool setWakeUpThresholdLSB(int16_t xh, int16_t xl, int16_t yh, int16_t yl, int16_t zh,
                            int16_t zl);
							
	/**
	 * @brief The Wake Up threshold range disabling /INT pulses between upper threshold and lower threshold is limited
	 *  	  to a window of the half output range. Here the adjustable range can be set in mT.
	 * 		  When all the measurement values Bx, By and Bz are within this range INT is disabled.
	 * 		  If the arguments are out of range or any upper threshold is smaller than the lower one, the function
	 * 		  returns false without taking effect.
	 * 		  If any of the ranges xh-xl, yh-yl or zh-zl is greater than half output range, false will be returned, even thought the values are written to the wake-up regeisters.
	 * @param xh Upper threshold in x direction in mT
	 * @param xl Lower threshold in x direction in mT
	 * @param yh Upper threshold in y direction in mT
	 * @param yl Lower threshold in y direction in mT
	 * @param zh Upper threshold in z direction in mT
	 * @param zl Lower threshold in z direction in mT
	 * @return true if configuration was successfull and interrupts will be sent, otherwise false.
	 */
    bool setWakeUpThresholdMT(float xh, float xl, float yh, float yl, float zh,
                            float zl);

    /**
	 * @brief Checks if WA bit is set. When not interrupt configuration is as specified by the CA and INT bits.
	 */
    bool wakeUpEnabled(void);
	
	/**
	 * @brief Enables the Wake Up functionality of the sensor. Following conditions must be fulfilled:
	 * 		  - Test modes must be disabled and the T-Bit in Register 0x06 needs to be 0.
	 *		  - CP parity bit must be odd
	 *		  - Configuration partiy must be flagged (CF bit in Register 0x06 needs to be 1).
	 *		  You can check, if the Wake-Up-functionality is activated with the function wakeUpEnabled()
	 */
    void enableWakeUp(void);
	
	/**
	 * @brief Disables the Wake Up functionality of the sensor.
	 */
    void disableWakeUp(void);

    /**
	 * @brief Sets the update rate in low power mode
	 * @param updateRate Update rate which is an unsigned integer from the 0 (the fastest) to 7 (the highest)
	 */
    void setUpdateRate(uint8_t updateRate);
	
	/**
	 * @brief Sets the magnetic range that can be measured. The smaller the range, the higher the sensitivity. 
	 * 		  Please note: The EXTRASHORT-range enables the T-Bit in Register 0x06 and therefore cannot be used together with the WakeUp-feature.
						   Before setting the range to EXTRASHORT, the WakeUp needs to be disabled via disableWakeUp(). Otherwise this function will return false without taking effect.
	 * @param range FULL, SHORT or EXTRASHORT, default is FULL.
	 * @return true if configuration was successfull, otherwise false.
	 */
	bool setMeasurementRange(Range_e range);

	/**
	 * @brief Reads measurement results from sensor
	 */
	Tli493d_Error updateData(void);

	/**
	 * @return the Cartesian x-coordinate
	 */
	float getX(void);
	/**
	 * @return the Cartesian y-coordinate
	 */
	float getY(void);
	/**
	 * @return the Cartesian z-coordinate
	 */
	float getZ(void);

	/**
	 * @return norm of the magnetic field vector sqrt(x^2 + y^2 + z^2)
	 */
	float getNorm(void);

	/**
	 * @return the Azimuth angle arctan(y/x)
	 */
	float getAzimuth(void);

	/**
	 * @return the angle in polar coordinates arctan(z/(sqrt(x^2+y^2)))
	 */
	float getPolar(void);

	/**
	 * @return the temperature value
	 */
	float getTemp(void);

	/**
	 * @brief Resets the sensor
	 */
	void resetSensor(void);

	void readDiagnosis(uint8_t (&diag)[7]);
	
		/**
	 * @brief Enables interrupts
	 */
	void enableInterrupt(void);

	/**
	 * @brief Disables interrupts; When Collision Avoidance is activated, Sensor read-outs are suppressed during ongoing ADC conversion (clock stretching)
	 */
	void disableInterrupt(void);

	/**
	 * @brief Enables collision avoidance. When Interrupt is deactivated, clock stretching is active.
	 * 		  For clock stretching: SCL and INT pins must be shorted.
	 */
	void enableCollisionAvoidance(void);

	/**
	 * @brief Disables collision avoidance; readouts may collide with ADC conversion
	 */
	void disableCollisionAvoidance(void);


  protected:
	tli493d::BusInterface_t mInterface;
	/**
	 * @brief Stores new values into the bus interface; for this function to take effect the function writeOut() should be called afterwards
	 * @param Register mask index as defined in @ref Registers_e
	 * @param Value to be written into the register field specified by the register index
	 */
	void setRegBits(uint8_t regMaskIndex, uint8_t data);

	/**
	 * @brief Returns the value of a register field
	 * @param Register mask index as defined in @ref Registers_e
	 */
	uint8_t getRegBits(uint8_t regMaskIndex);

  private:
	AccessMode_e mMode;
	const TypeAddress_e mProductType;
	int mPowerPin;
	bool mPowerLevel;
	int16_t mXdata;
	int16_t mYdata;
	int16_t mZdata;
	int16_t mTempdata;
	float mBMult = TLI493D_B_MULT_FULL;

	/**
	 * @brief Sets FP (fuse parity) and CP (configuration parity)
	 */
	void calcParity(uint8_t regMaskIndex);

	/**
	 * @brief Concatenates the upper bits and lower bits of magnetic or temperature measurements
	 */
	int16_t concatResults(uint8_t upperByte, uint8_t lowerByte, bool isB);
};

#endif /* TLI493D_H_INCLUDED */
