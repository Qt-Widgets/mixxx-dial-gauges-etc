/***************************************************************************
                          dlgprefeq.cpp  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgprefeq.h"
#include "engine/enginefilteriir.h"
#include <qlineedit.h>
#include <qwidget.h>
#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgraphicsscene.h>

#include <assert.h>

#define CONFIG_KEY "[Mixer Profile]"

const int kFrequencyUpperLimit = 20050;
const int kFrequencyLowerLimit = 16;

DlgPrefEQ::DlgPrefEQ(QWidget *pParent, ConfigObject<ConfigValue> *pConfig)
  : QWidget(pParent)
  , Ui::DlgPrefEQDlg()
#ifndef __LOFI__
  , m_COTLoFreq(ControlObject::getControl(ConfigKey(CONFIG_KEY, "LoEQFrequency")))
  , m_COTHiFreq(ControlObject::getControl(ConfigKey(CONFIG_KEY, "HiEQFrequency")))
  , m_COTLoFi(ControlObject::getControl(ConfigKey(CONFIG_KEY, "LoFiEQs")))
#endif
{
    m_pConfig = pConfig;

    setupUi(this);

    // Connection
#ifndef __LOFI__
    connect(SliderHiEQ, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateHiEQ()));
    connect(SliderHiEQ, SIGNAL(sliderReleased()), this, SLOT(slotUpdateHiEQ()));

    connect(SliderLoEQ, SIGNAL(valueChanged(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ, SIGNAL(sliderMoved(int)), this, SLOT(slotUpdateLoEQ()));
    connect(SliderLoEQ, SIGNAL(sliderReleased()), this, SLOT(slotUpdateLoEQ()));

    connect(CheckBoxLoFi, SIGNAL(stateChanged(int)), this, SLOT(slotLoFiChanged()));
#else
    CheckBoxLoFi->setChecked(true);
    slotLoFiChanged();
    CheckBoxLoFi->setEnabled(false);
#endif
    connect(PushButtonReset, SIGNAL(clicked(bool)), this, SLOT(reset()));

    m_lowEqFreq = 0;
    m_highEqFreq = 0;

    loadSettings();
}

DlgPrefEQ::~DlgPrefEQ()
{
}

void DlgPrefEQ::loadSettings()
{
    if (m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")) == QString("")) {
        // apparently we don't have any settings, set defaults
        CheckBoxLoFi->setChecked(true);
        setDefaultShelves();
    }
    SliderHiEQ->setValue(
        getSliderPosition(m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "HiEQFrequency")).toInt()));
    SliderLoEQ->setValue(
        getSliderPosition(m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "LoEQFrequency")).toInt()));

    if (m_pConfig->getValueString(ConfigKey(CONFIG_KEY, "LoFiEQs")) == QString("yes")) {
        CheckBoxLoFi->setChecked(true);
    } else {
        CheckBoxLoFi->setChecked(false);
    }

    slotUpdate();
    slotApply();
}

void DlgPrefEQ::setDefaultShelves()
{
    m_pConfig->set(ConfigKey(CONFIG_KEY, "HiEQFrequency"), ConfigValue(2500));
    m_pConfig->set(ConfigKey(CONFIG_KEY, "LoEQFrequency"), ConfigValue(250));
}

/** Resets settings, leaves LOFI box checked asis.
 */
void DlgPrefEQ::reset() {
    setDefaultShelves();
    loadSettings();
}

void DlgPrefEQ::slotLoFiChanged()
{
    GroupBoxHiEQ->setEnabled(!CheckBoxLoFi->isChecked());
    GroupBoxLoEQ->setEnabled(!CheckBoxLoFi->isChecked());
    if(CheckBoxLoFi->isChecked()) {
        m_pConfig->set(ConfigKey(CONFIG_KEY, "LoFiEQs"), ConfigValue(QString("yes")));
    } else {
        m_pConfig->set(ConfigKey(CONFIG_KEY, "LoFiEQs"), ConfigValue(QString("no")));
    }
    slotApply();
}

void DlgPrefEQ::slotUpdateHiEQ()
{
    if (SliderHiEQ->value() < SliderLoEQ->value())
    {
        SliderHiEQ->setValue(SliderLoEQ->value());
    }
    m_highEqFreq = getEqFreq(SliderHiEQ->value(),
                             SliderHiEQ->minimum(),
                             SliderHiEQ->maximum());
    validate_levels();
    if (m_highEqFreq < 1000) {
        TextHiEQ->setText( QString("%1 Hz").arg(m_highEqFreq));
    } else {
        TextHiEQ->setText( QString("%1 kHz").arg(m_highEqFreq / 1000.));
    }
    m_pConfig->set(ConfigKey(CONFIG_KEY, "HiEQFrequency"), ConfigValue(m_highEqFreq));

    slotApply();
}

void DlgPrefEQ::slotUpdateLoEQ()
{
    if (SliderLoEQ->value() > SliderHiEQ->value())
    {
        SliderLoEQ->setValue(SliderHiEQ->value());
    }
    m_lowEqFreq = getEqFreq(SliderLoEQ->value(),
                            SliderLoEQ->minimum(),
                            SliderLoEQ->maximum());
    validate_levels();
    if (m_lowEqFreq < 1000) {
        TextLoEQ->setText(QString("%1 Hz").arg(m_lowEqFreq));
    } else {
        TextLoEQ->setText(QString("%1 kHz").arg(m_lowEqFreq / 1000.));
    }
    m_pConfig->set(ConfigKey(CONFIG_KEY, "LoEQFrequency"), ConfigValue(m_lowEqFreq));

    slotApply();
}

int DlgPrefEQ::getSliderPosition(int eqFreq)
{
    if(eqFreq >= kFrequencyUpperLimit) {
        return 480;
    }
    double dsliderPos = pow(eqFreq, 1./4.);
    dsliderPos *= 40;
    return dsliderPos;
}


void DlgPrefEQ::slotApply()
{
#ifndef __LOFI__
    m_COTLoFreq.slotSet(m_lowEqFreq);
    m_COTHiFreq.slotSet(m_highEqFreq);
    m_COTLoFi.slotSet(CheckBoxLoFi->isChecked());
#endif
}

void DlgPrefEQ::slotUpdate()
{
    slotUpdateLoEQ();
    slotUpdateHiEQ();
    slotLoFiChanged();
}

int DlgPrefEQ::getEqFreq(int sliderVal, int minValue, int maxValue) {
    // We're mapping f(x) = x^4 onto the range kFrequencyLowerLimit,
    // kFrequencyUpperLimit with x [minValue, maxValue]. First translate x into
    // [0.0, 1.0], raise it to the 4th power, and then scale the result from
    // [0.0, 1.0] to [kFrequencyLowerLimit, kFrequencyUpperLimit].
    double normValue = static_cast<double>(sliderVal - minValue) /
            (maxValue - minValue);
    // Use a non-linear mapping between slider and frequency.
    normValue = normValue * normValue * normValue * normValue;
    double result = normValue * (kFrequencyUpperLimit-kFrequencyLowerLimit) +
            kFrequencyLowerLimit;
    return result;
}

void DlgPrefEQ::validate_levels() {
    m_highEqFreq = math_max(math_min(m_highEqFreq, kFrequencyUpperLimit),
                            kFrequencyLowerLimit);
    m_lowEqFreq = math_max(math_min(m_lowEqFreq, kFrequencyUpperLimit),
                           kFrequencyLowerLimit);
    if (m_lowEqFreq == m_highEqFreq) {
        if (m_lowEqFreq == kFrequencyLowerLimit) {
            ++m_highEqFreq;
        } else if (m_highEqFreq == kFrequencyUpperLimit) {
            --m_lowEqFreq;
        } else {
            ++m_highEqFreq;
        }
    }
}
