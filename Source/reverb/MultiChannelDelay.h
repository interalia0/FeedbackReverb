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
class MultiChannelDelay
{
public:
    
    float delayInMs = 150;
    double specSampleRate;
    
    void prepare(juce::dsp::ProcessSpec& spec)
    {
        delay.prepare(spec);
        dampingFilter.prepare(spec);
    }
    
    void configure(double sampleRate)
    {
        float delaySamplesBase = delayInMs / 1000 * sampleRate;
        for (int channel = 0; channel < channels; ++channel)
        {
            float ratio = channel * 1.0 / channels;
            delaySamples[channel] = std::pow(2, ratio) * delaySamplesBase;
            delay.setMaximumDelayInSamples(sampleRate * 2);
        }
                
        dampingFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        dampingFilter.setCutoffFrequency(4000);
        
    }
    
    void reset()
    {
        delay.reset();
        dampingFilter.reset();
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
                
                dampingFilter.processSample(channel, sum);
                
                buffer.setSample(channel, sample, sum);                
                delay.pushSample(channel, sum);
            }
        }        
    }
    
            
private:
    float decayGain = 0.85;
    
    std::array<int, channels> delaySamples;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delay;
    HouseholderMixer<channels> householderMixer;
    
    juce::dsp::StateVariableTPTFilter<float> dampingFilter;
};

