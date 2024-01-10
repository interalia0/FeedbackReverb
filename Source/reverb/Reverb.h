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
    
    Reverb(juce::AudioProcessorValueTreeState& state) : treeState(state) {}
    
    void prepare(juce::dsp::ProcessSpec& spec)
    {
        specSampleRate = spec.sampleRate;
        lateDiffuser.prepare(spec);
        lateReflections.prepare(spec);
    }
    
    void configure(double sampleRate)
    {
        lateDiffuser.configure(sampleRate);
        lateReflections.configure(sampleRate);
    }
    
    void reset()
    {
        lateDiffuser.reset();
        lateReflections.reset();
    }
    
    juce::AudioBuffer<float> processInPlace(juce::AudioBuffer<float>& buffer)
    {
        auto diffuse = lateDiffuser.processInPlace(buffer);
        auto output = lateReflections.processInPlace(diffuse);
        
        return output;
    }
    
    void setRt60()
    {
        rt60 = getRt60();
        float typicalLoopMs = lateReflections.delayInMs * 1.5;
        float loopsPerRt60 = rt60 / (typicalLoopMs * 0.001);
        float dbPerCycle = -60 / loopsPerRt60;
        decayValue = std::pow(10, dbPerCycle * 0.05);
        lateReflections.updateDecay(decayValue);
    }
    
    void updateDamping(float dampingFreq)
    {
        lateReflections.dampingFilter.setCutoffFrequency(dampingFreq);
        lateDiffuser.updateDamping(dampingFreq);
    }
private:
    
    float getRt60() const
    {
        return *treeState.getRawParameterValue("decay");
    }
    
    double specSampleRate;
    float rt60;
    
    float decayValue;
    float sizeValue;
    
    MultiChannelDelay<channels> earlyReflections;
    MultiChannelDelay<channels> lateReflections;
    Diffuser<channels, 2> earlyDiffuser{50};
    Diffuser<channels, diffusionSteps> lateDiffuser{800};

    juce::AudioProcessorValueTreeState& treeState;
};
