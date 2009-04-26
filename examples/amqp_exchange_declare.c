#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp.h>
#include <amqp_framing.h>

#include <unistd.h>

#include "example_utils.h"

int main(int argc, char const * const *argv) {
  char const *hostname;
  int port;
  char const *exchange;
  char const *exchangetype;

  int sockfd;
  amqp_connection_state_t conn;

  if (argc < 5) {
    fprintf(stderr, "Usage: amqp_exchange_declare host port exchange exchangetype\n");
    return 1;
  }

  hostname = argv[1];
  port = atoi(argv[2]);
  exchange = argv[3];
  exchangetype = argv[4];

  conn = amqp_new_connection();

  die_on_error(sockfd = amqp_open_socket(hostname, port), "Opening socket");
  amqp_set_sockfd(conn, sockfd);
  die_on_amqp_error(amqp_login(conn, "/", 131072, AMQP_SASL_METHOD_PLAIN, "guest", "guest"),
		    "Logging in");

  {
    amqp_exchange_declare_t s =
      (amqp_exchange_declare_t) {
        .ticket = 0,
	.exchange = amqp_cstring_bytes(exchange),
	.type = amqp_cstring_bytes(exchangetype),
	.passive = 0,
	.durable = 0,
	.auto_delete = 0,
	.internal = 0,
	.nowait = 0,
	.arguments = {.num_entries = 0, .entries = NULL}
      };
    die_on_amqp_error(amqp_simple_rpc(conn, 1, AMQP_EXCHANGE_DECLARE_METHOD,
				      AMQP_EXCHANGE_DECLARE_OK_METHOD, &s),
		      "Declaring exchange");
  }

  die_on_amqp_error(amqp_channel_close(conn, AMQP_REPLY_SUCCESS), "Closing channel");
  die_on_amqp_error(amqp_connection_close(conn, AMQP_REPLY_SUCCESS), "Closing connection");
  amqp_destroy_connection(conn);
  die_on_error(close(sockfd), "Closing socket");
  return 0;
}