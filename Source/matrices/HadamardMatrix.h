/*
  ==============================================================================

    HadamardMatrix.h
    Created: 27 Mar 2023 7:55:15pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

template <typename Sample, int channels>
class HadamardMixer {
public:
    void process(juce::AudioBuffer<Sample>& input, juce::AudioBuffer<Sample>& output) {
        int numSamples = input.getNumSamples();

        for (int sample = 0; sample < numSamples; ++sample) {
            unscaledInPlace(input, 0, channels, sample);
            scaleInPlace(input, sample);

            for (int channel = 0; channel < channels; ++channel) {
                output.setSample(channel, sample, input.getSample(channel, sample));
            }
        }
    }

private:
    static Sample scalingFactor() {
        return std::sqrt(Sample(1) / (channels ? channels : 1));
    }

    void scaleInPlace(juce::AudioBuffer<Sample>& data, int sampleIndex) {
        Sample factor = scalingFactor();
        for (int channel = 0; channel < channels; ++channel) {
            data.setSample(channel, sampleIndex, data.getSample(channel, sampleIndex) * factor);
        }
    }

    void unscaledInPlace(juce::AudioBuffer<Sample>& data, int startIndex, int size, int sampleIndex) {
        if (size <= 1) return;

        const int hSize = size / 2;
        unscaledInPlace(data, startIndex, hSize, sampleIndex);
        unscaledInPlace(data, startIndex + hSize, hSize, sampleIndex);

        for (int i = 0; i < hSize; ++i) {
            const Sample a = data.getSample(i + startIndex, sampleIndex);
            const Sample b = data.getSample(i + startIndex + hSize, sampleIndex);
            data.setSample(i + startIndex, sampleIndex, a + b);
            data.setSample(i + startIndex + hSize, sampleIndex, a - b);
        }
    }
};


