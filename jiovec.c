#include <sys/types.h>
#include <sys/param.h>
#include <sys/jail.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysctl.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <jail.h>

int
main()
{
	char jail_path[] = "/";
	char jail_name[] = "minimal";
	char jail_hostname[] = "minimal.example.org";
	char jail_ipv4[] = "127.0.13.41";
	char jail_ipv6[] = "fd::0d29";
	char jail_release[] = "13.2-RELEASE-p123";
	int jail_reldate = 1400123;
	int raw_sockets = 1;
	int persist = 1;
	int jid = 12345;

	char jail_errmsg[JAIL_ERRMSGLEN];
	jail_errmsg[0] = 0;

	// convert to binary form
	struct in_addr inet4_addr;
	int result = inet_pton(AF_INET, jail_ipv4, &inet4_addr);
	if (result <= 0) {
		if (result == 0)
			fprintf(stderr, "inet_pton: invalid IPv4 address\n");
		else
			perror("inet_pton");
		exit(EXIT_FAILURE);
	}

	struct in6_addr inet6_addr;
	result = inet_pton(AF_INET6, jail_ipv6, &inet6_addr);
	if (result <= 0) {
		if (result == 0)
			fprintf(stderr, "inet_pton: invalid IPv6 address\n");
		else
			perror("inet_pton");
		exit(EXIT_FAILURE);
	}

	struct iovec iov[] = {
		{ .iov_base = "path"         , .iov_len = sizeof("path")          },
		{ .iov_base = jail_path      , .iov_len = sizeof(jail_path)       },

		{ .iov_base = "name"         , .iov_len = sizeof("name")          },
		{ .iov_base = jail_name      , .iov_len = sizeof(jail_name)       },

		{ .iov_base = "host.hostname", .iov_len = sizeof("host.hostname") },
		{ .iov_base = jail_hostname  , .iov_len = sizeof(jail_hostname)   },

		{ .iov_base = "ip4.addr"     , .iov_len = sizeof("ip4.addr")      },
		{ .iov_base = &inet4_addr    , .iov_len = sizeof(inet4_addr)      },

		{ .iov_base = "ip6.addr"     , .iov_len = sizeof("ip6.addr")      },
		{ .iov_base = &inet6_addr    , .iov_len = sizeof(inet6_addr)      },

		{ .iov_base = "allow.raw_sockets", .iov_len = sizeof("allow.raw_sockets") },
		{ .iov_base = &raw_sockets,  .iov_len = sizeof(raw_sockets)       },

		{ .iov_base = "persist"      , .iov_len = sizeof("persist")       },
		{ .iov_base = &persist       , .iov_len = sizeof(persist)         },

		{ .iov_base = "jid"          , .iov_len = sizeof("jid")           },
		{ .iov_base = &jid           , .iov_len = sizeof(jid)             },

		{ .iov_base = "osrelease"    , .iov_len = sizeof("osrelease")     },
		{ .iov_base = jail_release   , .iov_len = sizeof(jail_release)    },

		{ .iov_base = "osreldate"    , .iov_len = sizeof("osreldate")     },
		{ .iov_base = &jail_reldate  , .iov_len = sizeof(jail_reldate)    },

		{ .iov_base = "errmsg"       , .iov_len = sizeof("errmsg")        },
		{ .iov_base = jail_errmsg    , .iov_len = JAIL_ERRMSGLEN          }
	};

	jid = jail_set(iov, sizeof(iov) / sizeof(*iov), JAIL_CREATE);

	if (jid < 0) {
		/* provide additional information via iovec errmsg */
		if (jail_errmsg[0]) {
			err(1, "jail_set: err: %s", strerror(errno));
		} else {
			err(1, "jail_set: msg: %s", jail_errmsg);
		}
		printf("FAILED\n");
		return EXIT_FAILURE;
	}

	/* from outside the jail, we can use jail_getid(), inside it will fail */
	jid = jail_getid(jail_name);

	if (jid > 0) {
		printf("OUTSIDE jid=%d.\n", jid);
	} else {
		printf("INSIDE  jid=%d.\n", jid);
		return EXIT_FAILURE;
	}

	/* attach and check jail_getid() again */
	if (jail_attach(jid) < 0) {
		err(1, "jail_attach");
	}

	jid = jail_getid(jail_name);

	if (jid > 0) {
		printf("OUTSIDE jid=%d.\n", jid);
		return EXIT_FAILURE;
	} else {
		printf("INSIDE  jid=%d.\n", jid);
	}

	printf("JAILED  pinging.\n");

	execlp("/sbin/ping", "ping", "-c", "5", "localhost", NULL);
	err(1, "execlp()");

	return EXIT_SUCCESS;
}
