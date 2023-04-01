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

    Diffuser(double diffusionMs)
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

    void processInPlace(juce::AudioBuffer<float>& buffer)
    {
        for (int step = 0; step < stepCount; ++step)
        {
            steps[step].process(buffer);
        }
    }
    
};
