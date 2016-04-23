/*
 * replaygainsettings.cpp
 *
 *  Created on: 09.03.2016
 *      Author: daniel
 */

#include "preferences/replaygainsettings.h"

namespace {
const char* kConfigKey = "[ReplayGain]";

const char* kInitialReplayGainBoost = "InitialReplayGainBoost";
const char* kInitialDefaultBoost = "InitialDefaultBoost";
// WARNING: Do not fix the "analyser" spelling here since user config files
// contain these strings.
const char* kReplayGainAnalyzerEnabled = "ReplayGainAnalyserEnabled";
const char* kReplayGainAnalyzerVersion = "ReplayGainAnalyserVersion";
const char* kReplayGainReanalyze = "ReplayGainReanalyze";

const char* kReplayGainEnabled = "ReplayGainEnabled";

const char* kInitialDefaultBoostDefault = "-6";
} // anonymous namespace

ReplayGainSettings::ReplayGainSettings(UserSettingsPointer pConfig)
    : m_pConfig(pConfig) {
}

int ReplayGainSettings::getInitialReplayGainBoost() const {
    return m_pConfig->getValueString(
            ConfigKey(kConfigKey, kInitialReplayGainBoost), "0").toInt();
}

void ReplayGainSettings::setInitialReplayGainBoost(int value) {
    m_pConfig->set(ConfigKey(kConfigKey, kInitialReplayGainBoost),
            ConfigValue(value));
}

int ReplayGainSettings::getInitialDefaultBoost() const {
    return m_pConfig->getValueString(ConfigKey(kConfigKey, kInitialDefaultBoost),
            kInitialDefaultBoostDefault).toInt();
}

void ReplayGainSettings::setInitialDefaultBoost(int value) {
    m_pConfig->set(ConfigKey(kConfigKey, kInitialDefaultBoost),
                ConfigValue(value));
}

bool ReplayGainSettings::getReplayGainEnabled() const {
    return m_pConfig->getValueString(
        ConfigKey(kConfigKey, kReplayGainEnabled), "1").toInt() == 1;
}

void ReplayGainSettings::setReplayGainEnabled(bool value) {
    if (value) {
        m_pConfig->set(ConfigKey(kConfigKey, kReplayGainEnabled), ConfigValue(1));
    } else {
        m_pConfig->set(ConfigKey(kConfigKey, kReplayGainEnabled), ConfigValue(0));
    }
}

bool ReplayGainSettings::getReplayGainAnalyzerEnabled() const {
    return m_pConfig->getValueString(
        ConfigKey(kConfigKey, kReplayGainAnalyzerEnabled), "1").toInt();
}

void ReplayGainSettings::setReplayGainAnalyzerEnabled(bool value) {
    m_pConfig->set(ConfigKey(kConfigKey, kReplayGainAnalyzerEnabled),
                ConfigValue(value));
}

int ReplayGainSettings::getReplayGainAnalyzerVersion() const {
    return m_pConfig->getValueString(
            ConfigKey(kConfigKey, kReplayGainAnalyzerVersion), "2").toInt();
}

void ReplayGainSettings::setReplayGainAnalyzerVersion(int value) {
    m_pConfig->set(ConfigKey(kConfigKey, kReplayGainAnalyzerVersion),
            ConfigValue(value));
}

bool ReplayGainSettings::getReplayGainReanalyze() const {
    return m_pConfig->getValueString(
        ConfigKey(kConfigKey, kReplayGainReanalyze)).toInt() > 0;
}

void ReplayGainSettings::setReplayGainReanalyze(bool value) {
    m_pConfig->set(ConfigKey(kConfigKey, kReplayGainReanalyze),
                ConfigValue(value));
}

bool ReplayGainSettings::isAnalyzerDisabled(int version, TrackPointer tio) const {
    int prefversion = getReplayGainAnalyzerVersion();
    bool analyzerEnabled = getReplayGainAnalyzerEnabled() && (version == prefversion);
    bool reanalyze = getReplayGainReanalyze();

    if (analyzerEnabled) {
        if (reanalyze) {
            // ignore stored replay gain
            return false;
        }
        return tio->getReplayGain().hasRatio();
    }
    // not enabled, pretend we have already a stored value.
    return true;
}