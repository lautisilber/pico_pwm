//
//  StomaSenseSessionWithJson.h
//  stoma_sense_algorithm
//
//  Created by Lautaro Silbergleit on 11/04/2025.
//

#ifndef _STOMA_SENSE_SESSION_WITH_JSON_H_
#define _STOMA_SENSE_SESSION_WITH_JSON_H_

#include <StomaSense/defs.h>
#include <StomaSense/protocol.h>
#include <StomaSense/jsonifyable.h>

#include <ArduinoJson.h>

namespace StomaSense
{

    class ProtocolWithJson : public Protocol, public Jsonifyable
    {
    public:
        ProtocolWithJson() : Protocol() {}
        ProtocolWithJson(const ProtocolStep *protocol_steps, size_t size)
            : Protocol(protocol_steps, size) {}
        ProtocolWithJson(std::initializer_list<ProtocolStep> protocol_steps)
            : Protocol(protocol_steps) {}

        void to_json(JsonObject *obj) const override;
        bool from_json(JsonObject *obj) override;
    };

}

#endif /* _STOMA_SENSE_SESSION_WITH_JSON_H_ */