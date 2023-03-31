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
    
    float delayMsRange = 50;
    
    void prepare(juce::dsp::ProcessSpec& spec)
    {
        delay.prepare(spec);
    }
    
    void reset()
    {
        delay.reset();
    }

    void configure(double sampleRate)
    {
        float delaySamplesRange = delayMsRange / 1000 * sampleRate;
        for (int channel = 0; channel < channels; ++channel)
        {
            float rangeLow = delaySamplesRange * channel / channels;
            float rangeHigh = delaySamplesRange * (channel + 1) / channels;
            delaySamples[channel] = randomInRange(rangeLow, rangeHigh);
            delay.setMaximumDelayInSamples(delaySamples[channel] + 1);
            delay.reset();
            flipPolarity[channel] = rand() % 2;
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
                float inSample = mixed.getSample(channel, sample);
                if (flipPolarity[channel])
                {
                    inSample *= -1;
                }
                buffer.setSample(channel, sample, inSample);
            }
        }
    }

private:
    double randomInRange(double low, double high)
    {
        // Use C++11 random number generation library
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(low, high);

        return dist(gen);
    }
    
    std::array<int, channels> delaySamples;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delay;
    std::array<bool, channels> flipPolarity;
    HadamardMixer<float, channels> hadamard;
};
    
    
