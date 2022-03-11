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

#include <algorithm>
#include "esp_log.h"

class SignedToUnsigned {

    private:

        template <class TYPE_S>
        void update_data(TYPE_S *data, uint32_t len, TYPE_S type_max)
        {
            int32_t frameCount = len / sizeof(TYPE_S);
            for (int i = 0; i < frameCount; i++) {
                    data[i] = data[i] + type_max;
            }
        }

    public:

        void update_audio_data(uint8_t* data, uint32_t len, uint32_t bps)
        {
            if (data == nullptr || len == 0) {
                return;
            }

            if (bps == 32) {
                update_data((int32_t*)data, len, (int32_t)0x80000000);
            } else if (bps == 16) {
                update_data((int16_t*)data, len, (int16_t)0x8000);
            } else {
                ESP_LOGE("SignedToUnsigned", "Unsupported bits per sample %u", bps);
            }
        }
};
