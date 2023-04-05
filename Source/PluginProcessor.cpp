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
                       ), reverb(treeState)
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
    spec.numChannels = revChannels;
    
    reverb.prepare(spec);
    reverb.configure(sampleRate);
    lowpass.prepare(spec);
    highpass.prepare(spec);
    
    highpass.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    
    upmixedBuffer.setSize(revChannels, samplesPerBlock);
    outputBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    
}

void FeedbackReverbAudioProcessor::releaseResources()
{
    reverb.reset();
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
    
    const float mixValue = treeState.getRawParameterValue("mix")->load();
    const float scaling = multiMix.scalingFactor2();

    const float bufferNumSamples = buffer.getNumSamples();
    const float bufferNumChannels = buffer.getNumChannels();
        
    if (bufferNumChannels >= 2)
    {
        for (int sample = 0; sample < bufferNumSamples; ++sample)
        {
            std::array<float, 2> inputStereo = {buffer.getSample(0, sample), buffer.getSample(1, sample)};
            multiMix.stereoToMulti(inputStereo, upmixed);
            
            for (int channel = 0; channel < revChannels; ++channel)
            {
                upmixedBuffer.setSample(channel, sample, upmixed[channel]);
            }
        }
    }
    else if (bufferNumChannels == 1)
    {
        for (int sample = 0; sample < bufferNumSamples; ++sample)
        {
            std::array<float, 1> inputMono = {buffer.getSample(0, sample)};
            multiMix.stereoToMulti(inputMono, upmixed);
            
            for (int channel = 0; channel < revChannels; ++channel)
            {
                upmixedBuffer.setSample(channel, sample, upmixed[channel]);
            }
        }
    }
    
    reverb.setRt60();
    reverb.setSize();
    upmixedBuffer = reverb.processInPlace(upmixedBuffer);

    setFilters();
    
    for (int sample = 0; sample < bufferNumSamples; ++sample)
    {
        std::array<float, revChannels> processed;
        std::array<float, 2> downmixed;
        
        for (int channel = 0; channel < revChannels; ++channel)
        {
            processed[channel] = upmixedBuffer.getSample(channel, sample);
        }
        
        multiMix.multiToStereo(processed, downmixed);
                        
        for (int channel = 0; channel < 2; ++channel)
        {
            const float input = buffer.getSample(channel, sample);
            auto filteredSample = filterProcess(channel, downmixed[channel]);
            const float output = input * (1.0 - mixValue) + (filteredSample * mixValue * scaling);
            buffer.setSample(channel, sample, output);
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
//    return new FeedbackReverbAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void FeedbackReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
    treeState.state.writeToStream (mos);
}

void FeedbackReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data, static_cast<size_t> (sizeInBytes));

    if (tree.isValid())
        treeState.replaceState (tree);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FeedbackReverbAudioProcessor();
}

void FeedbackReverbAudioProcessor::setFilters()
{
    float lowpassCutoff = treeState.getRawParameterValue("highcut")->load();
    lowpass.setCutoffFrequency(lowpassCutoff);
    
    float highpassCutoff = treeState.getRawParameterValue("lowcut")->load();
    highpass.setCutoffFrequency(highpassCutoff);
}

float FeedbackReverbAudioProcessor::filterProcess(int channel, float inputSample)
{
    auto filteredSample = lowpass.processSample(channel, inputSample);
    filteredSample = highpass.processSample(channel, inputSample);
    
    return filteredSample;
}

juce::AudioProcessorValueTreeState::ParameterLayout
FeedbackReverbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    using pID = juce::ParameterID;
    using Range = juce::NormalisableRange<float>;
    layout.add(std::make_unique<juce::AudioParameterFloat> (pID{"decay", 1}, "Decay", Range{0.1, 30, 0.1}, 2));
    layout.add(std::make_unique<juce::AudioParameterFloat> (pID{"size", 1}, "Size", Range{50, 200, 1}, 60));
    layout.add(std::make_unique<juce::AudioParameterFloat> (pID{"highcut", 1}, "Highcut", Range{20, 20000, 1, 1.4}, 8000));
    layout.add(std::make_unique<juce::AudioParameterFloat> (pID{"lowcut", 1}, "Lowcut", Range{20, 20000, 1, 0.15}, 100));
    layout.add(std::make_unique<juce::AudioParameterFloat> (pID{"mix", 1}, "Mix", Range{0, 1, 0.01}, 0.5));
    return layout;
}
