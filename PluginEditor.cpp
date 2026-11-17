#include "PluginProcessor.h"
#include "PluginEditor.h"

HarmonicSeriesSynthAudioProcessorEditor::HarmonicSeriesSynthAudioProcessorEditor (HarmonicSeriesSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setupKnob (rootNoteSlider, rootLabel, "Root MIDI (24=C1)");
    setupKnob (tiltSlider, tiltLabel, "Harmonic Tilt");

    wave1Box.addItemList ({ "Saw", "Square", "Triangle", "Sine" }, 1);
    addAndMakeVisible (wave1Box);
    wave1Label.setText ("Osc 1", juce::dontSendNotification);
    wave1Label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (wave1Label);

    wave2Box.addItemList ({ "Saw", "Square", "Triangle", "Sine" }, 1);
    addAndMakeVisible (wave2Box);
    wave2Label.setText ("Osc 2", juce::dontSendNotification);
    wave2Label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (wave2Label);

    setupKnob (detuneSlider, detuneLabel, "Detune (ct)");
    setupKnob (mixSlider, mixLabel, "Osc Mix");

    setupKnob (cutoffSlider, cutoffLabel, "Cutoff (Hz)");
    setupKnob (resoSlider, resoLabel, "Resonance");
    setupKnob (filterEnvSlider, filterEnvLabel, "Filter Env");

    setupKnob (attackSlider, attackLabel, "Attack");
    setupKnob (decaySlider, decayLabel, "Decay");
    setupKnob (sustainSlider, sustainLabel, "Sustain");
    setupKnob (releaseSlider, releaseLabel, "Release");
    setupKnob (gainSlider, gainLabel, "Master Vol");

    rootAtt      = std::make_unique<SliderAtt> (audioProcessor.apvts, "ROOT_NOTE", rootNoteSlider);
    tiltAtt      = std::make_unique<SliderAtt> (audioProcessor.apvts, "TILT", tiltSlider);
    wave1Att     = std::make_unique<ComboAtt>  (audioProcessor.apvts, "WAVE1", wave1Box);
    wave2Att     = std::make_unique<ComboAtt>  (audioProcessor.apvts, "WAVE2", wave2Box);
    detuneAtt    = std::make_unique<SliderAtt> (audioProcessor.apvts, "DETUNE", detuneSlider);
    mixAtt       = std::make_unique<SliderAtt> (audioProcessor.apvts, "OSC_MIX", mixSlider);
    cutoffAtt    = std::make_unique<SliderAtt> (audioProcessor.apvts, "CUTOFF", cutoffSlider);
    resoAtt      = std::make_unique<SliderAtt> (audioProcessor.apvts, "RESO", resoSlider);
    filterEnvAtt = std::make_unique<SliderAtt> (audioProcessor.apvts, "FILTER_ENV", filterEnvSlider);
    attackAtt    = std::make_unique<SliderAtt> (audioProcessor.apvts, "ATTACK", attackSlider);
    decayAtt     = std::make_unique<SliderAtt> (audioProcessor.apvts, "DECAY", decaySlider);
    sustainAtt   = std::make_unique<SliderAtt> (audioProcessor.apvts, "SUSTAIN", sustainSlider);
    releaseAtt   = std::make_unique<SliderAtt> (audioProcessor.apvts, "RELEASE", releaseSlider);
    gainAtt      = std::make_unique<SliderAtt> (audioProcessor.apvts, "GAIN", gainSlider);

    setSize (760, 420);
}

HarmonicSeriesSynthAudioProcessorEditor::~HarmonicSeriesSynthAudioProcessorEditor() {}

void HarmonicSeriesSynthAudioProcessorEditor::setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 65, 18);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::Font (12.0f));
    addAndMakeVisible (label);
}

void HarmonicSeriesSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff16181d));

    g.setColour (juce::Colour (0xff222630));
    g.fillRect (0, 0, getWidth(), 45);
    g.setColour (juce::Colour (0xff64dfdf));
    g.setFont (juce::Font (20.0f, juce::Font::bold));
    g.drawText ("HARMONIC SERIES SYNTH // OVERTONE KEYBOARD", 20, 10, 500, 25, juce::Justification::left);

    g.setColour (juce::Colour (0xff1f232c));
    g.fillRoundedRectangle (15, 60, 210, 160, 8.0f);
    g.fillRoundedRectangle (240, 60, 505, 160, 8.0f);
    g.fillRoundedRectangle (15, 235, 300, 165, 8.0f);
    g.fillRoundedRectangle (330, 235, 415, 165, 8.0f);

    g.setColour (juce::Colour (0xff80ffdb));
    g.setFont (juce::Font (13.0f, juce::Font::bold));
    g.drawText ("1. HARMONIC MAPPING", 25, 68, 190, 20, juce::Justification::left);
    g.drawText ("2. DUAL OSCILLATORS", 250, 68, 250, 20, juce::Justification::left);
    g.drawText ("3. STATE-VARIABLE FILTER", 25, 243, 250, 20, juce::Justification::left);
    g.drawText ("4. AMPLIFIER ENVELOPE & OUTPUT", 340, 243, 300, 20, juce::Justification::left);
}

void HarmonicSeriesSynthAudioProcessorEditor::resized()
{
    auto placeKnob = [] (juce::Slider& s, juce::Label& l, int x, int y)
    {
        l.setBounds (x, y, 85, 18);
        s.setBounds (x + 5, y + 20, 75, 85);
    };

    placeKnob (rootNoteSlider, rootLabel, 25, 95);
    placeKnob (tiltSlider, tiltLabel, 125, 95);

    wave1Label.setBounds (255, 100, 95, 18);
    wave1Box.setBounds   (255, 122, 95, 26);
    wave2Label.setBounds (365, 100, 95, 18);
    wave2Box.setBounds   (365, 122, 95, 26);
    placeKnob (detuneSlider, detuneLabel, 480, 95);
    placeKnob (mixSlider, mixLabel, 585, 95);

    placeKnob (cutoffSlider, cutoffLabel, 25, 270);
    placeKnob (resoSlider, resoLabel, 120, 270);
    placeKnob (filterEnvSlider, filterEnvLabel, 215, 270);

    placeKnob (attackSlider, attackLabel, 340, 270);
    placeKnob (decaySlider, decayLabel, 420, 270);
    placeKnob (sustainSlider, sustainLabel, 500, 270);
    placeKnob (releaseSlider, releaseLabel, 580, 270);
    placeKnob (gainSlider, gainLabel, 655, 270);
}
