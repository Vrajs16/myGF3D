#include <gfc_audio.h>
#include "gfc_hashmap.h"

void setSoundPack(const char *filename);

void playSound(const char *name, int loops, float volume, int channel, int group);

void stopSound();