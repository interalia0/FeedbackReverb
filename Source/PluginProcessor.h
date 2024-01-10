/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "reverb/Reverb.h"
#include "dsp/mix.h"

//==============================================================================
/**
*/
class FeedbackReverbAudioProcessor  : public juce::AudioProcessor
                                      
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    FeedbackReverbAudioProcessor();
    ~FeedbackReverbAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState treeState {*this, nullptr, "Parameters", createParameterLayout()};
    
private:
    void setFilters();
    float filterProcess(int channel, float inputSample);
    
    static const int revChannels = 12;

    std::array<float, 2> inputArray;
    std::array<float, 2> outputArray;
    
    std::array<float, revChannels> upMixed;
    std::array<float, revChannels> earlyRefArray;
    std::array<float, revChannels> lateRefArray;
    std::array<float, revChannels> processed;

    juce::AudioBuffer<float> earlyRefBuffer;
    juce::AudioBuffer<float> lateRefBuffer;

    signalsmith::mix::StereoMultiMixer<float, revChannels> multiMix;

    Reverb<revChannels, 4> earlyReverb;
    Reverb<revChannels, 6> lateReverb;
    
    juce::dsp::StateVariableTPTFilter<float> lowcut;
    juce::dsp::DryWetMixer<float> dryWet;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FeedbackReverbAudioProcessor)
};
