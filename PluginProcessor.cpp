#include "PluginProcessor.h"
#include "PluginEditor.h"

struct HarmonicSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class HarmonicVoice : public juce::SynthesiserVoice
{
public:
    HarmonicVoice (juce::AudioProcessorValueTreeState& state) : apvts (state) {}

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<HarmonicSound*> (sound) != nullptr;
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
        osc1.prepare (spec);
        osc2.prepare (spec);
        filter.prepare (spec);
        filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        adsr.setSampleRate (sampleRate);
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override
    {
        currentMidiNote = midiNoteNumber;
        noteVelocity = velocity;
        pitchBend = (currentPitchWheelPosition - 8192) / 8192.0f;

        updateParameters (true);
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            adsr.noteOff();
        }
        else
        {
            clearCurrentNote();
            adsr.reset();
        }
    }

    void pitchWheelMoved (int newPitchWheelValue) override
    {
        pitchBend = (newPitchWheelValue - 8192) / 8192.0f;
    }

    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        updateParameters (false);

        for (int i = 0; i < numSamples; ++i)
        {
            float env = adsr.getNextSample();
            if (! adsr.isActive())
            {
                clearCurrentNote();
                break;
            }

            float sample1 = osc1.processSample (0.0f);
            float sample2 = osc2.processSample (0.0f);
            float mixed = ((1.0f - oscMix) * sample1 + oscMix * sample2) * harmonicGain * noteVelocity;

            float filtered = filter.processSample (0, mixed) * env * masterGain;

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample (ch, startSample + i, filtered);
        }
    }

private:
    void updateParameters (bool forceReset)
    {
        int rootNote = (int) *apvts.getRawParameterValue ("ROOT_NOTE");
        float tilt = *apvts.getRawParameterValue ("TILT");
        int wave1 = (int) *apvts.getRawParameterValue ("WAVE1");
        int wave2 = (int) *apvts.getRawParameterValue ("WAVE2");
        float detuneCents = *apvts.getRawParameterValue ("DETUNE");
        oscMix = *apvts.getRawParameterValue ("OSC_MIX");
        float cutoff = *apvts.getRawParameterValue ("CUTOFF");
        float reso = *apvts.getRawParameterValue ("RESO");
        float envAmt = *apvts.getRawParameterValue ("FILTER_ENV");
        masterGain = *apvts.getRawParameterValue ("GAIN");

        int stepDiff = currentMidiNote - rootNote;
        double baseFreq = juce::MidiMessage::getMidiNoteInHertz (rootNote);
        double targetFreq = baseFreq;
        float effectiveHarmonic = 1.0f;

        if (stepDiff >= 0)
        {
            effectiveHarmonic = (float) (stepDiff + 1);
            targetFreq = baseFreq * effectiveHarmonic;
        }
        else
        {
            float subDiv = (float) (std::abs (stepDiff) + 1);
            targetFreq = baseFreq / subDiv;
            effectiveHarmonic = 1.0f;
        }

        targetFreq *= std::pow (2.0, (pitchBend * 2.0) / 12.0);

        double nyquist = getSampleRate() * 0.48;
        targetFreq = juce::jlimit (15.0, nyquist, targetFreq);

        float rawBalance = 1.0f / std::pow (effectiveHarmonic, tilt);
        harmonicGain = juce::jlimit (0.12f, 1.2f, rawBalance * 1.4f);

        setOscWaveform (osc1, wave1);
        setOscWaveform (osc2, wave2);

        double detuneRatio = std::pow (2.0, detuneCents / 1200.0);
        osc1.setFrequency ((float) targetFreq, forceReset);
        osc2.setFrequency ((float) (targetFreq * detuneRatio), forceReset);

        float modCutoff = juce::jlimit (20.0f, 20000.0f, cutoff + (envAmt * 8000.0f * (adsr.isActive() ? 1.0f : 0.0f)));
        filter.setCutoffFrequency (modCutoff);
        filter.setResonance (reso);

        juce::ADSR::Parameters adsrParams;
        adsrParams.attack  = *apvts.getRawParameterValue ("ATTACK");
        adsrParams.decay   = *apvts.getRawParameterValue ("DECAY");
        adsrParams.sustain = *apvts.getRawParameterValue ("SUSTAIN");
        adsrParams.release = *apvts.getRawParameterValue ("RELEASE");
        adsr.setParameters (adsrParams);
    }

    void setOscWaveform (juce::dsp::Oscillator<float>& osc, int type)
    {
        if (type == 0)
            osc.initialise ([] (float x) { return x / juce::MathConstants<float>::pi; }, 128);
        else if (type == 1)
            osc.initialise ([] (float x) { return x < 0.0f ? -1.0f : 1.0f; }, 128);
        else if (type == 2)
            osc.initialise ([] (float x) { return 2.0f * std::abs (x / juce::MathConstants<float>::pi) - 1.0f; }, 128);
        else
            osc.initialise ([] (float x) { return std::sin (x); }, 128);
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::dsp::Oscillator<float> osc1, osc2;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::ADSR adsr;

    int currentMidiNote = 36;
    float noteVelocity = 0.0f;
    float pitchBend = 0.0f;
    float oscMix = 0.5f;
    float harmonicGain = 1.0f;
    float masterGain = 0.5f;
};

HarmonicSeriesSynthAudioProcessor::HarmonicSeriesSynthAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameters())
{
    for (int i = 0; i < 12; ++i)
        synth.addVoice (new HarmonicVoice (apvts));

    synth.addSound (new HarmonicSound());
}

HarmonicSeriesSynthAudioProcessor::~HarmonicSeriesSynthAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout HarmonicSeriesSynthAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterInt> ("ROOT_NOTE", "Fundamental MIDI Note", 12, 60, 24));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("TILT", "Harmonic Tilt", 0.0f, 1.0f, 0.35f));

    juce::StringArray waves { "Saw", "Square", "Triangle", "Sine" };
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("WAVE1", "Osc 1 Wave", waves, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("WAVE2", "Osc 2 Wave", waves, 2));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DETUNE", "Osc 2 Detune", -50.0f, 50.0f, 6.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("OSC_MIX", "Osc Mix", 0.0f, 1.0f, 0.35f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("CUTOFF", "Filter Cutoff",
        juce::NormalisableRange<float> (40.0f, 18000.0f, 1.0f, 0.3f), 4500.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RESO", "Resonance", 0.5f, 8.0f, 1.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("FILTER_ENV", "Filter Env", 0.0f, 1.0f, 0.2f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", 0.005f, 3.0f, 0.02f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DECAY", "Decay", 0.01f, 3.0f, 0.4f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SUSTAIN", "Sustain", 0.0f, 1.0f, 0.75f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", 0.01f, 5.0f, 0.45f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("GAIN", "Master Volume", 0.0f, 1.0f, 0.4f));

    return { params.begin(), params.end() };
}

void HarmonicSeriesSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<HarmonicVoice*> (synth.getVoice (i)))
            voice->prepareToPlay (sampleRate, samplesPerBlock);
    }
}

void HarmonicSeriesSynthAudioProcessor::releaseResources() {}

bool HarmonicSeriesSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void HarmonicSeriesSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* HarmonicSeriesSynthAudioProcessor::createEditor()
{
    return new HarmonicSeriesSynthAudioProcessorEditor (*this);
}

void HarmonicSeriesSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void HarmonicSeriesSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HarmonicSeriesSynthAudioProcessor();
}
