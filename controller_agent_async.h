/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@lists.eurecom.fr

  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.

 *******************************************************************************/

/*! \file controller_agent_async.h
 * \brief channel implementation for async interface
 * \author Xenofon Foukas
 * \date 2016
 * \version 0.1
 */

#ifndef CONTROLLER_AGENT_ASYNC_H_
#define CONTROLLER_AGENT_ASYNC_H_

#include "controller_agent_net_comm.h"

typedef struct {
  mid_t            ctrl_id;
  socket_link_t   *link;
  message_queue_t *send_queue;
  message_queue_t *receive_queue;
  link_manager_t  *manager;
} controller_agent_async_channel_t;

controller_agent_async_channel_t * controller_agent_async_channel_info(mid_t mod_id, char *dst_ip, uint16_t dst_port);

int controller_agent_async_msg_send(void *data, int size, int priority, void *channel_info);

int controller_agent_async_msg_recv(void **data, int *size, int *priority, void *channel_info);

void controller_agent_async_release(controller_agent_channel_t *channel);


#endif /*CONTROLLER_AGENT_ASYNC_H_*/
