/*
  ==============================================================================

    Diffuser.h
    Created: 30 Mar 2023 8:10:37pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "DiffusionStep.h"

template <int channels, int stepCount>
struct Diffuser
{

    using Step = DiffusionStep<channels>;
    std::array<Step, stepCount> steps;

    Diffuser(float diffusionMs)
    {
        for (auto &step : steps)
        {
            diffusionMs *= 0.5;
            step.delayMsRange = diffusionMs;
        }
    }
        
    void prepare (juce::dsp::ProcessSpec& spec)
    {
        for (auto& step : steps)
        {
            step.prepare(spec);
        }
    }
    
    void reset()
    {
        for (auto& step : steps)
        {
            step.reset();
        }
    }
    
    void configure (double sampleRate)
    {
        for (auto& step : steps)
        {
            step.configure(sampleRate);
        }
    }
    
    void updateTime (float size)
    {
        for (auto& step : steps)
        {
            step.updateTime(size);
        }
    }
    void updateDamping (float& dampingValue)
    {
        for (auto& step : steps)
        {
            step.dampingFilter.setCutoffFrequency(dampingValue);
        }
    }

    juce::AudioBuffer<float> processInPlace(juce::AudioBuffer<float>& buffer)
    {
        for (int step = 0; step < stepCount; ++step)
        {
            buffer = steps[step].process(buffer);
        }
        
        return buffer;
    }
};
