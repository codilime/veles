async def blob_get_data(obj, params):
    width = obj.attr['width']
    start = params['start']
    end = params['end']
    # XXX
    pass

class Plugin:
    name = 'core'
    mthds = [
        ({'blob.data'}, 'get_data', blob_get_data),
    ]
