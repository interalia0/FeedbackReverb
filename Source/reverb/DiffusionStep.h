/*
  ==============================================================================

    DiffusionStep.h
    Created: 30 Mar 2023 7:32:26pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include "../matrices/HadamardMatrix.h"
#include <JuceHeader.h>
#include <random>

template <int channels>
class DiffusionStep
{
public:
    
    float delayMsRange = 80;
    float delaySamplesRange;
    
    void prepare(juce::dsp::ProcessSpec& spec)
    {
        hostSampleRate = spec.sampleRate;
        delay.prepare(spec);
        delay.setMaximumDelayInSamples(spec.sampleRate * 2);
        
        for (int channel = 0; channel < channels; ++channel)
        {
            smoothingFilters[channel].prepare(spec);
            smoothingFilters[channel].setCutoffFrequency(1.2);
        }
    }
    
    void reset()
    {
        delay.reset();
    }

    void configure(double sampleRate)
    {
        delaySamplesRange = delayMsRange / 1000 * sampleRate;
        for (int channel = 0; channel < channels; ++channel)
        {
            float rangeLow = delaySamplesRange * channel / channels;
            float rangeHigh = delaySamplesRange * (channel + 1) / channels;
            delaySamples[channel] = randomInRange(rangeLow, rangeHigh);
            flipPolarity[channel] = rand() % 2;
        }
    }
    
    juce::AudioBuffer<float> process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        juce::AudioBuffer<float> delayed(channels, numSamples);

        for (int channel = 0; channel < channels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float inputSample = buffer.getSample(channel, sample);                
                delay.pushSample(channel, inputSample);
                float delayedSample = delay.popSample(channel, delaySamples[channel], true);
                delayed.setSample(channel, sample, delayedSample);
            }
        }

        juce::AudioBuffer<float> mixed(channels, numSamples);
        hadamard.process(delayed, mixed);
        
        for (int channel = 0; channel < channels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float mixedSample = mixed.getSample(channel, sample);
                
                if (flipPolarity[channel])
                {
                    mixedSample *= -1;
                }
                buffer.setSample(channel, sample, mixedSample);
            }
        }
        
        return buffer;
    }

private:
    double randomInRange(double low, double high)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(low, high);

        return dist(gen);
    }
    
    std::array<int, channels> delaySamples;
    std::array<float, channels> smoothDelaySamples;
    
    double hostSampleRate;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delay;
    std::array<bool, channels> flipPolarity;
    HadamardMixer<float, channels> hadamard;
    std::array<juce::dsp::FirstOrderTPTFilter<float>, channels> smoothingFilters;
};
    
    
