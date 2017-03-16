# Copyright 2017 CodiLime
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from veles.async_conn.plugin import method, query
from veles.data.bindata import BinData
from veles.proto.blob import (
    query_blob_get_map,
    query_blob_get_data,
    method_blob_set_data,
    method_blob_truncate_data,
    BlobGetDataResult,
    BlobGetMapResult,
    BlobRange,
)


@query(query_blob_get_map, {'blob.stored'})
async def get_map_stored(conn, node, params, tracer):
    sz = await tracer.get_bindata_size(node, 'data')
    width = await tracer.get_attr(node, 'width')
    ope = BinData(width).octets_per_element()
    return BlobGetMapResult(
        map=[
            BlobRange(
                start=0,
                end=sz // ope,
            ),
        ],
        width=width,
    )


@query(query_blob_get_data, {'blob.stored'})
async def get_data_stored(conn, node, params, tracer):
    width = await tracer.get_attr(node, 'width')
    start = params.start
    end = params.end
    ope = BinData(width).octets_per_element()
    raw = await tracer.get_bindata(node, 'data', start * ope, end * ope)
    data = BinData.from_raw_data(width, raw)
    return BlobGetDataResult(
        data=data,
        mask=None,
    )


@method(method_blob_set_data, {'blob.stored'})
async def set_data_stored(conn, node, params):
    width = node.attr['width']
    sz = node.bindata.get('data', 0)
    ope = BinData(width).octets_per_element()
    if width != params.data.width:
        raise Exception('width mismatch')
    if sz < params.start * ope:
        raise Exception('write after EOF')
    anode = conn.get_node_norefresh(node.id)
    await anode.set_bindata('data', params.start * ope, params.data.raw_data)


@method(method_blob_truncate_data, {'blob.stored'})
async def truncate_data_stored(conn, node, params):
    width = node.attr['width']
    ope = BinData(width).octets_per_element()
    sz = node.bindata.get('data', 0)
    if params.size * ope > sz:
        raise Exception('cannot truncate to a bigger size')
    anode = conn.get_node_norefresh(node.id)
    await anode.set_bindata('data', params.size * ope, b'', truncate=True)


@query(query_blob_get_map, {'blob.remap'})
async def get_map_remap(trace, node, params):
    # XXX
    raise NotImplementedError
