class VelesException(Exception):
    pass


class ConnectionException(VelesException):
    def __init__(self, reason):
        message = 'Connection problem due to: %s' % reason
        super(ConnectionException, self).__init__(message)


class RequestFailed(VelesException):
    pass
