#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

void parse(char *a,char b[20][10])
{
	int i=0,j=0,k=0;
	while(a[i]!=0)
	{
		if(a[i]!=' ')
		{
			b[j][k]='\0';
			j++;
			k=0;
		}
		else
		{
			b[j][k]=a[i];
			k++;
		}
		i++;
	}
	b[j][k]='\0';
}

int changedir(char* pwd,char *a)
{
	int i=0,pos=0,prepos=0;
	if(strcmp(a,"..")==0)
	{
		while(pwd[i]!=0)
		{
			if(pwd[i]=='/')
			{
				prepos=pos;
				pos=i;
			}
			i++;
		}
		if(pos==prepos)
			return 1;
		pwd[prepos+1]=0;
		return 0;
	}
	else if(strcmp(a,".")==0)
	{
		return 0;
	}
	else if(a[0]=='/')
	{
		strcpy(pwd,a);
	}
	else
	{	
		strcat(pwd,a);
	}
	i=strlen(pwd);
	if(pwd[i-1]!='/')
	{
		pwd[i]='/';
		pwd[i+1]='\0';
	}
	return 0;
}

int listento(int port)
{
	int sockid;
	struct sockaddr_in serv_addr;
	if((sockid=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		printf("ERROR creating socket\n");
		return;
	}
	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(port);

	if(bind(sockid,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
		printf("Error Binding\n");
	listen(sockid,5);
	return sockid;
}

int main(int argc,char* argv[])
{
	DIR *dir;
	struct dirent *ds;
	int sockid,tsock,port,clilen,clilen2,n,newsock,f,tport;
	char buffer[512],filename[256],tosend[256],command[256],b[20][10]={10},pwd[30]="/",usrdir[256]="home",path[256];
	struct sockaddr_in cli_addr,cli_addr2;

	int fp;
	int tmp;
	
	mkdir("home",0777);
	printf("Enter the port number \n");
	scanf("%d",&port);
	gets(buffer);
	sockid=listento(port);
	clilen=sizeof(cli_addr);
	newsock=accept(sockid,(struct sockaddr *)&cli_addr,&clilen);
	
	if(newsock<0)
		printf("Enter in accept\n");
	while(1)
	{
		bzero(buffer,256);
		if(n=read(newsock,buffer,255))
		{
			if(strcmp(buffer,"exit")==0)
			{
				printf("Client has exited\n");	
				break;
			}
		}
		parse(buffer,b);
		bzero(tosend,256);
		
		if(strcmp(b[0],"pwd")==0)
		{
			sprintf(tosend,"%s \n",pwd);
			write(newsock,tosend,strlen(tosend));
			close(tmp);
		}
		else if(strcmp(b[0],"cd")==0)
		{
			if(changedir(pwd,b[1])==0)
				sprintf(tosend,"Directory changed to %s \n",b[1]);
			else
				sprintf(tosend,"Error changing directory \n");

			write(newsock,tosend,strlen(tosend));
		}
		else if(strcmp(b[0],"ls")==0)
		{
			strcpy(path,usrdir);
			strcat(path,pwd);
			dir=opendir(path);
			strcpy(tosend," ");
			write(newsock,tosend,strlen(tosend));

			while(ds=readdir(dir))
			{
				if(ds->d_name[0]=='.')
					continue;
				strcat(tosend,ds->d_name);
				n=strlen(tosend);
				if(ds->d_type==4)
				{
					strcat(tosend,"\t\t");
					strcat(tosend,"<DIR>");
				}
				strcat(tosend,"\n");
			}
			closedir(dir);
			strcat(tosend,"\n");
			write(newsock,tosend,strlen(tosend));
		}
		else if(strcmp(b[0],"get")==0)
		{
			strcpy(filename,usrdir);
			strcat(filename,pwd);
			strcat(filename,b[1]);
			tport=rand()%10000+1024;
			sprintf(tosend,"%d \n",tport);
			write(newsock,tosend,strlen(tosend));
			tsock=listento(tport);
			clilen2=sizeof(cli_addr2);
			tsock=accept(tsock,(struct sockaddr *)&cli_addr2,&clilen2);

			fp=open(filename,O_RDONLY);
			while((f=read(fp,buffer,4))>0)
				write(tsock,buffer,f);
		
			close(fp);
			close(tsock);
			shutdown(tsock,SHUT_RDWR);
			strcpy(tosend,"File copied\n");
			write(newsock,tosend,strlen(tosend));
		}
		else if(strcmp(b[0],"put")==0)
		{
			tport=rand()%10000+1024;
			sprintf(tosend,"%d\n",tport);
			strcpy(filename,usrdir);
			strcat(filename,pwd);
			strcat(filename,b[1]);
			
			fp=open(filename,O_CREAT|O_WRONLY);
			
			write(newsock,tosend,strlen(tosend));
			printf("%s \n",filename);
			tsock=listento(tport);
			clilen2=sizeof(cli_addr2);
			tsock=accept(tsock,(struct sockaddr *)&cli_addr2,&clilen2);

			while((f=read(tsock,buffer,4))>0)
				write(fp,buffer,f);
			fchmod(fp,0777);
			close(fp);

			close(tsock);
			shutdown(tsock,SHUT_RDWR);
			strcpy(tosend,"File copied\n");
			write(newsock,tosend,strlen(tosend));
		}
		else if(strcmp(b[0],"mkdir")==0)
		{
			strcpy(path,usrdir);
			strcat(path,pwd);
			strcat(path,b[1]);
			mkdir(path,0777);
			strcpy(tosend,"DOrectory created\n");
			write(newsock,tosend,strlen(tosend));
		}
		else
		{
			strcpy(tosend,"command not recognized\n");
			write(newsock,tosend,strlen(tosend));
		}
	}
	close(newsock);
	close(sockid);
	shutdown(sockid,SHUT_RDWR);
}
