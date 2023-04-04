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
class Reverb
{
public:
    
    Reverb(juce::AudioProcessorValueTreeState& state) : treeState(state)
    {
    }
    
    void prepare(juce::dsp::ProcessSpec& spec)
    {
        specSampleRate = spec.sampleRate;
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
    
    void setRt60()
    {
        rt60 = getRt60();
        float typicalLoopMs = feedback.delayInMs * 1.5;
        float loopsPerRt60 = rt60 / (typicalLoopMs * 0.001);
        float dbPerCycle = -60 / loopsPerRt60;
        decayValue = std::pow(10, dbPerCycle * 0.05);
        feedback.updateDecay(decayValue);
    }
    
    void setSize()
    {
        sizeValue = getSize();
        feedback.updateTime(sizeValue);
    }
private:
    
    float getRt60()
    {
        return *treeState.getRawParameterValue("decay");
    }
    
    float getSize()
    {
        return *treeState.getRawParameterValue("size");
    }
    
    static const int revChannels = 8;
    double specSampleRate;
    float rt60;
    
    float decayValue;
    float sizeValue;
    
    MultiChannelDelay<revChannels> feedback;
    Diffuser<revChannels, diffusionSteps> diffuser{150};
    juce::AudioProcessorValueTreeState& treeState;
    
    
};
