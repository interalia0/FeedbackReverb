/*
  ==============================================================================

    HouseholderMatrix.h
    Created: 27 Mar 2023 7:53:30pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class HouseholderMixer
{
public:
    HouseholderMixer(int numChannels);

    void process(const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output);

private:
//    void generateMatrix();
    
    int size;
    const float factor = -2.0f / (size * (size + 1));
//    std::vector<std::vector<float>> matrix;
};
