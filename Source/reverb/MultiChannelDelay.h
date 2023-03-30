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
        delay.prepare(spec);
        std::fill (lastDelayOutput.begin(), lastDelayOutput.end(), 0.0f);
    }
    
    void configure(double sampleRate)
    {
        float delaySamplesBase = delayInMs / 1000 * sampleRate;
        for (int channel = 0; channel < channels; ++channel)
        {
            float ratio = channel * 1.0 / channels;
            delaySamples[channel] = std::pow(2, ratio) * delaySamplesBase;
            delay.setMaximumDelayInSamples(delaySamples[channel] + 1);
            delay.reset();
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
                float delayedSample = delay.popSample(channel, delaySamples[channel], true);
                delayed.setSample(channel, sample, delayedSample);
            }
        }
        
        juce::AudioBuffer<float> mixed(channels, numSamples);
        householderMixer.process(delayed, mixed);
        
        for (int channel = 0; channel < channels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float inputSample = buffer.getSample(channel, sample);
                float mixedSample = mixed.getSample(channel, sample);
                float sum = inputSample + mixedSample * decayGain;
                
                buffer.setSample(channel, sample, sum);                
                delay.pushSample(channel, sum);
            }
        }
    }
        
private:
    float delayInMs = 80;
    float decayGain = 0.85;
    std::array<float, 2> lastDelayOutput;
    
    std::array<int, channels> delaySamples;
    juce::dsp::DelayLine<float> delay;
    HouseholderMixer householderMixer{channels};
};

