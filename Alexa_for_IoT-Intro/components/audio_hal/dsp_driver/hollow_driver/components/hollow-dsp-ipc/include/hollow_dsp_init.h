/*
 *      Copyright 2018, Espressif Systems (Shanghai) Pte Ltd.
 *  All rights regarding this code and its modifications reserved.
 *
 * This code contains confidential information of Espressif Systems
 * (Shanghai) Pte Ltd. No licenses or other rights express or implied,
 * by estoppel or otherwise are granted herein.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HOLLOW_DSP_INIT_H

#define HOLLOW_DSP_INIT_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define AUDIO_BUF_SIZE 4096

/*
 * Initialize the Custom DSP
 *
 * Typically, the initialisation will take a backup of the Queue handle.
 *
 * Later, whenever the DSP detects a Wake Word, it will send an event
 * to this queue as follows:
 *     struct dsp_event_data event_data = {
 *         .event = WW
 *     };
 *
 * On receiving this event, the va_dsp thread will later on query the
 * WW len by making a call to custom_dsp_get_ww_len() that should also
 * be implemented.
 */
void custom_dsp_init(QueueHandle_t queue);

/*
 * API to reset dsp
 */
void custom_dsp_reset();

/*
 * API to put dsp in low power mode
 * @NOTE: Implementation of this api is optional and doesnot affect the Alexa Functionality
 */
void custom_dsp_enter_low_power();

/*
 * Initilize acoustic echo cancellation on dsp
 */
void custom_dsp_aec_init();

/*
 * Put DSP in Mute state(Microphones to be shut off)
 */
void custom_dsp_mic_mute();

/*
 * Disable Mic Mute state, Microphones to be turned on
 */
void custom_dsp_mic_unmute();

/*
 * API to indicate dsp to start sending microphone data
 */
void custom_dsp_tap_to_talk();

/*
 * API to indicate dsp to start sending microphone data
 * @Note: This API is same as custom_dsp_tap_to_talk();
 */
void custom_dsp_start_capture();

/*
 * API to indicate dsp to stop sending microphone data
 */
void custom_dsp_stop_capture();

/*
 * API for dsp to send audio data, should return number of bytes read from the DSP audio buffer
 * @Note: Audio data format is 16KHz, 16BIT, MonO Channel. Little Endian
 */
int custom_dsp_stream_audio(uint8_t *buf, size_t max_len);

/*
 * API to get 'ALEXA' phrase length, returns phrase length in bytes
 */
size_t custom_dsp_get_ww_len();

#endif /* HOLLOW_DSP_INIT_H */
