/*
  ==============================================================================

    MultiChannelDelay.h
    Created: 28 Mar 2023 9:30:36pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include "../matrices/HouseholderMatrix.h"
#include <JuceHeader.h>

template <int channels>
struct MultiChannelDelay
{    
    void prepare(juce::dsp::ProcessSpec& spec)
    {
        delay.prepare(spec);
        delay.setMaximumDelayInSamples(spec.sampleRate);
        dampingFilter.prepare(spec);
    }
    
    void configure(double sampleRate)
    {
        float delaySamplesBase = delayInMs / 1000 * sampleRate;
        for (int channel = 0; channel < channels; ++channel)
        {
            float ratio = channel * 1.0 / channels;
            delaySamples[channel] = std::pow(2, ratio) * delaySamplesBase;
        }
        dampingFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        dampingFilter.setCutoffFrequency(500);
    }

    void reset()
    {
        delay.reset();
        dampingFilter.reset();
    }
        
    void setDecay(float rt60)
    {
        decayGain = rt60;
    }
        
    void processInPlace(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        juce::AudioBuffer<float> delayed(channels, numSamples);
                
        for (int channel = 0; channel < channels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float delayedSample = delay.popSample(channel, delaySamples[channel], true);
                dampingFilter.processSample(channel, delayedSample);
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
    
    float delayInMs = 150;
    float decayGain;
        
    std::array<int, channels> delaySamples;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delay;
    HouseholderMixer<channels> householderMixer;
    
    juce::dsp::StateVariableTPTFilter<float> dampingFilter;
};

