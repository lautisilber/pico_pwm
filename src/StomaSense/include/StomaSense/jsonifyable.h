#ifndef _STOMASENSE_JSONIFYABLE_H_
#define _STOMASENSE_JSONIFYABLE_H_

#include <ArduinoJson.h>
#include <StomaSense/stream.h>

namespace StomaSense {
    struct Jsonifyable
    {
        virtual void to_json(JsonObject *obj) const = 0;
        bool to_stream(Stream *stream, bool pretty) const;

        virtual bool from_json(JsonObject *obj) = 0;
        bool from_stream(Stream *stream);
    };
}

#endif /* _STOMASENSE_JSONIFYABLE_H_ */