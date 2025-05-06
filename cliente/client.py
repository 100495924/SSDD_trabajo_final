from enum import Enum
import argparse
import socket
import sys

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

    # se usa para saber el username activo
    # connect -> set to user (only if OK)
    # disconnect -> set to None (always)
    _active_user = None

    # ******************** METHODS *******************
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
        sock.sendall(str.encode() + b'\0') 
        # sock.sendall(b'\0') 

    def write_number(sock, num):
        a = str(num)
        client.write_string(sock, a)
    
    def socket_cheatsheet():
        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        try:
            # 3) write() -> enviar petición al servidor
            message_send = "String prueba"
            client.write_string(sock, message_send)
            message_send = 10
            client.write_number(sock, message_send)

            # 4) read() -> recibir respuesta del servidor
            message_read = client.read_string(sock)
            message_read = client.read_number(sock)

            print(f"mensaje: {message_read}")
        finally:
            # 5) close() -> cerrar sesión
            sock.close()


    @staticmethod
    def  register(user) :
        # Comprobar user <= 256 bytes

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        return_code = 2

        try:
            # 3) write() -> enviar petición al servidor
            client.write_string(sock, "REGISTER")
            client.write_string(sock, user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("REGISTER OK")
            elif return_code == 1:
                print("USERNAME IN USE")
            else:
                print("REGISTER FAIL")
        except:
            print("REGISTER FAIL")
        finally:
            # 5) close() -> cerrar sesión
            sock.close()

        return return_code

   
    @staticmethod
    def  unregister(user) :
        # Comprobar user <= 256 bytes

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        return_code = 2

        try:
            # 3) write() -> enviar petición al servidor
            client.write_string(sock, "UNREGISTER")
            client.write_string(sock, user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("UNREGISTER OK")
            elif return_code == 1:
                print("USER DOES NOT EXIST")
            else:
                print("UNREGISTER FAIL")
        except:
            print("UNREGISTER FAIL")
        finally:
            # 5) close() -> cerrar sesión
            sock.close()

        return return_code


    @staticmethod
    def  connect(user) :
        # TODO Comprobar user <= 256 bytes

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        return_code = 3

        try:
            # 3) write() -> enviar petición al servidor

            # TODO EMPEZAR EJECUCIÓN DE HILO

            ip_user = socket.gethostbyname(socket.gethostbyname())
            port_user = sock.getsockname()[1]

            client.write_string(sock, "CONNECT")
            client.write_string(sock, user)
            client.write_string(sock, ip_user)
            client.write_number(sock, port_user)

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
            sock.close()

        return return_code


    @staticmethod
    def  disconnect(user) :
        # TODO Comprobar user <= 256 bytes

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        return_code = 3

        try:
            # 3) write() -> enviar petición al servidor
            
            # TODO PARAR EJECUCIÓN DE HILO (INCLUSO CON ERRORES)

            client.write_string(sock, "DISCONNECT")
            client.write_string(sock, user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("DISCONNECT OK")
            elif return_code == 1:
                print("DISCONNECT FAIL, USER DOES NOT EXIST")
            elif return_code == 2:
                print("DISCONNECT FAIL, USER ALREADY CONNECTED")
            else:
                print("DISCONNECT FAIL")
        except:
            print("DISCONNECT FAIL")
        finally:
            # 5) close() -> cerrar sesión
            client._active_user = None
            sock.close()

        return return_code


    @staticmethod
    def  publish(fileName,  description) :
        # TODO Comprobar fileName <= 256 bytes
        # TODO Comprobar description <= 256 bytes
        # TODO Comprobar client._active_user != None

        # Se envia una cadena con el path absoluto del fichero (esta cadena no podra contener 
        # espacios en blanco). El tamaño maximo del path absoluto del fichero sera de 256bytes.

        # Se envia una cadena de caracteres con la descripcion del contenido. Esta cadena
        # podra contener espacios en blanco y su tamaño maximo sera de 256 bytes.

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        return_code = 4

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
            # 5) close() -> cerrar sesión
            sock.close()

        return return_code


    @staticmethod
    def  delete(fileName) :
        # TODO Comprobar fileName <= 256 bytes
        # TODO Comprobar client._active_user != None

        # Se envia una cadena con el path absoluto del fichero (esta cadena no podra contener 
        # espacios en blanco). El tamaño maximo del path absoluto del fichero sera de 256bytes.

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        return_code = 4

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
            # 5) close() -> cerrar sesión
            sock.close()

        return return_code

    @staticmethod
    def  listusers() :
        # TODO Comprobar client._active_user != None

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        return_code = 3

        try:
            # 3) write() -> enviar petición al servidor

            client.write_string(sock, "LIST_USERS")
            client.write_string(sock, client._active_user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("LIST_USERS OK")
                num_users = client.read_number(sock)
                # TODO imprimir database
                # por cada cliente enviar a 3 cadenas de caracteres
            elif return_code == 1:
                print("LIST_USERS FAIL, USER DOES NOT EXIST")
            elif return_code == 2:
                print("LIST_USERS FAIL, USER NOT CONNECTED")
            else:
                print("LIST_USERS FAIL")
        except:
            print("LIST_USERS FAIL")
        finally:
            # 5) close() -> cerrar sesión
            sock.close()

        return return_code

    @staticmethod
    def  listcontent(user) :
        # TODO Comprobar user <= 256 bytes

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el servidor
        sock.connect(server_address)

        return_code = 3

        try:
            # 3) write() -> enviar petición al servidor

            client.write_string(sock, "LIST_USERS")
            client.write_string(sock, client._active_user)
            client.write_string(sock, user)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("LIST_CONTENT OK")
                # TODO imprimir database
                num_files = client.read_number(sock)
                # Por cada fichero el servidor enviara una cadena de caracteres con el nombre del fichero 
                # (el tamaño maximo del fichero es de 256 bytes).
            elif return_code == 1:
                print("LIST_CONTENT FAIL, USER DOES NOT EXIST")
            elif return_code == 2:
                print("LIST_CONTENT FAIL, USER NOT CONNECTED")
            elif return_code == 3:
                print("LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
            else:
                print("LIST_CONTENT FAIL")
        except:
            print("LIST_USERS FAIL")
        finally:
            # 5) close() -> cerrar sesión
            sock.close()

        return return_code

    @staticmethod
    def  getfile(user,  remote_FileName,  local_FileName) :
        # TODO Comprobar user <= 256 bytes
        # Comprobar remote_FileName <= 256 bytes (?)
        # Comprobar local_FileName <= 256 bytes (?)

        # TODO Llamada a LIST_USERS
        code_listusers = client.listusers()
        if code_listusers != 0:
            print("GET_FILE FAIL")
            return code_listusers

        # TODO CONEXIÓN PEER TO PEER

        # 1) socket() -> crear socket cliente
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_address = (client._server, int(client._port))

        # 2) connect() -> conexión con el otro cliente
        sock.connect(server_address)

        return_code = 2

        try:
            # 3) write() -> enviar petición al servidor
            
            # Llamada a GET_FILE

            client.write_string(sock, "GET_FILE")
            client.write_string(sock, remote_FileName)

            # 4) read() -> recibir respuesta del servidor
            return_code = client.read_number(sock)

            if return_code == 0:
                print("GET_FILE OK")
            elif return_code == 1:
                print("GET_FILE FAIL, FILE NOT EXIST")
            else:
                print("GET_FILE FAIL")
        except:
            print("GET_FILE FAIL")
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
                            client.listusers()
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            client.listcontent(line[1])
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            client.disconnect(line[1])
                        else :
                            print("Syntax error. Usage: DISCONNECT <userName>")

                    elif(line[0]=="GET_FILE") :
                        if (len(line) == 4) :
                            client.getfile(line[1], line[2], line[3])
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            # Check if connected -> DISCONNECT (username??? elimnar todos???)
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