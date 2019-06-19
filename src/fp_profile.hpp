/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#ifndef FP_PROFILE_H
#define FP_PROFILE_H

/**
 * @addtogroup LED_TYPES
 * @{
 */
#define NUM_PATTERNS 3		/**< Total number of blink patterns */

typedef enum
{
	STATE_SLOW_BLINK = 0,	/**< 500ms ON, 1000ms OFF */
	STATE_DOUBLE_BLINK,		/**< (200ms ON - 100ms OFF) x 2 - 1000ms OFF */
	STATE_FAST_BLINK,		/**< 200ms ON - 100ms OFF */
}blinkPatternType_t;

/* @} */ // End of group LED_TYPES


#endif /*FP_PROFILE_H*/
