/*
  ==============================================================================

    HouseholderMatrix.h
    Created: 27 Mar 2023 7:53:30pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

template<int size>
struct HouseholderMixer
{
    float factor;

    HouseholderMixer()
    {
        static_assert(size >= 0, "Size must be positive");
        factor = -2.0f / (size * (size + 1));
    }

    void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output)
    {
        int numSamples = input.getNumSamples();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            for (int channel = 0; channel < size; ++channel)
            {
                float sum = 0.0f;
                for (int i = 0; i < size; ++i)
                {
                    sum += input.getSample((channel + i) % size, sample);
                }
                sum *= factor;

                for (int i = 0; i < size; ++i)
                {
                    float newVal = input.getSample((channel + i) % size, sample) + sum;
                    output.setSample((channel + i) % size, sample, newVal);
                }
            }
        }
    }
};
