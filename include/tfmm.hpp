/*
 * tfmm.hpp
 *
 *  Created on: Dec 7, 2017
 *      Author: nullifiedcat
 */

#pragma once

namespace tfmm
{
void StartQueue();
void StartQueueStandby();
void LeaveQueue();
void DisconnectAndAbandon();
void Abandon();
bool IsMMBanned();
int GetQueue();
} // namespace tfmm
