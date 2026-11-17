#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class HarmonicSeriesSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    HarmonicSeriesSynthAudioProcessorEditor (HarmonicSeriesSynthAudioProcessor&);
    ~HarmonicSeriesSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    HarmonicSeriesSynthAudioProcessor& audioProcessor;

    juce::Slider rootNoteSlider, tiltSlider, detuneSlider, mixSlider;
    juce::Slider cutoffSlider, resoSlider, filterEnvSlider;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, gainSlider;
    juce::ComboBox wave1Box, wave2Box;

    juce::Label titleLabel, tuningHeader, oscHeader, filterHeader, envHeader;
    juce::Label rootLabel, tiltLabel, wave1Label, wave2Label, detuneLabel, mixLabel;
    juce::Label cutoffLabel, resoLabel, filterEnvLabel;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, gainLabel;

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAtt> rootAtt, tiltAtt, detuneAtt, mixAtt;
    std::unique_ptr<SliderAtt> cutoffAtt, resoAtt, filterEnvAtt;
    std::unique_ptr<SliderAtt> attackAtt, decayAtt, sustainAtt, releaseAtt, gainAtt;
    std::unique_ptr<ComboAtt> wave1Att, wave2Att;

    void setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmonicSeriesSynthAudioProcessorEditor)
};
