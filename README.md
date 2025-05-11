# SSDD_trabajo_final
Servicio Peer to Peer de compartición de archivos 

Para compilar todos los archivos .c del proyecto, incluyendo los archivos necesarios para la Parte 3 de RPCs 
únicamente es necesario usar el comando "make all" en el directorio raíz del proyecto.
```
make all
```
mencionamos también que el archivo Makefile espera que los archivos fuente .c del servidor estén en el directorio 
servidor y los archivos fuente .c del servidor rpc dentro del directorio rpc, por lo tanto si se cambian los archivos 
fuente de sus respectivos directorios habrá que modificar los comandos  específicos mediante SERVER_DIR y RPC_DIR.

A continuación, todos los comandos que se enseñan están pensados para ejecutarse desde el directorio raíz del 
proyecto, al igual que con el archivo Makefile.

Para ejecutar un cliente python:
```
python3 cliente/client.py -s <IP> -p <PORT>
```

Para ejecutar el servidor del sistema: 
```
./servidor/servidor -p <PORT>
```

Para ejecutar el servicio web desarrollado en la parte 2:
```
python3 servicio-web/servidor-web.py
```

Para ejecutar el servicio log basado en RPCs desarrollado en la parte 3:
```
sudo rpcbind
./rpc/servidor-rpc
```
