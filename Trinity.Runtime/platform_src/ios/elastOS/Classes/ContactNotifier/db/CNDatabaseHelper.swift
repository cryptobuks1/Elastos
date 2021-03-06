/*
* Copyright (c) 2020 Elastos Foundation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
 
import SQLite
 
public class CNDatabaseHelper : SQLiteOpenHelper {
    private static let DATABASE_VERSION = 2
    
    private static let LOG_TAG = "ConNotDBHelper"
    
    // Tables
    private static let DATABASE_NAME = "contactnotifier.db"
    public static let CONTACTS_TABLE = "contacts"
    public static let SENT_INVITATIONS_TABLE = "sentinvitations"
    public static let RECEIVED_INVITATIONS_TABLE = "receivedinvitations"
    
    // Tables fields
    public static let DID_SESSION_DID = "didsessiondid"
    public static let INVITATION_ID = "iid"
    public static let DID = "did"
    public static let CARRIER_ADDRESS = "carrieraddress"
    public static let CARRIER_USER_ID = "carrieruserid"
    public static let NOTIFICATIONS_BLOCKED = "notificationsblocked"
    public static let ADDED_DATE = "added"
    public static let SENT_DATE = "sent"
    public static let RECEIVED_DATE = "received"
    public static let NAME = "name"
    public static let AVATAR_CONTENTTYPE = "avatar_contenttype"
    public static let AVATAR_DATA = "avatar_data"
    
    public static let KEY = "key"
    public static let VALUE = "value"
    
    public init() {
        let dataPath = NSHomeDirectory() + "/Documents/data/"
        super.init(dbFullPath: "\(dataPath)/\(CNDatabaseHelper.DATABASE_NAME)", dbNewVersion: CNDatabaseHelper.DATABASE_VERSION)
    }
    
    public override func onCreate(db: Connection) {
        // CONTACTS
        let contactsSQL = "create table " +
            CNDatabaseHelper.CONTACTS_TABLE + "(cid integer primary key autoincrement, " +
            CNDatabaseHelper.DID_SESSION_DID + " varchar(128), " +
            CNDatabaseHelper.DID + " varchar(128), " +
            CNDatabaseHelper.CARRIER_USER_ID + " varchar(128), " + // Permanent friend user id to talk (notifications) to him
            CNDatabaseHelper.NOTIFICATIONS_BLOCKED + " integer(1), " + // Whether this contact can send notifications to current user or not
            CNDatabaseHelper.ADDED_DATE + " TIMESTAMP, " +
            CNDatabaseHelper.NAME + " varchar(128), " +
            CNDatabaseHelper.AVATAR_CONTENTTYPE + " varchar(32), " +
            CNDatabaseHelper.AVATAR_DATA + " text)"
        try! db.execute(contactsSQL)
        
        // SENT INVITATIONS
        let sentInvitationsSQL = "create table " +
            CNDatabaseHelper.SENT_INVITATIONS_TABLE + "(" +
            CNDatabaseHelper.INVITATION_ID + " integer primary key autoincrement, " +
            CNDatabaseHelper.DID_SESSION_DID + " varchar(128), " +
            CNDatabaseHelper.DID + " varchar(128), " +
            CNDatabaseHelper.CARRIER_ADDRESS + " varchar(128), " +
            CNDatabaseHelper.SENT_DATE + " TIMESTAMP)"
        try! db.execute(sentInvitationsSQL)
        
        // RECEIVED INVITATIONS
        let receivedInvitationsSQL = "create table " +
            CNDatabaseHelper.RECEIVED_INVITATIONS_TABLE + "(" +
            CNDatabaseHelper.INVITATION_ID + " integer primary key autoincrement, " +
            CNDatabaseHelper.DID_SESSION_DID + " varchar(128), " +
            CNDatabaseHelper.DID + " varchar(128), " +
            CNDatabaseHelper.CARRIER_USER_ID + " varchar(128), " +
            CNDatabaseHelper.RECEIVED_DATE + " TIMESTAMP)";
        try! db.execute(receivedInvitationsSQL)
    }
    
    public override func onUpgrade(db: Connection, oldVersion: Int, newVersion: Int) {
        // Use the if (old < N) format to make sure users get all upgrades even if they directly upgrade from vN to v(N+5)
        if (oldVersion < 2) {
            Log.d(CNDatabaseHelper.LOG_TAG, "Upgrading database to v2")
            upgradeToV2(db: db)
        }
    }
    
    // 20200601 - Added contact name and avatar
    private func upgradeToV2(db: Connection) {
        do {
            var strSQL = "ALTER TABLE " + CNDatabaseHelper.CONTACTS_TABLE + " ADD COLUMN " + CNDatabaseHelper.NAME + " varchar(128); "
            try db.execute(strSQL)

            strSQL = "ALTER TABLE " + CNDatabaseHelper.CONTACTS_TABLE + " ADD COLUMN " + CNDatabaseHelper.AVATAR_CONTENTTYPE + " varchar(32); "
            try db.execute(strSQL)

            strSQL = "ALTER TABLE " + CNDatabaseHelper.CONTACTS_TABLE + " ADD COLUMN " + CNDatabaseHelper.AVATAR_DATA + " text; "
            try db.execute(strSQL)
        }
        catch (let error) {
            print(error)
        }
    }
    
    public override func onDowngrade(db: Connection, oldVersion: Int, newVersion: Int) {
    }
}
