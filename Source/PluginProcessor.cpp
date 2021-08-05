/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleeqAudioProcessor::SimpleeqAudioProcessor()
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

SimpleeqAudioProcessor::~SimpleeqAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleeqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleeqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleeqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleeqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleeqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleeqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleeqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleeqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleeqAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleeqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleeqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SimpleeqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleeqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleeqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool SimpleeqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleeqAudioProcessor::createEditor()
{
    // comment out and return GenericAudioProcssorEditor to test
    // audio parameters
    
//    return new SimpleeqAudioProcessorEditor (*this);

    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleeqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleeqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

// Here we call our createParameterLayout() function and return the layout
juce::AudioProcessorValueTreeState::ParameterLayout SimpleeqAudioProcessor::createParameterLayout()
{
    // for more info: https://docs.juce.com/master/classAudioProcessorParameter.html
    // The AudioProcessorParameter class has several derived types
    // that represent the following types of parameter layouts
    // * Sliders -> wide range of values (AudioParameterFloat)
    // * Switches
    // * Combo Boxes
    // * IntInputs
    
    // juce::AudioParameterFloat
    // @params:
    // * parameterID (String)
    // * parameterName (Name)
    // * normalisableRange (juce::NormalizableRange<float>)
    //      * typename ValueType (float)
    //      * rangeStart
    //      * rangeEnd
    //      * intervalValue (steps, e.g. 1 -> 20hz, 21hz, etc.... OR 10 -> 20hz, 30hz, etc...)
    //      * skewFactor:
    //          * is 1.0 no skew
    //          * < 1.0 lower end of range will fill more of slider's length
    //          * > 1.0 higher end of range will fill more of slider's length
    // * defaultValue (where the slider begins)
    
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // lowcut freq parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", "LowCut Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 20.f));
    
    // highcut freq parameters
    // set defaultValue to 20000 hz
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq", "HighCut Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 20000.f));
    
    // peak freq parameters
    // Change default to 1 kHz
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq", "Peak Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 1000.f));
    
    // peak gain parameters
    // range ( -24.f, 24.f )
    // increments in 0.5 db's
    // defaultValue = 0.f
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain", "Peak Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    
    // peak quality parameters (how wide or narrow the peak band is)
    // range 0.f - 10.f
    // increments in 0.05f
    // defaultValue = 1.f
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality", "Peak Quality", juce::NormalisableRange<float>(0.f, 10.f, 0.05f, 1.f), 1.f));
    
    // LowCut and HighCut slopes (both filters will share the same slopes)
    // Set up 4 choices: 12, 24, 36, 48 dB/octave slopes (12 dB/oct default)
    // Here we use AudioParameterChoice obj instead since we are choosing
    // between 5 options instead of gliding through a stepped-range of options
    
    // juce::AudioParameterChoice()
    // @params
    // * parameterID (String)
    // * parameterName (Name)
    // * choices (StringArray)
    // * defaultItemIndex (int)
    
    // declare var stringArray of type juce::StringArray
    juce::StringArray stringArray;
    // construct a for loop
    for ( int i = 1; i < 5; ++i )
    {
        juce::String str;
        str << i * 12;
        str << " dB/oct";
        stringArray.add(str);
    }
    
    // add LowCut slope to layout
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    // add HighCut slope to layout
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));

    
    // audio parameters are saved in layout and returned to the AudioProcessorTreeValueState constructor (in PluginProcessor.h)
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleeqAudioProcessor();
}
