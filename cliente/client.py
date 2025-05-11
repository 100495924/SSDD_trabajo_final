from enum import Enum
import argparse
import os
import socket
import sys
import threading
import zeep
from threading import Event
from threading import Thread

class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1


    _registered = []

    # se usa para saber el username activo
    # connect -> set to user (only if OK)
    # disconnect -> set to None (always)
    _active_user = None

    _database_listusers = []
    _stop_event = Event()
    _server_thread = None
    _server_thread_port = None

    # ******************** METHODS *******************

    # Funciones para escribir y leer en los sockets

    def read_string(sock):
        a = ''
        while True:
            msg = sock.recv(1)
            if (msg == b'\0'):
                break
            a += msg.decode()
        return a

    def read_number(sock):
        a = client.read_string(sock)
        return(int(a,10))

    def write_string(sock, str):
        sock.sendall(str.encode()) 
        sock.sendall(b'\0') 

    def write_number(sock, num):
        a = str(num)
        client.write_string(sock, a)
    
    # Función que ejecuta el segundo hilo que se crea al hacer CONNECT
    def socket_server(event):
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        ip_user = socket.gethostbyname(socket.gethostname())
        server_address = (ip_user, 0)
        sock.bind(server_address)

        client._server_thread_port = sock.getsockname()[1]
        sock.listen(5)

        while not event.is_set():
            try:
                connection, client_address = sock.accept()
                message = client.read_string(connection)
            except:
                client.write_number(connection, 2)
                connection.close()
                continue

            if message == "GET_FILE":
                # logica de mandar archivos
                try:
                    remote_filename = client.read_string(connection)
                    # El archivo local no puede ser un directorio y el camino hacia él por los directorios debe existir
                    # Permitimos que el archivo en sí no exista
                    if remote_filename[-1] != "/" and os.path.exists(os.path.dirname(remote_filename)):
                        remote_filesize = os.path.getsize(remote_filename)
                        file = open(remote_filename, "rb")
                    else:
                        raise FileNotFoundError
                    
                    # remote_filesize = os.path.getsize(remote_filename)
                except Exception as e:
                    if type(e) == FileNotFoundError:
                        client.write_number(connection, 1)
                    else:
                        client.write_number(connection, 2)
                    connection.close()
                else:
                    client.write_number(connection, 0)
                    data = file.read()
                    if data:
                        client.write_number(connection, remote_filesize)
                        connection.sendall(data)
                    else:
                        client.write_number(connection, 2)

                    file.close()
                    connection.close()
    
    # Función para el servicio web
    def get_date_hour():
        wsdl = 'http://localhost:30000/?wsdl'
        zepp_client = zeep.Client(wsdl=wsdl)
        return zepp_client.service.send_date_hour()


    @staticmethod
    def  register(user) :
        return_code = 2

        # Los inputs deben tener máximo 256 bytes (255 caracteres + "\0")
        if len(user) >= 256:
            print("REGISTER FAIL")
            return return_code
        
        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(server_address)
        except:
            print("REGISTER FAIL")
            return return_code

        try:

            # 3) write() -> enviar petición al servidor
            client.write_string(sock, "REGISTER")
            client.write_string(sock, user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                client._registered.append(user)
                print("REGISTER OK")
            elif return_code == 1:
                print("USERNAME IN USE")
            else:
                print("REGISTER FAIL")
        except:
            print("REGISTER FAIL")
        finally:
            # 5) close() -> cerrar sesión
            date_hour_str = client.get_date_hour()
            client.write_string(sock, date_hour_str)
            sock.close()

        return return_code

   
    @staticmethod
    def  unregister(user) :
        return_code = 2

        if client._active_user == user:
            code_disconnect = client.disconnect(user, False)
            if code_disconnect == 1:
                print("USER DOES NOT EXIST")
                return code_disconnect
            elif code_disconnect == 3:
                print("UNREGISTER FAIL")
                return code_disconnect
            # print("UNREGISTER FAIL")
            # return return_code
        # Los inputs deben tener máximo 256 bytes (255 caracteres + "\0")
        if len(user) >= 256:
            print("UNREGISTER FAIL")
            return return_code

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(server_address)
        except:
            print("UNREGISTER FAIL")
            return return_code

        try:
            # 3) write() -> enviar petición al servidor
            client.write_string(sock, "UNREGISTER")
            client.write_string(sock, user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                client._registered.remove(user)
                print("UNREGISTER OK")
            elif return_code == 1:
                print("USER DOES NOT EXIST")
            else:
                print("UNREGISTER FAIL")
        except:
            print("UNREGISTER FAIL")
        finally:
            # 5) close() -> cerrar sesión
            date_hour_str = client.get_date_hour()
            client.write_string(sock, date_hour_str)
            sock.close()

        return return_code


    @staticmethod
    def  connect(user) :
        return_code = 3

        # Si ya estás conectado a un usuario, no puedes conectarte otra vez (al mismo u otro usuario)
        if client._active_user != None:
            print("USER ALREADY CONNECTED")
            return_code = 2
            return return_code    
        # Los inputs deben tener máximo 256 bytes (255 caracteres + "\0")
        if len(user) >= 256:
            print("CONNECT FAIL")
            return return_code
        
        # EMPEZAR EJECUCIÓN DE HILO

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        ip_user = socket.gethostbyname(socket.gethostname())
        # port_user = sock.getsockname()[1]

        # CREAR SERVER THREAD

        client._server_thread_port = None
        client._stop_event.clear()
        client._server_thread = Thread(target=client.socket_server, args=(client._stop_event,), daemon=True)
        client._server_thread.start()

        while client._server_thread_port == None:
            pass

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(server_address)
        except:
            print("CONNECT FAIL")
            return return_code

        try:
            # 3) write() -> enviar petición al servidor
            client.write_string(sock, "CONNECT")
            client.write_string(sock, user)
            client.write_string(sock, ip_user)
            client.write_number(sock, client._server_thread_port)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                client._active_user = user
                print("CONNECT OK")
            elif return_code == 1:
                print("CONNECT FAIL, USER DOES NOT EXIST")
            elif return_code == 2:
                print("USER ALREADY CONNECTED")
            else:
                print("CONNECT FAIL")
        except:
            print("CONNECT FAIL")
        finally:
            # 5) close() -> cerrar sesión
            date_hour_str = client.get_date_hour()
            client.write_string(sock, date_hour_str)
            sock.close()

        return return_code


    @staticmethod
    def  disconnect(user, print_result) :
        return_code = 3

        # Los inputs deben tener máximo 256 bytes (255 caracteres + "\0")
        if len(user) >= 256:
            if print_result:
                print("DISCONNECT FAIL")
            return return_code

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(server_address)
        except:
            print("DISCONNECT FAIL")
            return return_code

        try:
            # 3) write() -> enviar petición al servidor
            
            # PARAR EJECUCIÓN DE SERVER THREAD (INCLUSO CON ERRORES)

            client.write_string(sock, "DISCONNECT")
            client.write_string(sock, user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if print_result:
                if return_code == 0:
                    print("DISCONNECT OK")
                elif return_code == 1:
                    print("DISCONNECT FAIL, USER DOES NOT EXIST")
                elif return_code == 2:
                    print("DISCONNECT FAIL, USER NOT CONNECTED")
                else:
                    print("DISCONNECT FAIL")
        except:
            if print_result:
                print("DISCONNECT FAIL")
        finally:
            # 5) close() -> cerrar sesión
            client._active_user = None

            # Recibir fecha y hora por servicio web y enviarlo al servidor
            date_hour_str = client.get_date_hour()
            client.write_string(sock, date_hour_str)

            # destruir thread para detectar get_file
            if (client._server_thread != None): 
                client._stop_event.set()
                client._server_thread = None
                client._server_thread_port = None

            sock.close()

        return return_code


    @staticmethod
    def  publish(fileName,  description) :
        return_code = 4

        # La operación solo se puede realizar si se esta registrado y conectado
        # Decidimos no mandar None por socket porque puede crear conflictos en el servidor
        if client._active_user == None:
            if client._active_user in client._registered:
                print("PUBLISH FAIL, USER NOT CONNECTED")
                return 2
            else:
                print("PUBLISH FAIL, USER DOES NOT EXIST")
                return 1
        # Los inputs deben tener máximo 256 bytes (255 caracteres + "\0")
        if len(fileName) >= 256 or len(description) >= 256:
            print("PUBLISH FAIL")
            return return_code

        # Se envia una cadena con el path absoluto del fichero (esta cadena no podra contener 
        # espacios en blanco). El tamaño maximo del path absoluto del fichero sera de 256bytes.

        # Se envia una cadena de caracteres con la descripcion del contenido. Esta cadena
        # podra contener espacios en blanco y su tamaño maximo sera de 256 bytes.

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(server_address)
        except:
            print("PUBLISH FAIL")
            return return_code

        try:
            # 3) write() -> enviar petición al servidor

            client.write_string(sock, "PUBLISH")
            client.write_string(sock, client._active_user)
            client.write_string(sock, fileName)
            client.write_number(sock, description)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("PUBLISH OK")
            elif return_code == 1:
                print("PUBLISH FAIL, USER DOES NOT EXIST")
            elif return_code == 2:
                print("PUBLISH FAIL, USER NOT CONNECTED")
            elif return_code == 3:
                print("PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
            else:
                print("PUBLISH FAIL")
        except:
            print("PUBLISH FAIL")
        finally:
            # Recibir fecha y hora por servicio web y enviarlo al servidor
            date_hour_str = client.get_date_hour()
            client.write_string(sock, date_hour_str)
        
            # 5) close() -> cerrar sesión
            sock.close()

        return return_code


    @staticmethod
    def  delete(fileName) :
        return_code = 4

        # La operación solo se puede realizar si se esta registrado y conectado
        # Decidimos no mandar None por socket porque puede crear conflictos en el servidor
        if client._active_user == None:
            if client._active_user in client._registered:
                print("DELETE FAIL, USER NOT CONNECTED")
                return 2
            else:
                print("DELETE FAIL, USER DOES NOT EXIST")
                return 1
        # Los inputs deben tener máximo 256 bytes (255 caracteres + "\0")
        if len(fileName) >= 256:
            print("DELETE FAIL")
            return return_code

        # Se envia una cadena con el path absoluto del fichero (esta cadena no podra contener 
        # espacios en blanco). El tamaño maximo del path absoluto del fichero sera de 256bytes.

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(server_address)
        except:
            print("DELETE FAIL")
            return return_code


        try:
            # 3) write() -> enviar petición al servidor
        
            client.write_string(sock, "DELETE")
            client.write_string(sock, client._active_user)
            client.write_string(sock, fileName)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("DELETE OK")
            elif return_code == 1:
                print("DELETE FAIL, USER DOES NOT EXIST")
            elif return_code == 2:
                print("DELETE FAIL, USER NOT CONNECTED")
            elif return_code == 3:
                print("DELETE FAIL, CONTENT NOT PUBLISHED")
            else:
                print("DELETE FAIL")
        except:
            print("DELETE FAIL")
        finally:
            # Recibir fecha y hora por servicio web y enviarlo al servidor
            date_hour_str = client.get_date_hour()
            client.write_string(sock, date_hour_str)

            # 5) close() -> cerrar sesión
            sock.close()

        return return_code

    @staticmethod
    def  listusers(print_users) :
        return_code = 3

        # La operación solo se puede realizar si se esta registrado y conectado
        # Decidimos no mandar None por socket porque puede crear conflictos en el servidor
        if client._active_user == None:
            if client._active_user in client._registered:
                print("LIST_USERS FAIL, USER NOT CONNECTED")
                return 2
            else:
                print("LIST_USERS FAIL, USER DOES NOT EXIST")
                return 1

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(server_address)
        except:
            print("LIST_USERS FAIL")
            return return_code


        try:
            # 3) write() -> enviar petición al servidor

            client.write_string(sock, "LIST_USERS")
            client.write_string(sock, client._active_user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                num_users = client.read_number(sock)
                # Imprimir database
                client._database_listusers = []

                user_info = {
                    "username": "",
                    "ip": "",
                    "port": 0
                }
                # por cada cliente el servidor envía 3 cadenas de caracteres
                # se almacenan en _data_listusers
                for _ in range(num_users):
                    user_info_copy = user_info.copy()   # Copia del diccionario
                    user_info_copy["username"] = client.read_string(sock)
                    user_info_copy["ip"] = client.read_string(sock)
                    user_info_copy["port"] = client.read_number(sock)
                    client._database_listusers.append(user_info_copy)
                # imprimir en pantalla
                if print_users:
                    print("LIST_USERS OK")
                    for user in client._database_listusers:
                        print(f"{user['username']}\t{user['ip']}\t{user['port']}")
            elif return_code == 1 and print_users:
                print("LIST_USERS FAIL, USER DOES NOT EXIST")
            elif return_code == 2 and print_users:
                print("LIST_USERS FAIL, USER NOT CONNECTED")
            elif print_users:
                print("LIST_USERS FAIL")
        except:
            print("LIST_USERS FAIL")
        finally:
            # Recibir fecha y hora por servicio web y enviarlo al servidor
            date_hour_str = client.get_date_hour()
            client.write_string(sock, date_hour_str)

            # 5) close() -> cerrar sesión
            sock.close()

        return return_code

    @staticmethod
    def  listcontent(user) :
        return_code = 4

        # La operación solo se puede realizar si se esta registrado y conectado
        # Decidimos no mandar None por socket porque puede crear conflictos en el servidor
        if client._active_user == None:
            if client._active_user in client._registered:
                print("LIST_CONTENT FAIL, USER NOT CONNECTED")
                return 2
            else:
                print("LIST_CONTENT FAIL, USER DOES NOT EXIST")
                return 1
        # Los inputs deben tener máximo 256 bytes (255 caracteres + "\0")
        if len(user) >= 256:
            print("LIST_CONTENT FAIL")
            return return_code

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(server_address)
        except:
            print("LIST_CONTENT FAIL")
            return return_code


        try:
            # 3) write() -> enviar petición al servidor

            client.write_string(sock, "LIST_CONTENT")
            client.write_string(sock, client._active_user)
            client.write_string(sock, user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("LIST_CONTENT OK")
                # Imprimir database
                num_files = client.read_number(sock)
                # Por cada fichero el servidor enviara una cadena de caracteres con el nombre del fichero 
                for _ in range(num_files):
                    file_name = client.read_string(sock)
                    file_description = client.read_string(sock)
                    print(file_name)

            elif return_code == 1:
                print("LIST_CONTENT FAIL, USER DOES NOT EXIST")
            elif return_code == 2:
                print("LIST_CONTENT FAIL, USER NOT CONNECTED")
            elif return_code == 3:
                print("LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
            else:
                print("LIST_CONTENT FAIL")
        except:
            print("LIST_CONTENT FAIL")
        finally:
            # Recibir fecha y hora por servicio web y enviarlo al servidor
            date_hour_str = client.get_date_hour()
            client.write_string(sock, date_hour_str)

            # 5) close() -> cerrar sesión
            sock.close()

        return return_code

    @staticmethod
    def  getfile(user,  remote_FileName,  local_FileName) :
        return_code = 2

        # El usuario remoto no puedes ser tú mismo
        if client._active_user == user:
            print("GET_FILE FAIL")
            return return_code
        # Los inputs deben tener máximo 256 bytes (255 caracteres + "\0")
        if len(user) >= 256 or len(remote_FileName) >= 256 or len(local_FileName) >= 256:
            print("GET_FILE FAIL")
            return return_code
        # El archivo local no puede ser un directorio y el camino hacia él por los directorios debe existir
        # Permitimos que el archivo en sí no exista
        if not (local_FileName[-1] != "/" and os.path.exists(os.path.dirname(local_FileName))):
            print("GET_FILE FAIL")
            return return_code
            
        # Llamada a LIST_USERS
        # Aquí se verifica si el usuario está registrado y conectado
        code_listusers = client.listusers(False)
        if code_listusers != 0:
            print("GET_FILE FAIL")
            return code_listusers
        
        # CONEXIÓN PEER TO PEER

        # 1) socket() -> crear socket cliente

        # Conseguir IP y puerto de "user"
        user_info = None
        user_info_found = False
        i = 0
        while not user_info_found and i < len(client._database_listusers):
            user_info = client._database_listusers[i]
            if user_info["username"] == user:
                user_info_found = True
            i += 1
        # El usuario remoto no está conectado
        if not user_info_found:
            print("GET_FILE FAIL")
            return return_code

        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        peer_address = (user_info["ip"], int(user_info["port"]))

        try:
            # 2) connect() -> conexión con el servidor
            sock.connect(peer_address)
        except:
            print("GET_FILE FAIL")
            return return_code

        try:
            # 3) write() -> enviar petición al servidor
            
            # Llamada a GET_FILE
            client.write_string(sock, "GET_FILE")
            client.write_string(sock, remote_FileName)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("GET_FILE OK")
                num_bytes_remote_file = client.read_number(sock)
                # El contenido del archivo remoto se guarda en el archivo local especificado
                # La flag "w+b" hace que el archivo se crea si no existe y se trunca si existe
                file = open(local_FileName, 'w+b')
                while True:
                    data = sock.recv(num_bytes_remote_file+1)
                    if not data:
                        break
                    file.write(data)
                file.close()
            elif return_code == 1:
                print("GET_FILE FAIL, FILE NOT EXIST")
            else:
                print("GET_FILE FAIL")
                file.close()
        except:
            print("GET_FILE FAIL")
            file.close()
        finally:
            # 5) close() -> cerrar sesión
            sock.close()

        return return_code

    # *
    # **
    # * @brief Command interpreter for the client. It calls the protocol functions.
    @staticmethod
    def shell():

        while (True) :
            try :
                command = input("c> ")
                line = command.split(" ")
                if (len(line) > 0):

                    line[0] = line[0].upper()

                    if (line[0]=="REGISTER") :
                        if (len(line) == 2) :
                            client.register(line[1])
                        else :
                            print("Syntax error. Usage: REGISTER <userName>")

                    elif(line[0]=="UNREGISTER") :
                        if (len(line) == 2) :
                            client.unregister(line[1])
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            client.connect(line[1])
                        else :
                            print("Syntax error. Usage: CONNECT <userName>")
                    
                    elif(line[0]=="PUBLISH") :
                        if (len(line) >= 3) :
                            #  Remove first two words
                            description = ' '.join(line[2:])
                            client.publish(line[1], description)
                        else :
                            print("Syntax error. Usage: PUBLISH <fileName> <description>")

                    elif(line[0]=="DELETE") :
                        if (len(line) == 2) :
                            client.delete(line[1])
                        else :
                            print("Syntax error. Usage: DELETE <fileName>")

                    elif(line[0]=="LIST_USERS") :
                        if (len(line) == 1) :
                            client.listusers(True)
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            client.listcontent(line[1])
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            client.disconnect(line[1], True)
                        else :
                            print("Syntax error. Usage: DISCONNECT <userName>")

                    elif(line[0]=="GET_FILE") :
                        if (len(line) == 4) :
                            client.getfile(line[1], line[2], line[3])
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            # Check if connected -> DISCONNECT
                            if client._active_user != None:
                                client.disconnect(client._active_user, False)
                            break
                        else :
                            print("Syntax error. Use: QUIT")
                    else :
                        print("Error: command " + line[0] + " not valid.")
            except Exception as e:
                print("Exception: " + str(e))

    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 client.py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 client.py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535")
            return False
        
        client._server = args.s
        client._port = args.p

        return True


    # ******************** MAIN *********************
    @staticmethod
    def main(argv) :
        if (not client.parseArguments(argv)) :
            client.usage()
            return

        #  Write code here
        client.shell()
        print("+++ FINISHED +++")
    

if __name__=="__main__":
    client.main([])