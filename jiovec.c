#include <sys/types.h>
#include <sys/param.h>
#include <sys/jail.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysctl.h>

#include <netinet/in.h>
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

	char jail_errmsg[JAIL_ERRMSGLEN];
	jail_errmsg[0] = 0;

	struct iovec iov[] = {
		{ .iov_base = "path"         , .iov_len = sizeof("path")          },
		{ .iov_base = jail_path      , .iov_len = sizeof(jail_path)       },

		{ .iov_base = "name"         , .iov_len = sizeof("name")          },
		{ .iov_base = jail_name      , .iov_len = sizeof(jail_name)       },

		{ .iov_base = "errmsg"       , .iov_len = sizeof("errmsg")        },
		{ .iov_base = jail_errmsg    , .iov_len = JAIL_ERRMSGLEN          }
	};

	int jid = jail_set(iov, sizeof(iov) / sizeof(*iov), JAIL_CREATE | JAIL_ATTACH);

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

	printf("JAILED jid=%d is sleeping.\n", jid);

	execlp("/bin/sleep", "sleep", "5", NULL);
	err(1, "execlp()");
	return EXIT_SUCCESS;
}