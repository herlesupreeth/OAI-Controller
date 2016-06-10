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

/*! \file controller_agent.h
 * \brief top level enb agent receive thread and itti task
 * \author Navid Nikaein and Xenofon Foukas
 * \date 2016
 * \version 0.1
 */

#include "controller_agent_common.h"
#include "log.h"
#include "controller_agent.h"
#include "controller_agent_mac_defs.h"
#include "controller_agent_mac.h"

#include "controller_agent_extern.h"

#include "assertions.h"

#include "controller_agent_net_comm.h"
#include "controller_agent_async.h"

//#define TEST_TIMER

controller_agent_instance_t controller_agent[NUM_MAX_CONTROLLER];

char in_ip[40];
static uint16_t in_port;

void *send_thread(void *args);
void *receive_thread(void *args);
pthread_t new_thread(void *(*f)(void *), void *b);
// Protocol__ProgranMessage *controller_agent_timeout(void* args);

/* 
 * enb agent task mainly wakes up the tx thread for periodic and oneshot messages to the controller 
 * and can interact with other itti tasks
*/
// void *controller_agent_task(void *args){

//   controller_agent_instance_t         *d = (controller_agent_instance_t *) args;
//   Protocol__ProgranMessage *msg;
//   void *data;
//   int size;
//   err_code_t err_code;
//   int                   priority;

//   MessageDef                     *msg_p           = NULL;
//   const char                     *msg_name        = NULL;
//   instance_t                      instance;
//   int                             result;


//   itti_mark_task_ready(TASK_CONTROLLER_AGENT);

//   do {
//     // Wait for a message
//     itti_receive_msg (TASK_CONTROLLER_AGENT, &msg_p);
//     DevAssert(msg_p != NULL);
//     msg_name = ITTI_MSG_NAME (msg_p);
//     instance = ITTI_MSG_INSTANCE (msg_p);

//     switch (ITTI_MSG_ID(msg_p)) {
//     case TERMINATE_MESSAGE:
//       itti_exit_task ();
//       break;

//     case MESSAGE_TEST:
//       LOG_I(CONTROLLER_AGENT, "Received %s\n", ITTI_MSG_NAME(msg_p));
//       break;
    
//     case TIMER_HAS_EXPIRED:
//       msg = controller_agent_process_timeout(msg_p->ittiMsg.timer_has_expired.timer_id, msg_p->ittiMsg.timer_has_expired.arg);
//       if (msg != NULL){
// 	data=controller_agent_pack_message(msg,&size);
// 	if (controller_agent_msg_send(d->ctrl_id, CONTROLLER_AGENT_DEFAULT, data, size, priority)) {
// 	  err_code = PROTOCOL__PROGRAN_ERR__MSG_ENQUEUING;
// 	  goto error;
// 	}

// 	LOG_D(CONTROLLER_AGENT,"sent message with size %d\n", size);
//       }
//       break;

//     default:
//       LOG_E(CONTROLLER_AGENT, "Received unexpected message %s\n", msg_name);
//       break;
//     }

//     result = itti_free (ITTI_MSG_ORIGIN_ID(msg_p), msg_p);
//     AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
//     continue;
//   error:
//     LOG_E(CONTROLLER_AGENT,"controller_agent_task: error %d occured\n",err_code);
//   } while (1);

//   return NULL;
// }

// void *receive_thread(void *args) {

//   controller_agent_instance_t         *d = args;
//   void                  *data;
//   int                   size;
//   int                   priority;
//   err_code_t             err_code;

//   Protocol__ProgranMessage *msg;
  
//   while (1) {
//     // Check this part - CONTROLLER_AGENT_DEFAULT should not be used , rather CONTROLLER_AGENT_MAC may be used instead
//     if (controller_agent_msg_recv(d->ctrl_id, CONTROLLER_AGENT_DEFAULT, &data, &size, &priority)) {
//       err_code = PROTOCOL__PROGRAN_ERR__MSG_DEQUEUING;
//       goto error;
//     }

//     // LOG_D(CONTROLLER_AGENT,"received message with size %d\n", size);
//     LOG_I(CONTROLLER_AGENT,"received message with size %d\n", size);
  
    
//     msg=controller_agent_handle_message(d->ctrl_id, data, size);

//     free(data);
    
//     // check if there is something to send back to the controller
//     if (msg != NULL){
//       data=controller_agent_pack_message(msg,&size);
     
//       if (controller_agent_msg_send(d->ctrl_id, CONTROLLER_AGENT_DEFAULT, data, size, priority)) {
// 	err_code = PROTOCOL__PROGRAN_ERR__MSG_ENQUEUING;
// 	goto error;
//       }
      
//       // LOG_D(CONTROLLER_AGENT,"sent message with size %d\n", size);
//       LOG_I(CONTROLLER_AGENT,"sent message with size %d\n", size);
//     }
    
//   }

//   return NULL;

// error:
//   LOG_E(CONTROLLER_AGENT,"receive_thread: error %d occured\n",err_code);
//   return NULL;
// }

void *send_thread(void *args) {

  controller_agent_instance_t         *d = args;
  void                  *data, *packed_request_buffer;
  int                   size, packed_size;
  int                   priority;
  err_code_t             err_code;
  Protocol__ProgranMessage *msg;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  xid_t xid = 1;
  priority = 200;
  mid_t mod_id = d->ctrl_id;
  Protocol__ProgranMessage *req_msg;
  // stats_request_config_s request_config = (stats_request_config_s *) malloc(sizeof(stats_request_config_s));
  stats_request_config_t request_config;
  // report_config_t report_config = (report_config_t *) malloc(sizeof(report_config_t));
  report_config_t report_config;

  // Form the message here and use controller_agent_msg_send() function

  request_config.report_type = PROTOCOL__PRP_STATS_TYPE__PRST_COMPLETE_STATS;
  request_config.report_frequency = PROTOCOL__PRP_STATS_REPORT_FREQ__PRSRF_CONTINUOUS;

  //Set the number of UEs and create list with their RNTIs stats configs
  report_config.nr_ue = 1;    
  // report_config.ue_report_type = (ue_report_type_t *) malloc(sizeof(ue_report_type_t) * report_config.nr_ue);
  report_config.ue_report_type = (ue_report_type_t *) malloc(sizeof(ue_report_type_t));
  if (report_config.ue_report_type == NULL) {
    printf("Error in creating report_config message");
  }

  // report_config.ue_report_type[0].ue_rnti = 0;
  report_config.ue_report_type[0].ue_report_flags = PROTOCOL__PRP_UE_STATS_TYPE__PRUST_DL_CQI;

  //Set the number of CCs and create a list with the cell stats configs
  report_config.nr_cc = 1;
  // report_config.cc_report_type = (cc_report_type_t *) malloc(sizeof(cc_report_type_t) * report_config.nr_cc);
  report_config.cc_report_type = (cc_report_type_t *) malloc(sizeof(cc_report_type_t));
  if (report_config.cc_report_type == NULL) {
    printf("Error in creating report_config message");
  }
  // report_config.cc_report_type[0].cc_id = 0;
  report_config.cc_report_type[0].cc_report_flags = PROTOCOL__PRP_CELL_STATS_TYPE__PRCST_NOISE_INTERFERENCE;
  
  request_config.period = 10000;
  request_config.config = &report_config;
  
  controller_agent_mac_stats_request(mod_id, xid, &request_config, &req_msg);

  packed_request_buffer = controller_agent_pack_message(req_msg,&packed_size);

  if (req_msg != NULL){
    if (controller_agent_msg_send(d->ctrl_id, CONTROLLER_AGENT_DEFAULT, packed_request_buffer, packed_size, priority)) {
      err_code = PROTOCOL__PROGRAN_ERR__MSG_ENQUEUING;
      goto error;
    }
    
    // LOG_D(CONTROLLER_AGENT,"sent message with size %d\n", size);
    printf("sent message with size %d\n", packed_size);
  }

  // free(packed_request_buffer);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  while (1) {
    if (controller_agent_msg_recv(d->ctrl_id, CONTROLLER_AGENT_DEFAULT, &data, &size, &priority)) {
      err_code = PROTOCOL__PROGRAN_ERR__MSG_DEQUEUING;
      goto error;
    }

    // LOG_D(CONTROLLER_AGENT,"received message with size %d\n", size);
    printf("received message with size %d\n", size);
  
    // Here upon receiving data handle it to print to the console (look at controller_agent_mac_stats_reply() for field that you get)
    msg=controller_agent_handle_message(d->ctrl_id, data, size);
    free(data);
    // if(controller_agent_mac_destroy_stats_reply((Protocol__ProgranMessage *) data) < 0){
    //   printf("Error occured in destriying reply message\n");
    // }
  }

  // free(data);
  // if(controller_agent_mac_destroy_stats_reply((Protocol__ProgranMessage *) data) < 0){
  //   printf("Error occured in destriying reply message\n");
  // }
  controller_agent_mac_destroy_stats_request(req_msg);
  free(packed_request_buffer);
  free(report_config.ue_report_type);
  free(report_config.cc_report_type);
  pthread_exit(NULL);
  return NULL;

error:
  // LOG_E(CONTROLLER_AGENT,"receive_thread: error %d occured\n",err_code);
  printf("receive_thread: error %d occured\n",err_code);
  return NULL;
}


/* utility function to create a thread */
pthread_t new_thread(void *(*f)(void *), void *b) {
  pthread_t t;
  pthread_attr_t att;

  if (pthread_attr_init(&att)){ 
    fprintf(stderr, "pthread_attr_init err\n"); 
    exit(1); 
  }
  if (pthread_attr_setdetachstate(&att, PTHREAD_CREATE_DETACHED)) { 
    fprintf(stderr, "pthread_attr_setdetachstate err\n"); 
    exit(1); 
  }
  if (pthread_create(&t, &att, f, b)) { 
    fprintf(stderr, "pthread_create err\n"); 
    exit(1); 
  }
  if (pthread_attr_destroy(&att)) { 
    fprintf(stderr, "pthread_attr_destroy err\n"); 
    exit(1); 
  }

  return t;
}

int controller_agent_start(mid_t mod_id){
  
  int channel_id;

  controller_agent[mod_id].ctrl_id = mod_id;

  strcpy(in_ip, DEFAULT_CONTROLLER_AGENT_IPv4_ADDRESS );
  in_port = DEFAULT_CONTROLLER_AGENT_PORT;

  LOG_I(CONTROLLER_AGENT,"starting enb agent client for module id %d on ipv4 %s, port %d\n",controller_agent[mod_id].ctrl_id,in_ip,in_port);

  /*
   * Initialize the channel container
   */
  controller_agent_init_channel_container();

  /*Create the async channel info*/
  controller_agent_instance_t *channel_info = controller_agent_async_channel_info(mod_id, in_ip, in_port);

  /*Create a channel using the async channel info*/
  channel_id = controller_agent_create_channel((void *) channel_info, 
					controller_agent_async_msg_send, 
					controller_agent_async_msg_recv,
					controller_agent_async_release);

  
  if (channel_id <= 0) {
    goto error;
  }

  controller_agent_channel_t *channel = get_channel(channel_id);
  
  if (channel == NULL) {
    goto error;
  }


  // Everything correct till here
  /*Register the channel for all underlying agents (use CONTROLLER_AGENT_MAX)*/
  controller_agent_register_channel(mod_id, channel, CONTROLLER_AGENT_MAX);

  /*Example of registration for a specific agent(MAC):
   *controller_agent_register_channel(mod_id, channel, CONTROLLER_AGENT_MAC);
   */

  /*Initialize the continuous MAC stats update mechanism*/
  //controller_agent_init_cont_mac_stats_update(mod_id); Really not needed in controller
  
  // Here it is expects some kind of message from controller e.g: request for MAC stats
  // new_thread(send_thread, &controller_agent[mod_id]);
  pthread_t server_thread = (pthread_t) send_thread(&controller_agent[mod_id]);
  // pthread_join(server_thread, NULL);

  /*Initialize and register the mac xface. Must be modified later
   *for more flexibility in agent management */

  // AGENT_MAC_xface *mac_agent_xface = (AGENT_MAC_xface *) malloc(sizeof(AGENT_MAC_xface));
  // controller_agent_register_mac_xface(mod_id, mac_agent_xface); // Not needed at controller
  
  /* 
   * initilize a timer 
   */ 
  
  // controller_agent_init_timer(); // Not needed at controller
  
  /* 
   * start the enb agent task for tx and interaction with the underlying network function
   */ 
  
  /*if (itti_create_task (TASK_CONTROLLER_AGENT, controller_agent_task, (void *) &controller_agent[mod_id]) < 0) {
    LOG_E(CONTROLLER_AGENT, "Create task for eNB Agent failed\n");
    return -1;
  } 
*/
  printf("server ends\n");
  // LOG_I(CONTROLLER_AGENT,"server ends\n");
  return 0;

error:
  LOG_I(CONTROLLER_AGENT,"there was an error\n");
  return 1;

}



 // int controller_agent_stop(mid_t mod_id){ 
  
 //   int i=0; 

 //   controller_agent_destroy_timers(); 
 //   for ( i =0; i < controller_agent_info.nb_modules; i++) { 
  
 //     destroy_link_manager(controller_agent[i].manager); 
  
 //     destroy_message_queue(controller_agent[i].send_queue); 
 //     destroy_message_queue(controller_agent[i].receive_queue); 
  
 //     close_link(controller_agent[i].link); 
 //   } 
 // } 



// Protocol__ProgranMessage *controller_agent_timeout(void* args){

//   //  controller_agent_timer_args_t *timer_args = calloc(1, sizeof(*timer_args));
//   //memcpy (timer_args, args, sizeof(*timer_args));
//   controller_agent_timer_args_t *timer_args = (controller_agent_timer_args_t *) args;
  
//   LOG_I(CONTROLLER_AGENT, "controller_agent %d timeout\n", timer_args->mod_id);
//   //LOG_I(CONTROLLER_AGENT, "eNB action %d ENB flags %d \n", timer_args->cc_actions,timer_args->cc_report_flags);
//   //LOG_I(CONTROLLER_AGENT, "UE action %d UE flags %d \n", timer_args->ue_actions,timer_args->ue_report_flags);
  
//   return NULL;
// }
