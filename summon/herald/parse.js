/* Parse Herald advertisements */

var parse_advertisement = function (advertisement, cb) {

    if (advertisement.localName === 'herald') {
        if (advertisement.manufacturerData) {
            // Need at least 3 bytes. Two for manufacturer identifier and
            // one for the service ID.
            if (advertisement.manufacturerData.length >= 3) {
                // Check that manufacturer ID and service byte are correct
                var manufacturer_id = advertisement.manufacturerData.readUIntLE(0, 2);
                var service_id = advertisement.manufacturerData.readUInt8(2);
                if (manufacturer_id == 0x02E0 && service_id == 0x21) {
                    // OK! This looks like a Herald packet
                    // Length should be at least header + version + 1 character
                    if (advertisement.manufacturerData.length >= (3+1+1)) {
                        var herald = advertisement.manufacturerData.slice(3);

                        // Version is the first thing.
                        var version = herald.readUInt8(0);

                        //var room = herald.toString('utf8', 1);
                        var room = herald.readUIntBE(2,2);

                        var out = {
                            device: 'Herald',
                            room_string: room,
                            _meta: {
                                room: room
                            },
                        };

                        cb(out);
                        return;
                    }
                }
            }
        }
    }

    cb(null);
}


module.exports = {
    parseAdvertisement: parse_advertisement
};
