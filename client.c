#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
void error(const char *msg)
{
    perror(msg);
    printf("closing connection with the server\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n,c=0,temp,subject_no,choice,count_login,count_attempt;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    float percentage,marks,new_marks;
    char buffer[256],name[20],password[20];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);

    // create stream socket

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    //build server address structure

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    //connect to server

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    else{printf("connected to server successfully\n");}

    //Enter username and password

    bzero(buffer,256);
    bzero(name,20);
    bzero(password,20);
    count_login=1;
    do
    {
        printf("\nEnter Username : ");
        scanf("%s",name);
        n=write(sockfd,name,20);
        if (n < 0)
         error("ERROR writing to socket");
        printf("Enter password : ");
        scanf("%s",password);
        n=write(sockfd,password,20);
        if (n < 0)
         error("ERROR writing to socket");
        n=read(sockfd,&c,sizeof(int));
        if (n < 0)
         error("ERROR reading from socket");
        if(c==0)
            printf("\nWrong Username or Password:Attempts remaining=%d\n",5-count_login);
        count_login++;
        if(count_login==6)
            error("Too many wrong attempts");
    }while(c==0);


    //Menu for instructor

    if(c==1)
    {
        printf("\n*******Welcome Instructor*******\n");
        while(true)
        {
            printf("\nEnter 1 to get marks and percentage of students\n");
            printf("Enter 2 to get class average\n");
            printf("Enter 3 to get number of students failed in each subject\n");
            printf("Enter 4 to get name of best and worst performing student\n");
            printf("Enter 5 to update marks of a student\n");
            printf("Enter 6 to quit\n");
            scanf("%d",&choice);
            n=write(sockfd,&choice,sizeof(int));
            if (n < 0)
                error("ERROR writing to socket");
            switch(choice)
            {
                case 1:
                    for(int i=1;i<=20;i++)
                    {
                        n=read(sockfd,name,20);
                        if (n < 0)
                            error("ERROR reading from socket");
                        printf("\n%s",name);
                        printf("\n");
                        for(int j=1;j<=5;j++)
                        {
                            n=read(sockfd,&marks,sizeof(float));
                            if (n < 0)
                                error("ERROR reading from socket");
                            printf("Marks in subject%d=%0.2f",j,marks);
                            printf("\n");
                        }
                        n=read(sockfd,&percentage,sizeof(float));
                        if (n < 0)
                            error("ERROR reading from socket");
                        printf("Percentage=%0.2f",percentage);
                        printf("\n");
                    }
                    break;
                case 2:
                    n=read(sockfd,&percentage,sizeof(float));
                    if (n < 0)
                        error("ERROR reading from socket");
                    printf("Class average percentage=%0.2f\n",percentage);
                    break;
                case 3:
                    for(int i=1;i<=5;i++)
                    {
                        n=read(sockfd,&temp,sizeof(int));
                        if (n < 0)
                            error("ERROR reading from socket");
                        printf("Students failed in subject%d=%d",i+1,temp);
                        printf("\n");
                    }
                    break;
                case 4:
                    n=read(sockfd,&percentage,sizeof(float));
                    if (n < 0)
                        error("ERROR reading from socket");
                    n=read(sockfd,name,20);
                    if (n < 0)
                        error("ERROR reading from socket");
                    printf("Highest percentage=%0.2f scored by %s",percentage,name);
                    printf("\n");
                    n=read(sockfd,&percentage,sizeof(float));
                    if (n < 0)
                        error("ERROR reading from socket");
                    n=read(sockfd,name,20);
                    if (n < 0)
                        error("ERROR reading from socket");
                    printf("Lowest percentage=%0.2f scored by %s",percentage,name);
                    printf("\n");
                    break;
                case 5:
                    count_attempt=1;
                    do
                    {
                        printf("\nEnter name of student:");
                        scanf("%s",name);
                        n=write(sockfd,name,20);
                        if (n < 0)
                            error("ERROR writing to socket");
                        printf("Enter subject no. and new mark");
                        scanf("%d %f",&subject_no,&new_marks);
                        n=write(sockfd,&subject_no,sizeof(int));
                        if (n < 0)
                            error("ERROR writing to socket");
                        n=write(sockfd,&new_marks,sizeof(float));
                        if (n < 0)
                            error("ERROR writing to socket");
                        n=read(sockfd,&temp,sizeof(int));
                        if (n < 0)
                            error("ERROR reading from socket");
                        if(temp==0 || (subject_no<0 || subject_no>5) || (new_marks<0 || new_marks>100))
                            printf("\nWrong credentials entered:Attempts remaining=%d\n",5-count_attempt);
                        count_attempt++;
                        if(count_attempt==6)
                            error("Too many wrong attempts");
                    }while(temp==0 || (subject_no<0 || subject_no>5) || (new_marks<0 || new_marks>100));
                    printf("Mark of %s in subject%d updated\n",name,subject_no);
                    break;
                case 6:
                    error("Exiting Client");
                default:
                    bzero(buffer,256);
                    n=read(sockfd,buffer,255);
                    if (n < 0)
                        error("ERROR reading from socket");
                    printf("%s\n",buffer);
                    break;
            }
        }

    }

    //Menu for students

    else if(c==2)
    {
        printf("\n*******Welcome %s*******\n",name);
        while(true)
        {
            printf("\nEnter 1 to get your marks of each subject\n");
            printf("Enter 2 to get your percentage\n");
            printf("Enter 3 to get subjects with minimum and maximum marks\n");
            printf("Enter 4 to quit\n");
            scanf("%d",&choice);
            n=write(sockfd,&choice,sizeof(int));
            if (n < 0)
                error("ERROR writing to socket");
            switch(choice)
            {
                case 1:
                    for(int i=1;i<=5;i++)
                    {
                        n=read(sockfd,&marks,sizeof(float));
                        if (n < 0)
                            error("ERROR reading from socket");
                        printf("Mark in Subject%d=%0.2f",i,marks);
                        printf("\n");
                    }
                    break;
                case 2:
                    n=read(sockfd,&percentage,sizeof(float));
                    if (n < 0)
                        error("ERROR reading from socket");
                    printf("Your Percentage=%0.2f",percentage);
                    printf("\n");
                    break;
                case 3:
                    n=read(sockfd,&marks,sizeof(float));
                    if (n < 0)
                        error("ERROR reading from socket");
                    printf("Minimum marks=%0.2f in subject(s):",marks);
                    while(1)
                    {
                        n=read(sockfd,&temp,sizeof(int));
                        if (n < 0)
                            error("ERROR reading from socket");
                        if(temp==-1)
                            break;
                        printf("Subject%d ",temp+1);
                    }
                    printf("\n");
                    n=read(sockfd,&marks,sizeof(float));
                    if (n < 0)
                        error("ERROR reading from socket");
                    printf("Maximum marks=%0.2f in subject(s):",marks);
                    while(1)
                    {
                        n=read(sockfd,&temp,sizeof(int));
                        if (n < 0)
                            error("ERROR reading from socket");
                        if(temp==-1)
                            break;
                        printf("Subject%d ",temp+1);
                    }
                    printf("\n");
                    break;
                case 4:
                    error("Exiting client");
                default:
                    bzero(buffer,256);
                    n=read(sockfd,buffer,255);
                    if (n < 0)
                        error("ERROR reading from socket");
                    printf("%s",buffer);
                    printf("\n");
                    break;
            }
        }
    }
    return 0;
}
