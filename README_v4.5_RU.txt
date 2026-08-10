X-SIDE VST3 v4.5 SAFE MINIMAL

Это диагностическая версия для Ableton Live.

Из неё намеренно удалено всё, кроме:
- VST3 processor;
- pass-through audio;
- основных VST3 parameters;
- очень простого GUI.

НЕТ:
- MIDI enumeration;
- MIDI open;
- factory preset JSON usage;
- LIVE;
- SONGS;
- tabs;
- timers;
- async MIDI;
- модальных окон.

Цель:
Если v4.5 открывается в Ableton — проблема точно в одном из удалённых модулей,
и дальше функции будут возвращаться по одной.

Если даже v4.5 падает — значит искать нужно уже в базовой конфигурации
JUCE/VST3/AudioProcessor, а не в нашей MIDI/LIVE/SONGS логике.
