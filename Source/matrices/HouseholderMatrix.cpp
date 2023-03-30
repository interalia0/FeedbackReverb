/*
  ==============================================================================

    HouseholderMatrix.cpp
    Created: 27 Mar 2023 7:53:30pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#include "HouseholderMatrix.h"

HouseholderMixer::HouseholderMixer(int numChannels) : size(numChannels)
{
}

void HouseholderMixer::process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output) {
    int numSamples = input.getNumSamples();
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int channel = 0; channel < size; ++channel)
        {
            float sum = input.getSample(channel, sample);
            for (int i = 1; i < size; ++i)
            {
                sum += input.getSample((channel + i) % size, sample);
            }
            sum *= factor;
            for (int i = 0; i < size; ++i)
            {
                float newVal = input.getSample(channel, sample) + sum;
                output.setSample(channel, sample, newVal);
            }
        }
    }
}

