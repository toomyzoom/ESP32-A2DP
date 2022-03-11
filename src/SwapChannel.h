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

#include "esp_log.h"

/**
 * @brief Handle swapping left and right channel
 */
class SwapChannel {

    private:

        bool is_enabled = false;

        template <class TYPE_T>
        void update_data(TYPE_T *data, uint32_t len)
        {
            const int32_t channels = 2;
            int32_t frameCount = len / sizeof(TYPE_T);
            for (int i = 0; i < frameCount; i += channels, data += channels) {
                TYPE_T temp = data[0];
                data[0] = data[1];
                data[1] = temp;
            }
        }

    public:

        void update_audio_data(uint8_t* data, uint32_t len, uint32_t bps)
        {
            if (data == nullptr || len == 0 || !is_enabled) {
                return;
            }

            if (bps == 32) {
                update_data((int32_t*)data, len);
            } else if (bps == 16) {
                update_data((int16_t*)data, len);
            } else {
                ESP_LOGE("SwapChannel", "Unsupported bits per sample %u", bps);
            }
        }

        void set_enabled(bool enabled) {
            is_enabled = enabled;
        }
};
