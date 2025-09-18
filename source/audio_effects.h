#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>

namespace AudioEffects {
	enum {
		EFF_NONE,
		EFF_MASKVOICE,
        EFF_REVERB,
	};

    class DelayLine
    {
    public:
        float* buffer;
        int writeIndex;
        int bufferSize;

        DelayLine(int size = 480)
        {
            bufferSize = size;
            buffer = new float[bufferSize];
            memset(buffer, 0, sizeof(float) * bufferSize);
            writeIndex = 0;
        }

        void Write(float value)
        {
            buffer[writeIndex] = value;
            writeIndex = (writeIndex + 1) % bufferSize;
        }

        float Read(int delaySamples)
        {
            int readIndex = (writeIndex - delaySamples + bufferSize) % bufferSize;

            return buffer[readIndex];
        }
    };

    class CombFilter
    {
    public:
        DelayLine* delayLine;
        float feedbackGain; // How much signal feeds back (controls decay)
        float filterStore;  // Stores previous filtered value for smoothing
        float damping;      // Controls high-frequency damping

        CombFilter(int delaySamples, float feedback, float damping)
        {
            delayLine = new DelayLine(delaySamples);
            feedbackGain = feedback;
            this->damping = damping;
            filterStore = 0;
        }

        float Process(float input)
        {
            float delayedSample = delayLine->Read(delayLine->bufferSize);

            // Damping (low-pass filter on feedback)
            filterStore = (delayedSample * (1.0f - damping)) + (filterStore * damping);

            float output = input + filterStore * feedbackGain;
            delayLine->Write(output); // Feed processed signal back into delay
            return output;
        }
    };

    // Basic All-Pass Filter (smears phase, adds density)
    class AllPassFilter
    {
    public:
        DelayLine* delayLine;
        float feedbackGain; // Coefficient for feedback/feedforward

        AllPassFilter(int delaySamples, float gain)
        {
            delayLine = new DelayLine(delaySamples);
            feedbackGain = gain;
        }

        float Process(float input)
        {
            float delayedSample = delayLine->Read(delayLine->bufferSize);
            float output = -input * feedbackGain + delayedSample;
            delayLine->Write(input + output * feedbackGain); // Feedforward part
            return output;
        }
    };

    auto filter = CombFilter(1557, pow(0.001, 1557.0f / (1 * 24000)), 0.5);

    std::map<int, double> filterStore; // multiple players speaking will cause problems, so do that
    double damping2 = 0.95;
    
	void VoiceInMask(uint16_t* sampleBuffer, int samples, int uid, double damping = damping2) {
		for (int i = 0; i < samples; i++) {
			double signedSample = static_cast<double>(static_cast<int16_t>(sampleBuffer[i]) - 32768); // samples ranging from -32768 to 32768
            //signedSample = (delayedSample * (1.0f - damping)) + (filterStore * damping);
            signedSample = (signedSample * (1.0f - damping)) + (filterStore[uid] * damping);
            filterStore[uid] = signedSample;
			reinterpret_cast<uint16_t*>(sampleBuffer)[i] = static_cast<uint16_t>(signedSample + 32768);

			//if (i < 10)
			//{
			//	Msg((std::to_string(i) + " = " + std::to_string(sampleBuffer[i]) + " 2\n").c_str());
			//}
		}
	}

    void Reverb(uint16_t* sampleBuffer, int samples) {
        for (int i = 0; i < samples; i++) {
            double signedSample = static_cast<double>(static_cast<int16_t>(sampleBuffer[i]) - 32768);
            signedSample = filter.Process(signedSample);
            reinterpret_cast<uint16_t*>(sampleBuffer)[i] = static_cast<uint16_t>(signedSample + 32768);
        }
    }
}