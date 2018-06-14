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

                        var room = herald.readUInt16BE(1).toString();
                        //var room = herald.toString();
                        //var room=hex2a(herald);

                        var out = {
                            device: 'Herald',
                            room_string: room,
                            version: version,
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

//function hex2a(hexx) {
   // var hex = hexx.toString(); 
    //var string = '';
    //for (var i = 0; (i < hex.length && hex.substr(i, 2) !== '00'); i += 2)
        //string += String.fromCharCode(parseInt(hex.substr(i, 2), 16));
   // return string;
//}

module.exports = {
    parseAdvertisement: parse_advertisement
};
