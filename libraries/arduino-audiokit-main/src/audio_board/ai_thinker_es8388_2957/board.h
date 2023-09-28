/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _AUDIO_BOARD_H_
#define _AUDIO_BOARD_H_

#include "audio_hal/audio_hal.h"
#include "board_def.h"
#include "audio_hal/board_pins_config.h"//#include "esp_peripherals.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Audio board handle
 */
struct audio_board_handle {
    audio_hal_handle_t audio_hal; /*!< audio hardware abstract layer handle */
};

typedef struct audio_board_handle *audio_board_handle_t;

/**
 * @brief Initialize audio board
 *
 * @return The audio board handle
 */
audio_board_handle_t audio_board_init(void);

/**
 * @brief Initialize codec chip
 *
 * @return The audio hal handle
 */
audio_hal_handle_t audio_board_codec_init(void);


/**
 * @brief Query audio_board_handle
 *
 * @return The audio board handle
 */
audio_board_handle_t audio_board_get_handle(void);

/**
 * @brief Uninitialize the audio board
 *
 * @param audio_board The handle of audio board
 *
 * @return  0       success,
 *          others  fail
 */
esp_err_t audio_board_deinit(audio_board_handle_t audio_board);


/**
 * @brief Updates the headphone status - switches the PA power of when the headphone is connected
 * 
 */
void audio_board_update_headphone_status();

#ifdef __cplusplus
}
#endif

#endif
