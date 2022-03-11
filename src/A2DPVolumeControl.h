#pragma once

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Copyright 2020 Phil Schatzmann
// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD

#include "SoundData.h"
#include "esp_log.h"

/**
 * @brief Abstract class for handling of the volume of the audio data
 * @author Phil Schatzmann
 * @copyright Apache License Version 2
 */

class A2DPVolumeControl {

    private:

        template <class TYPE_T>
        void update_data_volume(TYPE_T *data, uint32_t len)
        {
            const int32_t channels = 2;
            int32_t frameCount = len / sizeof(TYPE_T);
            float vf = volumeFactor;
            float vfMax = volumeFactorMax;
            for (int i = 0; i < frameCount; i += channels){
                // if mono -> we provide the same output on both channels
                float pcmLeft = data[i];
                float pcmRight = data[i + 1];
                if (mono_downmix) {
                    pcmRight = pcmLeft = (pcmLeft + pcmRight) / 2.0f;
                }
                // adjust the volume
                if (is_volume_used) {
                    pcmLeft = pcmLeft * vf / vfMax;
                    pcmRight = pcmRight * vf / vfMax;
                }
                data[i] = (TYPE_T)round(pcmLeft);
                data[i + 1] = (TYPE_T)round(pcmRight);
            }
        }

    protected:
        bool is_volume_used = false;
        bool mono_downmix = false;
        int32_t volumeFactor;
        int32_t volumeFactorMax;

    public:
        A2DPVolumeControl() {
            volumeFactorMax = 0x1000;
        }

        virtual void update_audio_data(uint8_t* data, uint32_t len, uint32_t bps)
        {
            if (data == nullptr || len == 0 || (!mono_downmix && !is_volume_used)) {
                return;
            }

            ESP_LOGD("VolumeControl", "update_audio_data");
            if (bps == 32) {
                update_data_volume((int32_t*)data, len);
            } else if (bps == 16) {
                update_data_volume((int16_t*)data, len);
            } else {
                ESP_LOGE("VolumeControl", "Unsupported bits per sample %u", bps);
            }
        }

        // provides a factor in the range of 0 to 4096
        int32_t get_volume_factor() {
            return volumeFactor;
        }

        // provides the max factor value 4096
        int32_t get_volume_factor_max() {
            return volumeFactorMax;
        }

        void set_enabled(bool enabled) {
            is_volume_used = enabled;
        }

        void set_mono_downmix(bool enabled) {
            mono_downmix = enabled;
        }

        virtual void set_volume(uint8_t volume) = 0;
};

/**
 * @brief Default implementation for handling of the volume of the audio data
 * @author elehobica
 * @copyright Apache License Version 2
 */
class A2DPDefaultVolumeControl : public A2DPVolumeControl {

        virtual void set_volume(uint8_t volume) {
            constexpr double base = 1.4;
            constexpr double bits = 12;
            constexpr double zero_ofs = pow(base, -bits);
            constexpr double scale = pow(2.0, bits);
            double volumeFactorFloat = (pow(base, volume * bits / 127.0 - bits) - zero_ofs) * scale / (1.0 - zero_ofs);
            volumeFactor = volumeFactorFloat;
            if (volumeFactor > 0x1000) {
                volumeFactor = 0x1000;
            }
        }
};

/**
 * @brief  Exponentional volume control
 * @author rbruelma
 */
class A2DPSimpleExponentialVolumeControl : public A2DPVolumeControl {
        virtual void set_volume(uint8_t volume) {
            double volumeFactorFloat = volume;
            volumeFactorFloat = pow(2.0, volumeFactorFloat * 12.0 / 127.0);
            int32_t volumeFactor = volumeFactorFloat - 1.0;
            if (volumeFactor > 0xfff) {
                volumeFactor = 0xfff;
            }
        }
};

/**
 * @brief The simplest possible implementation of a VolumeControl
 * @author pschatzmann
 * @copyright Apache License Version 2
 */
class A2DPLinearVolumeControl : public A2DPVolumeControl {

        A2DPLinearVolumeControl() {
            volumeFactorMax = 128;
        }

        virtual void set_volume(uint8_t volume) {
            volumeFactor = volume;
        }
};

/**
 * @brief Keeps the audio data as is -> no volume control!
 * @author pschatzmann
 * @copyright Apache License Version 2
 */
class A2DPNoVolumeControl : public A2DPVolumeControl {
    public:
        virtual void update_audio_data(uint8_t* data, uint16_t len, uint32_t bps) {
        }
};