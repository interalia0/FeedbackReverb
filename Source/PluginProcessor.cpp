/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FeedbackReverbAudioProcessor::FeedbackReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

FeedbackReverbAudioProcessor::~FeedbackReverbAudioProcessor()
{
}

//==============================================================================
const juce::String FeedbackReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FeedbackReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FeedbackReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FeedbackReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FeedbackReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FeedbackReverbAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FeedbackReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FeedbackReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String FeedbackReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void FeedbackReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FeedbackReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 8;
    delay.prepare(spec);
    delay.configure(sampleRate);
    diffuser.prepare(spec);
    diffuser.configure(sampleRate);
    
    
    upmixedBuffer.setSize(revChannels, samplesPerBlock);

    
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void FeedbackReverbAudioProcessor::releaseResources()
{
    delay.reset();
    diffuser.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FeedbackReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void FeedbackReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        std::array<float, 2> inputStereo = {buffer.getSample(0, sample), buffer.getSample(1, sample)};
        std::array<float, 8> upmixed;
        
        multiMix.stereoToMulti(inputStereo, upmixed);
        
        for (int channel = 0; channel < 8; ++channel)
        {
            upmixedBuffer.setSample(channel, sample, upmixed[channel]);
        }
    }
    
    diffuser.processInPlace(upmixedBuffer);
    delay.processInPlace(upmixedBuffer);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        std::array<float, 8> processed;
        std::array<float, 2> downmixed;
        
        for (int channel = 0; channel < 8; ++channel)
        {
            processed[channel] = upmixedBuffer.getSample(channel, sample);
        }
        
        multiMix.multiToStereo(processed, downmixed);
        
        const float scaling = multiMix.scalingFactor2();
        
        for (int channel = 0; channel < 2; ++channel)
        {
            buffer.setSample(channel, sample, downmixed[channel] * scaling);
        }
    }
}

//==============================================================================
bool FeedbackReverbAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FeedbackReverbAudioProcessor::createEditor()
{
    return new FeedbackReverbAudioProcessorEditor (*this);
}

//==============================================================================
void FeedbackReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FeedbackReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FeedbackReverbAudioProcessor();
}
