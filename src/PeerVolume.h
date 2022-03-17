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

#pragma once

#include <algorithm>
#include <deque>
#include <utility>
#include "esp_bt.h"
#include "esp_log.h"


/*
 * Store volume for bluetooth a2dp sink connections.
 */
class PeerVolume {

private:

    uint8_t max_size;
    std::deque<std::pair<uint64_t, uint8_t>> volume;

    uint8_t default_volume;

    inline uint64_t peer_addr_to_int(const esp_bd_addr_t peer_bd_addr) const
    {
        return ((uint64_t)peer_bd_addr[5] << 40) |
               ((uint64_t)peer_bd_addr[4] << 32) |
               ((uint64_t)peer_bd_addr[3] << 24) |
               ((uint64_t)peer_bd_addr[2] << 16) |
               ((uint64_t)peer_bd_addr[1] << 8) |
               ((uint64_t)peer_bd_addr[0] << 0);
    }

public:

    PeerVolume(uint8_t max_size = 5, uint8_t default_volume = 0) {
        this->max_size = max_size;
        set_default_volume(default_volume);
    }

    void set_default_volume(uint8_t default_volume)
    {
        this->default_volume = std::min((uint8_t)127, default_volume);
    }

    uint8_t get(const esp_bd_addr_t bd_addr)
    {
        uint64_t addr_u64 = peer_addr_to_int(bd_addr);
        auto it  = std::find_if(volume.begin(), volume.end(),
                                [&]( const std::pair<uint64_t, uint8_t> v )
                                {
                                    return v.first == addr_u64;
                                });

        if (it != volume.end()) {
            return (*it).second;
        } else {
            return default_volume;
        }
    }

    uint8_t& operator [](const esp_bd_addr_t bd_addr)
    {
        uint64_t addr_u64 = peer_addr_to_int(bd_addr);
        auto it  = std::find_if(volume.begin(), volume.end(),
                                [&]( const std::pair<uint64_t, uint8_t> v )
                                {
                                    return v.first == addr_u64;
                                });

        if (it == volume.end()) {
            while (volume.size() > this->max_size) {
                volume.pop_front();
            }
            volume.push_back(std::make_pair(addr_u64, default_volume));
            return volume.back().second;
        } else {
            return (*it).second;
        }
    }

    bool contains(esp_bd_addr_t bd_addr)
    {
        uint64_t addr_u64 = peer_addr_to_int(bd_addr);
        auto it  = std::find_if(volume.begin(), volume.end(),
                                [&]( const std::pair<uint64_t, uint8_t> v )
                                {
                                    return v.first == addr_u64;
                                });

        return it != volume.end();
    }

};
