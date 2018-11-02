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
#ifndef LEDMGR_TYPES_H
#define LEDMGR_TYPES_H
#include <stdio.h>

#define LOG(level, text, ...) do {\
	printf("%s[%d] - %s: " text, __FUNCTION__, __LINE__, level, ##__VA_ARGS__);}while(0);

#define ERROR(text, ...) do {\
	printf("%s[%d] - %s: " text, __FUNCTION__, __LINE__, "ERROR", ##__VA_ARGS__);}while(0);
#define INFO(text, ...) do {\
	printf("%s[%d] - %s: " text, __FUNCTION__, __LINE__, "INFO", ##__VA_ARGS__);}while(0);

#ifdef DEBUG
#define DEBUG(text, ...) do {\
	printf("%s[%d] - %s: " text, __FUNCTION__, __LINE__, "DEBUG", ##__VA_ARGS__);}while(0);
#else
#define DEBUG(text, ...)
#endif

#define REPORT_IF_UNEQUAL(lhs, rhs) do {\
	if((lhs) != (rhs)) LOG("ERROR", "Unexpected error!\n");}while(0);

typedef enum
{
	STATE_STEADY_ON = 0,
	STATE_STEADY_OFF,
	STATE_BLINKING,
	STATE_UNKNOWN,
}indicatorState_t;

typedef struct
{
	unsigned int length; //milliseconds
	bool isOn; 
}blinkOp_t;

typedef struct
{
	unsigned int id;
	unsigned char num_sequences;
	blinkOp_t * sequence; /*Array of {duration, intensity} values in a defined sequence*/
}blinkPattern_t;

#endif /*LEDMGR_TYPES_H*/
