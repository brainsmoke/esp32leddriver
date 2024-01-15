# remote debugging

#def init(server):
#
#    import uctypes,cball,binascii
#
#    @server.route("/read", "POST")
#    def handler(req):
#        value, token = get_val(req)
#        values = value.split(',')
#        addr = int(values[0], 16)
#        size = int(values[1], 16)
#        b = uctypes.bytes_at(addr, size)
#        req.write_all(binascii.hexlify(b))
#
#    uint32 = { 'val': 0 | uctypes.UINT32 }
#    @server.route("/write4", "POST")
#    def handler(req):
#        value, token = get_val(req)
#        values = value.split(',')
#        addr = int(values[0], 16)
#        val = int(values[1], 16)
#        v = uctypes.struct(addr, uint32, uctypes.NATIVE)
#        v.val = val
#        req.write_all(hex(v.val))
#
#    @server.route("/write", "POST")
#    def handler(req):
#        value, token = get_val(req)
#        values = value.split(',')
#        addr = int(values[0], 16)
#        b = binascii.unhexlify(values[1])
#        ba = uctypes.bytearray_at(addr, len(b))
#        cball.bytearray_memcpy(ba, b)
#        b = uctypes.bytes_at(addr, len(b))
#        req.write_all(binascii.hexlify(b))

