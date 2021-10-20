/**
* Programa que envia a traves de un socket de Flujos de datos un mensaje a un servidor.
* Redes de Computadoras I
* Universidad Industrial de Santander
* ENTRADA: IP o nombre del servidor
* SALIDA: NA
* PROCESO: Este programa crea un socket y recibe un mensaje de bienvenida del servidor,
* envia un mensaje de texto a un servidor y finalmente recibe un mensaje de recibido
**/


#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>        
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// PUERTO DEL SERVIDOR
#define PORT 5020

// EL TAMANO MAXIMO DE DATOS
#define MAXDATASIZE 1024


int main(int argc, char *argv[]){

	// DESCRIPTOR PARA LA GESTION DEL SOCKET
	int fd;
	int numbytes;    
	FILE* fichero;    

	// BUFFER PARA ALMACENAR LOS DATOS
	char buf[MAXDATASIZE];  

	// ESTRUCTURA PARA ALMACENAR LA INFORMACION
	// DEL SERVIDOR
	struct hostent *he;         

	// DECLARACION DEL SOCKET DEL SERVIDOR
	struct sockaddr_in server;
  
	char band;

	// EL MENSAJE QUE SE ENVIARA AL SERVIDOR
	char message[1024]="HOLA MUNDO";

	fichero=fopen("mauriciotriplehpta.html","wt");
	bzero(buf,sizeof(buf));
	// VALIDACION DE LOS PARAMETROS DE ENTRADA
	if (argc !=3) { 
		printf("Uso: %s <Direccion IP o Nombre> <nombre archivo a pedir>\n",argv[0]);
		exit(-1);
	}


	// OBTENER EL NOMBRE DEL SERVIDOR O LA IP
	if ((he=gethostbyname(argv[1]))==NULL){       
		printf("gethostbyname() error\n");
		exit(-1);
	}

	// CREACION DE UN EXTREMO DE LA COMUNICACION
	if ((fd=socket(AF_INET, SOCK_STREAM, 0))==-1){
		printf("socket() error\n");
		exit(-1);
	}

// ESPECIFICACION DE VALORES AL SOCKET DEL SERVIDOR
//
	// SE ESPECIFICA QUE SE UTILIZARA IPv4
	server.sin_family = AF_INET;

	// SE ESPECIFICA EL PUERTO DEL SOCKET
	server.sin_port = htons(PORT); 

	// SE ESPECIFICA LA IP O NOMBRE DEL SERVIDOR QUE VIENE
	// DE LA FUNCION gethostbyname
	server.sin_addr = *((struct in_addr *)he->h_addr);  

	// SE LLENA CON CEROS EL RESTO DE LOS CAMPOS DEL SOCKET
	bzero(&(server.sin_zero),8);

	// SE REALIZA LA CONEXION AL SERVIDOR
	if(connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr))==-1){ 
		printf("connect() error\n");
		exit(-1);
	}

	// SE ENVIA EL MENSAJE
	send(fd,argv[2],strlen(argv[2]),0);
	
	if ((numbytes=recv(fd,buf,MAXDATASIZE,0)) == -1){  
		printf("Error en recv() \n");
		exit(-1);
	}

	// SE AGREGA UN FIN DE CADENA AL BUFFER DE RECEPCION
	buf[numbytes]='\0';

	// SE HACE UN LAZO PARA RECIBIR LINEA POR LINEA
		// EL ARCHIVO
		while(read(fd,buf,1024) > 0){
			// SE MUESTRA EL CONTENIDO DEL BUFFER
			// ES DECIR, UNA LINEA DEL ARCHIVO TEXTO
			printf("%s ", buf);
			fprintf(fichero,"%s\n",buf);
		}
		fclose(fichero);
	// SE CIERRA LA CONEXION CON EL SERVIDOR
	close(fd);      
  
}
