/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Cut filter slope dB/oct names
enum Slope {
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

// extract params values from AudioProcessorValueTreeState using data structure
struct ChainSettings
{
    float peakFreq { 0 }, peakGainInDecibles { 0 }, peakQuality {1.f};
    float lowCutFreq { 0 }, highCutFreq { 0 };
    Slope lowCutSlope { Slope::Slope_12 }, highCutSlop { Slope::Slope_12 };
};

// helper function that will pass params into the data structure
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class SimpleeqAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleeqAudioProcessor();
    ~SimpleeqAudioProcessor() override;

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
    
    // Notes:
    // * Audio plugins depend on parameters that control the DSP (and GUI)
    // * JUCE uses the object AudioProcessorValueTreeState to coordinate
    // * the parameter controls and sync up the parameters
    //   with the vars in DSP and knobs on GUI
    
    // apvts expects a list of all parameters when it is created
    // this function provides the parameters as an apvts ParameterLayout
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    
    // Here we declare an AudioProcessorValueTreeState obj called apvts
    // @params:
    // * AudioProcessorToConnectTo = *this
    // * UndoManager = nullptr (not using)
    // * Identifier = "Parameters"
    // * ParameterLayour = createParameterLayout()
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};

private:
    // create alias for our normal filters (Peak/Parametric)
    using Filter = juce::dsp::IIR::Filter<float>;
    
    // create alias for our cut filters
    // We chain 4 filters to the processor chain to represent each of the
    // optional cut filter slopes (12, 24, 36, 48 dB/oct)
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    
    // create a mono signal chain of our 3 filters (LowCut -> Peak -> HighCut
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
    
    // need two instances if we want to do stereo processing
    MonoChain leftChain, rightChain;
    
    // before we use our filter chains we need to prepare them
    // see prepareToPlay method in PluginProcessor.cpp
    
    enum ChainPositions
    {
        LowCut,
        Peak,
        HighCut
    };
    
    void updatePeakFilter(const ChainSettings& chainSettings);
    using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);
    
    template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftLowCut,
                         const CoefficientType& cutCoefficients,
                         const Slope& lowCutSlope)
    {
        // bypasses all 4 positions in the chain
        leftLowCut.template setBypassed<0>(true);
        leftLowCut.template setBypassed<1>(true);
        leftLowCut.template setBypassed<2>(true);
        leftLowCut.template setBypassed<3>(true);
        // want to switch filters based on dB/oct name
        // see enum defined in header
        // NOTE: enums constants are of int type which is what the AudioParameterChoice object is expressed in
        
//        switch ( chainSettings.lowCutSlope )
        switch ( lowCutSlope )
        {
                // switch cases
            case Slope_12:
            {   // for each switch case we add another filter coefficient
                // and switch bypass off in the filter chain position
                *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
                leftLowCut.template setBypassed<0>(false);
                break;
            }
            case Slope_24:
            {
                *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
                leftLowCut.template setBypassed<0>(false);
                *leftLowCut.template get<1>().coefficients = *cutCoefficients[1];
                leftLowCut.template setBypassed<1>(false);
                break;
            }
            case Slope_36:
            {
                *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
                leftLowCut.template setBypassed<0>(false);
                *leftLowCut.template get<1>().coefficients = *cutCoefficients[1];
                leftLowCut.template setBypassed<1>(false);
                *leftLowCut.template get<2>().coefficients = *cutCoefficients[2];
                leftLowCut.template setBypassed<2>(false);
                break;
            }
            case Slope_48:
            {
                *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
                leftLowCut.template setBypassed<0>(false);
                *leftLowCut.template get<1>().coefficients = *cutCoefficients[1];
                leftLowCut.template setBypassed<1>(false);
                *leftLowCut.template get<2>().coefficients = *cutCoefficients[2];
                leftLowCut.template setBypassed<2>(false);
                *leftLowCut.template get<3>().coefficients = *cutCoefficients[3];
                leftLowCut.template setBypassed<3>(false);
                break;
            }
        }
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleeqAudioProcessor)
};
