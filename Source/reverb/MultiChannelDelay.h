/*
  ==============================================================================

    MultiChannelDelay.h
    Created: 28 Mar 2023 9:30:36pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include "../matrices/HouseholderMatrix.h"
#include "../dsp/mix.h"
#include <JuceHeader.h>

template <int channels = 8>
class MultiChannelDelay
{
public:
    
    void prepare(juce::dsp::ProcessSpec& spec, double sampleRate)
    {
        delays.prepare(spec);
        std::fill (lastDelayOutput.begin(), lastDelayOutput.end(), 0.0f);
    }
    
    void configure(double sampleRate)
    {
        float delaySamplesBase = delayInMs / 1000 * sampleRate;
        for (int channel = 0; channel < channels; ++channel)
        {
            float ratio = channel * 1.0 / channels;
            delaySamples[channel] = std::pow(2, ratio) * delaySamplesBase;
            delays.setMaximumDelayInSamples(delaySamples[channel] + 1);
            delays.reset();
        }
    }
        
    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        juce::AudioBuffer<float> delayed(channels, numSamples);
        
        
        for (int channel = 0; channel < channels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float delayedSample = delays.popSample(channel, delaySamples[channel], true);

                delayed.setSample(channel, sample, delayedSample);
            }
        }
        
        householderMixer.processInPlace(delayed);
        
        for (int channel = 0; channel < channels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                auto input = buffer.getSample(channel, sample);
                auto mixed = delayed.getSample(channel, sample);
                float sum = input + mixed * feedback;
                
                buffer.setSample(channel, sample, sum);
                
                delays.pushSample(channel, sum);
            }
        }
    }
        
private:
    float delayInMs = 150;
    float feedback = 0.85;
    std::array<float, 2> lastDelayOutput;
    
    std::array<int, channels> delaySamples;
    juce::dsp::DelayLine<float> delays;
    HouseholderMixer householderMixer{channels};
};

