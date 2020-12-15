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

#include <stdio.h>
#include <stdlib.h>
#include <hollow_dsp_init.h>

extern const uint8_t firmware_start[] asm("_binary_firmware_bin_start");
extern const uint8_t firmware_end[]   asm("_binary_firmware_bin_end");

void custom_dsp_reset()
{

}

size_t custom_dsp_get_ww_len()
{
    /* Some random number */
    return 256;
}

void custom_dsp_mic_mute()
{

}

void custom_dsp_mic_unmute()
{

}

void custom_dsp_tap_to_talk()
{

}

void custom_dsp_start_capture()
{

}

void custom_dsp_stop_capture()
{

}

int custom_dsp_stream_audio(uint8_t *buf, size_t max_len)
{
    return max_len;
}

void custom_dsp_init(QueueHandle_t queue)
{

}

void custom_dsp_aec_init()
{

}

void custom_dsp_enter_low_power()
{

}
