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

    void processInPlace(juce::AudioBuffer<float>& input);

private:
    void generateMatrix();
    
    int size;
    std::vector<std::vector<float>> matrix;
};
