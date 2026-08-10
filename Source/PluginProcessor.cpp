#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <BinaryData.h>

XSideAudioProcessor::XSideAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createLayout())
{
    auto json = juce::String::fromUTF8(
        BinaryData::factory_presets_json,
        BinaryData::factory_presets_jsonSize);

    factoryPresets = juce::JSON::parse(json);
    liveSlots.resize(12);
}

juce::AudioProcessorValueTreeState::ParameterLayout
XSideAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<juce::AudioParameterInt>("effect",    "Effect",    0, 31, 0));
    p.push_back(std::make_unique<juce::AudioParameterInt>("variation", "Variation", 0, 31, 0));
    p.push_back(std::make_unique<juce::AudioParameterInt>("engine",    "Engine",    0,  2, 0));
    p.push_back(std::make_unique<juce::AudioParameterInt>("editA",     "Param A",   0, 63, 0));
    p.push_back(std::make_unique<juce::AudioParameterInt>("editB",     "Param B",   0, 63, 0));
    p.push_back(std::make_unique<juce::AudioParameterInt>("lowEQ",     "Low EQ",    0, 32,16));
    p.push_back(std::make_unique<juce::AudioParameterInt>("highEQ",    "High EQ",   0, 32,16));
    p.push_back(std::make_unique<juce::AudioParameterInt>("dryWet",    "Dry/Wet",   0,100,100));
    p.push_back(std::make_unique<juce::AudioParameterBool>("bypass",   "Bypass", false));

    return { p.begin(), p.end() };
}

bool XSideAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    const auto& in  = l.getMainInputChannelSet();
    const auto& out = l.getMainOutputChannelSet();

    return in == out
        && (out == juce::AudioChannelSet::mono()
         || out == juce::AudioChannelSet::stereo());
}

void XSideAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(buffer);
}

juce::var XSideAudioProcessor::snapshot() const
{
    auto* o = new juce::DynamicObject();
    o->setProperty("name", currentPresetName);

    for (auto id : { "effect","variation","engine","editA",
                     "editB","lowEQ","highEQ","dryWet" })
        o->setProperty(id, apvts.getRawParameterValue(id)->load());

    return juce::var(o);
}

void XSideAudioProcessor::applySnapshot(const juce::var& v)
{
    if (!v.isObject())
        return;

    auto* o = v.getDynamicObject();
    currentPresetName = o->getProperty("name").toString();

    for (auto id : { "effect","variation","engine","editA",
                     "editB","lowEQ","highEQ","dryWet" })
    {
        if (o->hasProperty(id))
            if (auto* p = apvts.getParameter(id))
                p->setValueNotifyingHost(
                    p->convertTo0to1((float)o->getProperty(id)));
    }
}

void XSideAudioProcessor::loadFactoryPreset(const juce::var& srcVar)
{
    if (!srcVar.isObject())
        return;

    auto* src = srcVar.getDynamicObject();
    auto* o = new juce::DynamicObject();

    o->setProperty("name", src->getProperty("name"));

    struct Map { const char* dst; const char* src; };

    for (auto m : {
        Map{"effect","effect"},
        Map{"variation","variation"},
        Map{"engine","engine"},
        Map{"editA","edit_a"},
        Map{"editB","edit_b"},
        Map{"lowEQ","eq_lo"},
        Map{"highEQ","eq_hi"},
        Map{"dryWet","mix"} })
        o->setProperty(m.dst, src->getProperty(m.src));

    applySnapshot(juce::var(o));
}

void XSideAudioProcessor::storeLiveSlot(int slot)
{
    if (juce::isPositiveAndBelow(slot,12))
        liveSlots.set(slot,snapshot());
}

void XSideAudioProcessor::recallLiveSlot(int slot)
{
    if (juce::isPositiveAndBelow(slot,12) && !liveSlots[slot].isVoid())
        applySnapshot(liveSlots[slot]);
}

juce::String XSideAudioProcessor::getLiveSlotName(int slot) const
{
    if (!juce::isPositiveAndBelow(slot,12) || liveSlots[slot].isVoid())
        return "EMPTY";

    return liveSlots[slot].getProperty("name","LIVE").toString();
}

void XSideAudioProcessor::addSong(const juce::String& name)
{
    auto* s = new juce::DynamicObject();
    s->setProperty("name",name);
    s->setProperty("steps",juce::Array<juce::var>{});
    songs.add(juce::var(s));
}

void XSideAudioProcessor::addCurrentToSong(int songIndex)
{
    if (!juce::isPositiveAndBelow(songIndex,songs.size()))
        return;

    auto* s = songs.getReference(songIndex).getDynamicObject();
    if (!s) return;

    auto steps = s->getProperty("steps");

    if (auto* arr = steps.getArray())
        arr->add(snapshot());
    else
    {
        juce::Array<juce::var> a;
        a.add(snapshot());
        s->setProperty("steps",a);
    }
}

void XSideAudioProcessor::recallSongStep(int songIndex,int stepIndex)
{
    if (!juce::isPositiveAndBelow(songIndex,songs.size()))
        return;

    if (auto* arr = songs[songIndex].getProperty("steps",{}).getArray())
        if (juce::isPositiveAndBelow(stepIndex,arr->size()))
            applySnapshot((*arr)[stepIndex]);
}

int XSideAudioProcessor::getSongCount() const
{
    return songs.size();
}

juce::String XSideAudioProcessor::getSongName(int i) const
{
    return juce::isPositiveAndBelow(i,songs.size())
        ? songs[i].getProperty("name","Song").toString()
        : juce::String();
}

int XSideAudioProcessor::getSongStepCount(int i) const
{
    if (!juce::isPositiveAndBelow(i,songs.size()))
        return 0;

    if (auto* arr=songs[i].getProperty("steps",{}).getArray())
        return arr->size();

    return 0;
}

void XSideAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto root = apvts.copyState();

    root.setProperty("presetName",currentPresetName,nullptr);
    root.setProperty("live",juce::JSON::toString(juce::var(liveSlots)),nullptr);
    root.setProperty("songs",juce::JSON::toString(juce::var(songs)),nullptr);

    if (auto xml=root.createXml())
        copyXmlToBinary(*xml,dest);
}

void XSideAudioProcessor::setStateInformation(const void* data,int size)
{
    if (auto xml=getXmlFromBinary(data,size))
    {
        auto st=juce::ValueTree::fromXml(*xml);

        if (st.isValid())
        {
            apvts.replaceState(st);

            currentPresetName=
                st.getProperty("presetName","DSP Capture").toString();

            if (auto v=juce::JSON::parse(
                    st.getProperty("live","[]").toString()); v.isArray())
                liveSlots=*v.getArray();

            if (auto v=juce::JSON::parse(
                    st.getProperty("songs","[]").toString()); v.isArray())
                songs=*v.getArray();
        }
    }
}

juce::AudioProcessorEditor* XSideAudioProcessor::createEditor()
{
    return new XSideAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new XSideAudioProcessor();
}
