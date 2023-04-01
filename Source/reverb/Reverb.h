/*
  ==============================================================================

    Reverb.h
    Created: 1 Apr 2023 11:38:14am
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Diffuser.h"
#include "MultiChannelDelay.h"


template <int channels, int diffusionSteps>
struct Reverb
{
    Reverb(float roomSizeMs, float rt60) : diffuser(roomSizeMs)
    {
        feedback.delayInMs = roomSizeMs;

        // How long does our signal take to go around the feedback loop?
        double typicalLoopMs = roomSizeMs*1.5;
        // How many times will it do that during our RT60 period?
        double loopsPerRt60 = rt60/(typicalLoopMs*0.001);
        // This tells us how many dB to reduce per loop
        double dbPerCycle = -60/loopsPerRt60;

        feedback.decayGain = std::pow(10, dbPerCycle*0.05);
    }
    
    void prepare(juce::dsp::ProcessSpec& spec)
    {
        diffuser.prepare(spec);
        feedback.prepare(spec);
    }
    
    void configure(double sampleRate)
    {
        diffuser.configure(sampleRate);
        feedback.configure(sampleRate);
    }
    
    void reset()
    {
        diffuser.reset();
        feedback.reset();
    }
    
    void processInPlace(juce::AudioBuffer<float>& buffer)
    {
        diffuser.processInPlace(buffer);
        feedback.processInPlace(buffer);
    }
    
    static const int revChannels = 8;

    MultiChannelDelay<revChannels> feedback;
    Diffuser<revChannels, diffusionSteps> diffuser;
};
