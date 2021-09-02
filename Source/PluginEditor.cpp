/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleeqAudioProcessorEditor::SimpleeqAudioProcessorEditor (SimpleeqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    peakFreqSliderAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQualitySliderAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // Gets components for the editor
    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

SimpleeqAudioProcessorEditor::~SimpleeqAudioProcessorEditor()
{
}

//==============================================================================
void SimpleeqAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    auto bounds = getBounds();
    auto responseCurveArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    auto width = responseCurveArea.getWidth();
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> mags;
    mags.resize(width);
    
    for ( int i = 0; i < width; i++ )
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(width), 20.0, 20000.0);
        
        if (! monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        mags[i] = Decibels::gainToDecibels(mag);
    }
    
    Path responseCurve;
    const double outputMin = responseCurveArea.getBottom();
    const double outputMax = responseCurveArea.getY();
    
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseCurveArea.getX(), map(mags.front()));
    
    for ( int i = 1; i < mags.size(); i++ )
    {
        responseCurve.lineTo(responseCurveArea.getX() + i, map(mags[i]));
    }
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseCurveArea.toFloat(), 4.f, 1.f);
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

void SimpleeqAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    // bottom 2/3 of the GUI will be dedicated to sliders
    
    // retrieve the bounds of GUI
    auto bounds = getBounds();
    // remove 33% of the area from top for the response curve
    bounds.removeFromTop(bounds.getHeight() * 0.33);
    // remove 33% of area from left for low cut params
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    // remove 50% of remaining area (which is 33 of total) for high cut params
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    highCutSlopeSlider.setBounds(highCutArea);
    
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> SimpleeqAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}
