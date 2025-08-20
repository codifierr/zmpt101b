#include "zmpt101b.h"

namespace esphome {
namespace zmpt101b {

/// @brief ZMPT101B constructor
/// @param pin analog pin that ZMPT101B connected to.
/// @param frequency AC system frequency
ZMPT101B::ZMPT101B(uint8_t pin_, uint16_t frequency_, float sensitivity_)
{
	this->pin = pin_;
	this->period = 1000000 / frequency_;
	
	this->sensitivity = sensitivity_;

}

/// @brief Calculate zero point
/// @return zero / center value
int ZMPT101B::getZeroPoint()
{
	uint32_t Vsum = 0;
	uint32_t measurements_count = 0;
	uint32_t t_start = micros();

	while (micros() - t_start < this->period)
	{
		Vsum += analogRead(pin);
		measurements_count++;
	}

	return Vsum / measurements_count;
}

/// @brief Calculate root mean square (RMS) of AC valtage
/// @param loopCount Loop count to calculate
/// @return root mean square (RMS) of AC valtage
float ZMPT101B::getRmsVoltage(uint8_t loopCount)
{
	double readingVoltage = 0.0f;

	for (uint8_t i = 0; i < loopCount; i++)
	{
		int zeroPoint = this->getZeroPoint();
		
		// Validate zero point is reasonable (not floating)
		// For ESP8266, valid ADC range is 0-1024, reasonable center should be 300-700
		if (zeroPoint < 100 || zeroPoint > 900) {
			// Pin might be floating or disconnected
			return 0.0;
		}

		int32_t Vnow = 0;
		uint32_t Vsum = 0;
		uint32_t measurements_count = 0;
		uint32_t t_start = micros();
		int32_t maxDeviation = 0;

		while (micros() - t_start < this->period)
		{
			Vnow = analogRead(pin) - zeroPoint;
			Vsum += (Vnow * Vnow);
			measurements_count++;
			
			// Track maximum deviation from zero point
			if (abs(Vnow) > maxDeviation) {
				maxDeviation = abs(Vnow);
			}
		}
		
		// Check if signal is above noise floor
		// If maximum deviation is too small, it's likely just noise
		if (maxDeviation < 3) {  // Minimum signal threshold (adjustable)
			return 0.0;  // No meaningful signal detected
		}

		double rmsValue = sqrt((double)Vsum / measurements_count);
		readingVoltage += (rmsValue / ADC_SCALE) * VREF * this->sensitivity;
	}

	float finalVoltage = readingVoltage / loopCount;
	
	// Apply final minimum threshold filter
	// Values below this are considered noise/disconnected
	if (finalVoltage < 0.1) {
		return 0.0;
	}

	return finalVoltage;
}

}
}
