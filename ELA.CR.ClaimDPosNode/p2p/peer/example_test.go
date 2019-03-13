package peer_test

import (
	"fmt"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/elastos/Elastos.ELA/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p/peer"
	"math/rand"
	"net"
)

func makeEmptyMessage(cmd string) (message p2p.Message, err error) {
	switch cmd {
	case p2p.CmdVersion:
		message = new(msg.Version)
	case p2p.CmdVerAck:
		message = new(msg.VerAck)
	default:
		err = fmt.Errorf("unknown message type %s", cmd)
	}

	return message, err
}

// mockRemotePeer creates a basic inbound peer listening on the simnet port for
// use with Example_peerConnection.  It does not return until the listner is
// active.
func mockRemotePeer() error {
	// Configure peer to act as a simnet node that offers no services.
	peerCfg := &peer.Config{
		Magic:            123123,
		ProtocolVersion:  0,
		Services:         0,
		DisableRelayTx:   true,
		HostToNetAddress: nil,
		MakeEmptyMessage: makeEmptyMessage,
		BestHeight: func() uint64 {
			return 0
		},
		IsSelfConnection: func(nonce uint64) bool {
			return false
		},
		GetVersionNonce: func() uint64 {
			return rand.Uint64()
		},
	}

	// Accept connections on the simnet port.
	listener, err := net.Listen("tcp", "127.0.0.1:20338")
	if err != nil {
		return err
	}
	go func() {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Printf("Accept: error %v\n", err)
			return
		}

		// Create and start the inbound peer.
		p := peer.NewInboundPeer(peerCfg)
		p.AssociateConnection(conn)
	}()

	return nil
}

//// This example demonstrates the basic process for initializing and creating an
//// outbound peer.  Peers negotiate by exchanging version and verack messages.
//// For demonstration, a simple handler for version message is attached to the
//// peer.
//func Example_newOutboundPeer() {
//	if testing.Short() {
//		return
//	}
//	// Ordinarily this will not be needed since the outbound peer will be
//	// connecting to a remote peer, however, since this example is executed
//	// and tested, a mock remote peer is needed to listen for the outbound
//	// peer.
//	if err := mockRemotePeer(); err != nil {
//		fmt.Printf("mockRemotePeer: unexpected error %v\n", err)
//		return
//	}
//
//	// Create an outbound peer that is configured to act as a simnet node
//	// that offers no services and has listeners for the version and verack
//	// messages.  The verack listener is used here to signal the code below
//	// when the handshake has been finished by signalling a channel.
//	verack := make(chan struct{})
//	peerCfg := &peer.Config{
//		Magic:            123123,
//		ProtocolVersion:  0,
//		Services:         0,
//		DisableRelayTx:   true,
//		HostToNetAddress: nil,
//		MakeEmptyMessage: makeEmptyMessage,
//		BestHeight: func() uint64 {
//			return 0
//		},
//		IsSelfConnection: func(nonce uint64) bool {
//			return false
//		},
//		GetVersionNonce: func() uint64 {
//			return rand.Uint64()
//		},
//	}
//	peerCfg.AddMessageFunc(func(peer *peer.Peer, message p2p.Message) {
//		switch message.(type) {
//		case *msg.Version:
//			fmt.Println("outbound: received version")
//		case *msg.VerAck:
//			verack <- struct{}{}
//		}
//	})
//
//	p, err := peer.NewOutboundPeer(peerCfg, "127.0.0.1:20338")
//	if err != nil {
//		fmt.Printf("NewOutboundPeer: error %v\n", err)
//		return
//	}
//
//	// Establish the connection to the peer address and mark it connected.
//	conn, err := net.Dial("tcp", p.Addr())
//	if err != nil {
//		fmt.Printf("net.Dial: error %v\n", err)
//		return
//	}
//	p.AssociateConnection(conn)
//
//	// Wait for the verack message or timeout in case of failure.
//	select {
//	case <-verack:
//	case <-time.After(time.Second * 1):
//		fmt.Printf("Example_peerConnection: verack timeout")
//	}
//
//	// Disconnect the peer.
//	p.Disconnect()
//	p.WaitForDisconnect()
//
//	// Output:
//	// outbound: received version
//}
