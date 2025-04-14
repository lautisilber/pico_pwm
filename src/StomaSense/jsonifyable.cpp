//
//  StomaSenseDefs.cpp
//  stoma_sense_algorithm
//
//  Created by Lautaro Silbergleit on 11/04/2025.
//

#include <StomaSense/jsonifyable.h>
#include <ArduinoJson.h>

using namespace StomaSense;
// using namespace ArduinoJson;

bool Jsonifyable::to_stream(Stream *stream, bool pretty) const {
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    to_json(&obj);
    if (pretty) {
        return serializeJsonPretty(obj, *stream);
    } else {
        return serializeJson(obj, *stream);
    }
}

bool Jsonifyable::from_stream(Stream *stream) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, *stream);
    if (err) {
        STOMASENSE_WARN("couldn't deserialize json");
        return false;
    }
    if (!doc.is<JsonObject>()) {
        STOMASENSE_WARN("deserialized json wasn't an object");
        return false;
    }
    JsonObject obj = doc.to<JsonObject>();
    return from_json(&obj);
}