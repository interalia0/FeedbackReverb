/*
  ==============================================================================

    HouseholderMatrix.cpp
    Created: 27 Mar 2023 7:53:30pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#include "HouseholderMatrix.h"

HouseholderMixer::HouseholderMixer(int numChannels) : size(numChannels)
{
    generateMatrix();
}

void HouseholderMixer::processInPlace(juce::AudioBuffer<float>& input)
{
    int numSamples = input.getNumSamples();
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int i = 0; i < size; ++i)
        {
            float sum = 0.0f;
            for (int j = 0; j < size; ++j)
            {
                sum += matrix[i][j] * input.getSample(j, sample);
            }
            input.setSample(i, sample, sum);
        }
    }
}

void HouseholderMixer::generateMatrix()
{
    matrix.resize(size, std::vector<float>(size, 0.0f));
    float factor = -2.0f / (size * (size + 1));
    for (int i = 0; i < size; ++i)
    {
        for (int j = 0; j < size; ++j)
        {
            if (i == j)
            {
                matrix[i][j] = 1.0f;
            }
            else
            {
                matrix[i][j] = factor * (1.0f / (i + j + 1));
            }
        }
    }
}
