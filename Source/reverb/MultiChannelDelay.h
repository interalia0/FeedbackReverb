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
    float delayInMs = 125;
    float decayGain = 0.6f;

    juce::dsp::StateVariableTPTFilter<float> dampingFilter;


    void prepare(juce::dsp::ProcessSpec& spec)
    {
        hostSampleRate = spec.sampleRate;
        
        delay.prepare(spec);
        delay.setMaximumDelayInSamples(spec.sampleRate);
        dampingFilter.prepare(spec);
        dampingFilter.setCutoffFrequency(15000);
        
        for (int channel = 0; channel < channels; ++channel)
        {
            smoothingFilters[channel].prepare(spec);
            smoothingFilters[channel].setCutoffFrequency(1);
        }

    }
    
    void configure(double sampleRate)
    {
        delaySamplesBase = delayInMs / 1000 * sampleRate;
        for (int channel = 0; channel < channels; ++channel)
        {
            float ratio = channel * 1.0 / channels;
            delaySamples[channel] = std::pow(2, ratio) * delaySamplesBase;
        }
    }

    void reset() 
    {
        delay.reset();
    }
        
    void updateDecay(float rt60)
    {
        decayGain = rt60;
    }
            
    juce::AudioBuffer<float> processInPlace(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        juce::AudioBuffer<float> delayed(channels, numSamples);
                
        for (int channel = 0; channel < channels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                smoothDelaySamples[channel] = smoothingFilters[channel].processSample(channel, delaySamples[channel]);
                float delayedSample = delay.popSample(channel, smoothDelaySamples[channel], true);

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
                mixedSample = dampingFilter.processSample(channel, mixedSample);
                float sum = inputSample + mixedSample * decayGain;
                
                buffer.setSample(channel, sample, sum);                
                delay.pushSample(channel, sum);
            }
        }
        
        return buffer;
    }

private:    
    
    float delaySamplesBase;
    double hostSampleRate;
        
    std::array<int, channels> delaySamples;
    std::array<float, channels> smoothDelaySamples;

    juce::dsp::DelayLine<float> delay;
    HouseholderMixer<channels> householderMixer;
    
    std::array<juce::dsp::FirstOrderTPTFilter<float>, channels> smoothingFilters;
};

