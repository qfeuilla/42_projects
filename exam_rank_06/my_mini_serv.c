#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>

int max_fd;
int actual_clientid;

typedef struct  s_client
{
    int fd;
    int id;
    struct s_client *next;
}               t_client;

void exit_msg(char *msg) {
    write(2, msg, strlen(msg));
    exit(1);
}

void exit_fatal() {
    exit_msg("Fatal error\n");
}

void init_set(int fd, fd_set *set_read, t_client *clients) {
    FD_ZERO(set_read);
    max_fd = fd;
    t_client *tmp = clients;
    for (; tmp; tmp=tmp->next) {
        FD_SET(tmp->fd, set_read);
        max_fd = max_fd < tmp->fd ? tmp->fd : max_fd;
    }
    FD_SET(fd, set_read);
}

void close_all_clients(t_client *clients) {
    t_client *tmp = clients;
    for (; tmp; tmp = tmp->next)
        close(tmp->fd);
}

int add_client(int connfd, t_client **clients, int servfd) {
    t_client *new;

    if (!(new = malloc(sizeof(t_client)))) {
        close_all_clients(*clients);
        close(connfd);
        close(servfd);
        exit_fatal();
    }
    new->fd = connfd;
    new->id = actual_clientid++;
    new->next = NULL;
    if (!(*clients))
        (*clients) = new;
    else {
        t_client *tmp = *clients;
        for (; tmp->next; tmp = tmp->next) ;
        tmp->next = new;
    }
    return new->id;
}

void send_all(t_client *clients, int fd, char *str) {
    t_client *tmp = clients;
    for (; tmp; tmp = tmp->next)
        if (tmp->fd != fd)
            send(tmp->fd, str, strlen(str), 0);
}

int remove_client(t_client **clients, int fd) {
    t_client *tmp = *clients;
    int id;

    if (tmp && tmp->fd == fd) {
        (*clients) = tmp->next;
        id = tmp->id;
        close(tmp->fd);
        free(tmp);
    } else if (tmp) {
        t_client *prev;
        for (; tmp && tmp->next && tmp->next->fd != fd; tmp = tmp->next) ;
        prev = tmp;
        tmp = tmp->next;
        prev->next = tmp->next;
        id = tmp->id;
        close(tmp->fd);
        free(tmp);
    }
    return (id);
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

int main(int ac, char **av) {
    int sockfd, connfd;
    unsigned int len;
    struct sockaddr_in servaddr, cli;
    char *buff, *str;

    int port;
    fd_set set_read;
    t_client *clients = NULL;
    int select_res, recv_res;
    int id;

    if (ac != 2) 
        exit_msg("Wrong number of arguments\n");
    
    if ((port = atoi(av[1])) <= 0)
        exit_fatal();
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit_fatal();
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(0x7f000001);
    servaddr.sin_port = htons(port);

    if ((bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
        close(sockfd);
        exit_fatal();
    }

    if (listen(sockfd, 10) != 0) {
        close(sockfd);
        exit_fatal();
    }

    len = sizeof(cli);
    actual_clientid = 0;
    while (1) {
        init_set(sockfd, &set_read, clients);
        select_res = select(max_fd + 1, &set_read, NULL, NULL, NULL);
        if (select_res > 0) {
            if (FD_ISSET(sockfd, &set_read)) {
                if ((connfd = accept(sockfd, (struct sockaddr *)&cli, &len)) >= 0) {
                    id = add_client(connfd, &clients, sockfd);
                    if (max_fd < connfd)
                        max_fd = connfd;
                    if (!(str = malloc(strlen("server: client  just arrived\n") + 24))) {
                        close_all_clients(clients);
                        close(sockfd);
                        exit_fatal();
                    }
                    sprintf(str, "server: client %d just arrived\n", id);
                    send_all(clients, connfd, str);
                    free(str);
                }
            } else {
                t_client *tmp = clients;
                while (tmp)
                {
                    connfd = tmp->fd;
                    id = tmp->id;
                    tmp = tmp->next;
                    if ((buff = malloc(4097)) == NULL) {
                        close_all_clients(clients);
                        close(sockfd);
                        exit_fatal();
                    }
                    if (FD_ISSET(connfd, &set_read)) {
                        bzero(buff, 4097);
                        recv_res = recv(connfd, buff, 4096, 0);
                        if (recv_res == 0) {
                            id = remove_client(&clients, connfd);
                            if (!(str = malloc(strlen("server: client  just left\n") + 24))) {
                                close_all_clients(clients);
                                close(sockfd);
                                exit_fatal();
                            }
                            sprintf(str, "server: client %d just left\n", id);
                            send_all(clients, connfd, str);
                            free(str);
                        } else if (recv_res > 0) {
                            if (!(str = malloc(4097 + 32))) {
                                close_all_clients(clients);
                                close(sockfd);
                                exit_fatal();
                            }
                            char *msg;
                            while (extract_message(&buff, &msg)) {
                                sprintf(str, "client %d: %s", id, msg);
                                free(msg);
                                send_all(clients, connfd, str);
                            }
                            free(str);
                        }
                    }
                    free(buff);
                }
            }
        }
    }
}