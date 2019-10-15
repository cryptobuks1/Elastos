import Foundation

public class DID: NSObject {

    public static let METHOD: String = "elastos"
    var method: String!
    public var methodSpecificId: String! // icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN
    private var document: DIDDocument?
    private var resolved: Bool?
    private var resolveTimestamp: Date?
    private var listener: DListener?

    init(_ method: String, _ methodSpecificId: String) {
        self.method = method
        self.methodSpecificId = methodSpecificId
    }
    
    public override init() {
        super.init()
    }

    public init(_ did: String) throws {
        super.init()
        self.listener = DListener(self)
        try ParserHelper.parase(did, true, self.listener!)
    }

    public func toExternalForm() -> String {
        return String("did:\(method!):\(methodSpecificId!)")
    }

    public func toString() -> String {
        return toExternalForm()
    }
    
    public override var hash: Int {
        return DID.METHOD.hash + self.methodSpecificId.hash
    }

    public override func isEqual(_ object: Any?) -> Bool {

        if object is DID {
            let did = object as! DID
            let didExternalForm = did.toExternalForm()
            let selfExternalForm = toExternalForm()
            return didExternalForm.isEqual(selfExternalForm)
        }
        
        if object is String {
            let did = object as! String
            let selfExternalForm = toExternalForm()
            return did.isEqual(selfExternalForm)
        }
        
        return super.isEqual(object);
    }
    
    
    public func resolve() throws -> DIDDocument {
        if document != nil {return document!}
        do {
            document = try DIDStore.shareInstance()!.resolveDid(self)
        } catch {
            throw error
        }
        
        if document != nil {
            self.resolveTimestamp = Date()
        }
        return document!
    }
    
    // delete after test
    public static func testSign() throws {
        
        let dhKey: HDKey = try HDKey.fromMnemonic("cloth always junk crash fun exist stumble shift over benefit fun toe", "")
        let key: DerivedKey = try dhKey.derive(0)
        
        let privateKey: [UInt8] = try key.getPrivateKeyBytes()
        print(privateKey)
        
        let data = Data(bytes: privateKey, count: privateKey.count)
        let pkPointer: UnsafePointer<UInt8> = data.toPointer()!
        let none: UnsafePointer<Int8> = "1111".toUnsafePointerInt8()!
        let raw: UnsafeMutableRawPointer = UnsafeMutableRawPointer(mutating: none)
        var _: CVaListPointer = CVaListPointer(_fromUnsafeMutablePointer: raw)

        let sign: UnsafeMutablePointer<Int8> = UnsafeMutablePointer.allocate(capacity: 1000)

        let args: [CVarArg] = ["foo", 3, "foo".count, "hello world".count]
//        withVaList(args) { ecdsa_sign_base64v(sign, pkPointer, 2, args) }
        
//        QIOWXABY+daUhaEDZ3uP6CmApXyZpQqw5/8BUWJVxEQ=
//        QL4eNEJpsIMsaGjBorRPlkr/eOR1Ee9GME/y53KHXSk=
        let sk: UnsafeMutablePointer<Int8> = ecdsa_sign_base64v(sign, pkPointer, 2, getVaList(args))
        print(String(cString: sign))
    }
}

extension Data {
  func toPointer() -> UnsafePointer<UInt8>? {
    let buffer = UnsafeMutablePointer<UInt8>.allocate(capacity: count)
    let stream = OutputStream(toBuffer: buffer, capacity: count)

    stream.open()
    withUnsafeBytes({ (p: UnsafePointer<UInt8>) -> Void in
      stream.write(p, maxLength: count)
    })

    stream.close()

    return UnsafePointer<UInt8>(buffer)
  }
}

class DListener: DIDURLBaseListener {
    
    public var name: String?
    public var value: String?
    public var did: DID?

    init(_ did: DID) {
        super.init()
        self.did = did
    }
    
    override func exitMethod(_ ctx: DIDURLParser.MethodContext) {
        let method: String = ctx.getText()
        if (method != DID.METHOD){
            // TODO: throw error
            // let error = DIDError.failue("Unknown method: \(method)")
        }
        self.did!.method = DID.METHOD
    }
    
    override func exitMethodSpecificString(_ ctx: DIDURLParser.MethodSpecificStringContext) {
        self.did!.methodSpecificId = ctx.getText()
    }

}

