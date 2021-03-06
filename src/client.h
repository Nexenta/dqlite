/* Core dqlite client logic for encoding requests and decoding responses. */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <raft.h>

#include <stdint.h>

#include "lib/buffer.h"

#include "tuple.h"

struct client
{
	int fd;		     /* Connected socket */
	unsigned db_id;      /* Database ID provided by the server */
	struct buffer read;  /* Read buffer */
	struct buffer write; /* Write buffer */
};

struct row
{
	struct value *values;
	struct row *next;
};

struct rows
{
	unsigned column_count;
	const char **column_names;
	struct row *next;
};

struct server
{
	int	id;
	char	addr[INET_ADDRSTRLEN + 6 + 1];
};

struct servers
{
	uint64_t	servers_nr;
	struct server	*nodes;
};

/* Initialize a new client, writing requests to fd. */
int clientInit(struct client *c, int fd);

/* Release all memory used by the client, and close the client socket. */
void clientClose(struct client *c);

/* Initialize the connection by writing the protocol version. This must be
 * called before using any other API. */
int clientSendHandshake(struct client *c);

/* Send a request to open a database */
int clientSendOpen(struct client *c, const char *name);

/* Receive the response to an open request. */
int clientRecvDb(struct client *c);

/* Send a request to prepare a statement. */
int clientSendPrepare(struct client *c, const char *sql);

/* Receive the response to a prepare request. */
int clientRecvStmt(struct client *c, unsigned *stmt_id);

/* Send a request to execute a statement. */
int clientSendExec(struct client *c, unsigned stmt_id);

/* Receive the response to an exec request. */
int clientRecvResult(struct client *c,
			unsigned *last_insert_id,
			unsigned *rows_affected);

/* Send a request to perform a query. */
int clientSendQuery(struct client *c, unsigned stmt_id);

/* Receive the response of a query request. */
int clientRecvRows(struct client *c, struct rows *rows);

/* Send a raft connect request. */
int clientSendConnect(struct client *c, raft_id id, const char *address);

/* Release all memory used in the given rows object. */
void clientCloseRows(struct rows *rows);

/* Send a request to add a dqlite node. */
int clientSendAdd(struct client *c, unsigned id, const char *address);

/* Send a request to assign a role to a node. */
int clientSendAssign(struct client *c, unsigned id, int role);

/* Send a request to remove a server from the cluster. */
int clientSendRemove(struct client *c, unsigned id);

/* Receive an empty response. */
int clientRecvEmpty(struct client *c);

/* Send a request to obtain a list of servers from the cluster */
int clientSendCluster(struct client *c);

/* Receive a list of servers from a cluster request */
int clientRecvServers(struct client *c, struct servers *servers);

/* Release all memory used in the given servers object. */
void clientCloseServers(struct servers *servers);

/* Send a request to check if it's connected to a leader */
int clientSendLeader(struct client *c);

/* Receive a server from a leader request */
int clientRecvServer(struct client *c, struct server *server);

#endif /* CLIENT_H_*/
