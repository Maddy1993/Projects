namespace java generated.thrift.impl

enum MessageType {
    COMMIT_REQUEST,
    COMMIT,
    ABORT,
    SUCCESS
}

struct TwoPCPacket {
    1: i32 MessageType,
    2: string message
}

service TwoPCCommitProtocol extends {
    TwoPCPacket     getMessagePacket(),
    i32             getMessageType(),
    void            setType(1:i32 MessageType),
    string          getMessage(),
    void            setMessage(2:string message)
    void            sendPacket
}
