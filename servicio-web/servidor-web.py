import datetime, logging, socket
from wsgiref.simple_server import make_server
from spyne import Application, ServiceBase, Unicode, rpc
from spyne.protocol.soap import Soap11
from spyne.server.wsgi import WsgiApplication

class FechaHora(ServiceBase):
    @rpc(_returns=Unicode)
    def send_date_hour(ctx):
        date = datetime.datetime.now()
        date_time = date.strftime("%d/%m/%Y %H:%M:%S")
        return date_time

application = Application(services=[FechaHora],
                          tns='http://tests.python-zeep.org/',
                          in_protocol=Soap11(validator='lxml'),
                          out_protocol=Soap11())
application = WsgiApplication(application)

if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    logging.getLogger('spyne.protocol.xml').setLevel(logging.DEBUG)
    port = 30_000
    #ip_user = socket.gethostbyname(socket.gethostname())
    ip_user = 'localhost'
    logging.info(f"listening to {ip_user}:{port}")
    server = make_server(ip_user, port, application)
    server.serve_forever()