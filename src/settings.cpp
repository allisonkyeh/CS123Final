#include "settings.h"
#include <QFile>
#include <QSettings>

Settings settings;


/**
  Loads the application settings, or, if no saved settings are available,
  loads default values for the settings. You can change the defaults here.
**/
void Settings::loadSettingsOrDefaults() {
    // Set the default values below
    QSettings s("CS123", "Lab07");
    mode = s.value("mode", MODE_BLUR).toInt();
    mFrontImagePath = ":/images/posz.jpg";
    mBackImagePath = ":/images/negz.jpg";
    mTopImagePath = ":/images/posy.jpg";
    mBottomImagePath = ":/images/negy.jpg";
    mLeftImagePath = ":/images/negx.jpg";
    mRightImagePath = ":/images/posx.jpg";
}

void Settings::saveSettings() {
    QSettings s("CS123", "Lab07");
    s.setValue("mode", mode);
}


