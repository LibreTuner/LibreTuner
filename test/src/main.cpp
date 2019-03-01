#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "serializeddata.h"

#include <vector>




struct Serializable {
    std::string string;
    int integer;
    float number;
};


namespace serialize {
    template<typename D>
    void deserialize(D &d, Serializable &ser) {
        d.deserialize(ser.string);
        d.deserialize(ser.integer);
        d.deserialize(ser.number);
    }

    template<typename S>
    void serialize(S &s, const Serializable &ser) {
        s.serialize(ser.string);
        s.serialize(ser.integer);
        s.serialize(ser.number);
    }
}




TEST_CASE("Serialization") {
    using Buffer = std::vector<char>;
    using OutputAdapter = serialize::OutputBufferAdapter<Buffer>;
    using InputAdapter = serialize::InputBufferAdapter<Buffer>;
    
    Buffer buffer;
    
    SECTION("Fundamental types") {
        // Serialize
        {
            serialize::Serializer<OutputAdapter> serializer(buffer);
            serializer.save<uint32_t>(0xdeadbeef);
            serializer.save<bool>(true);
            serializer.save<float>(63.2f);
        }
        // Deserialize
        {
            serialize::Deserializer<InputAdapter> deserializer(buffer);
            
            uint32_t u32;
            deserializer.load(u32);
            CHECK(u32 == 0xdeadbeef);
            
            bool b;
            deserializer.load(b);
            CHECK(b);
            
            float f;
            deserializer.load(f);
            CHECK(f == Approx(63.2f));
        }
    }
    
    SECTION("Strings") {
        // Serialize
        {
            serialize::Serializer<OutputAdapter> serializer(buffer);
            serializer.save<const char*>("string");
            serializer.save<std::string>("stdstring");
        }
        // Deserialize
        {
            serialize::Deserializer<InputAdapter> deserializer(buffer);
            std::string string1, string2;
            deserializer.load(string1);
            CHECK(string1 == "string");
            deserializer.load(string2);
            CHECK(string2 == "stdstring");
        }
    }
    
    SECTION("Vectors") {
        // Serialize
        {
            serialize::Serializer<OutputAdapter> serializer(buffer);
            std::vector<int> vec;
            vec.reserve(128);
            for (int i = 0; i < 128; ++i) {
                vec.emplace_back(i);
            }
            serializer.save(vec);
        }
        // Deserialize
        {
            serialize::Deserializer<InputAdapter> deserializer(buffer);
            
            std::vector<int> vec;
            deserializer.load(vec);
            REQUIRE(vec.size() == 128);
            for (int i = 0; i < 128; ++i) {
                CHECK(vec[i] == i);
            }
        }
           
    }
    
    SECTION("Structs") {
        // Serialize
        {
            Serializable ser{"string", 123, 0.123f};
            serialize::Serializer<OutputAdapter> serializer(buffer);
            serializer.save(ser);
            
            // Vector of structs
            std::vector<Serializable> ser_vec;
            ser_vec.reserve(16);
            for (int i = 0; i < 16; ++i) {
                ser_vec.emplace_back(Serializable{"string" + std::to_string(i), i, 0.123f});
            }
            serializer.save(ser_vec);
        }
        // Deserialize
        {
            serialize::Deserializer<InputAdapter> deserializer(buffer);
            
            Serializable ser;
            deserializer.load(ser);
            
            CHECK(ser.string == "string");
            CHECK(ser.integer == 123);
            CHECK(ser.number == Approx(0.123f));
        }
    }
    
    SECTION("Literals") {
        // Serialize
        {
            serialize::Serializer<OutputAdapter> serializer(buffer);
            int ar[] = {0, 1, 2, 3};
            serializer.save(ar);
            REQUIRE(buffer.size() == sizeof(ar));
        }
        // Deserialize
        {
            serialize::Deserializer<InputAdapter> deserializer(buffer);
            
            int ar[4];
            deserializer.load(ar);
            
            for (int i = 0; i < sizeof(ar) / sizeof(int); ++i) {
                CHECK(ar[i] == i);
            }
        }
    }
}


