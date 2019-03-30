package peer

import (
	"bytes"
	"crypto/rand"
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"net"
	"strconv"
	"sync"
	"sync/atomic"
	"time"

	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p"
)

const (
	// idleTimeout is the duration of inactivity before we time out a peer.
	idleTimeout = 30 * time.Second

	// negotiateTimeout is the duration of inactivity before we timeout a
	// peer that hasn't completed the initial version negotiation.
	negotiateTimeout = 5 * time.Second

	// writeTimeout is the duration of write message before we time out a peer.
	writeTimeout = 5 * time.Second
)

var (
	// ErrPeerDisconnected is the error to return when attempt to send message
	// to peer, but the peer was disconnected.
	ErrPeerDisconnected = errors.New("peer already disconnected")
)

// PID is the encoded public key data used as peer's ID.
type PID [33]byte

// Equal returns if the passed PID is equal to origin PID.
func (p PID) Equal(o PID) bool {
	return bytes.Equal(p[:], o[:])
}

// String returns the hexadecimal format string of the PID.
func (p PID) String() string {
	return hex.EncodeToString(p[:])
}

// outMsg is used to house a message to be sent along with a channel to signal
// when the message has been sent (or won't be sent due to things such as
// shutdown)
type outMsg struct {
	msg      p2p.Message
	doneChan chan<- error
}

// StatsSnap is a snapshot of peer stats at a point in time.
type StatsSnap struct {
	ID             uint64
	PID            string
	Addr           string
	Services       uint64
	LastSend       time.Time
	LastRecv       time.Time
	ConnTime       time.Time
	Version        uint32
	Inbound        bool
	LastPingTime   time.Time
	LastPingMicros int64
}

// MessageFunc is a message handler in peer's configuration
type MessageFunc func(peer *Peer, msg p2p.Message)

// Config is a descriptor which specifies the peer instance configuration.
type Config struct {
	PID              PID
	Magic            uint32
	ProtocolVersion  uint32
	Services         uint64
	Port             uint16
	PingInterval     time.Duration
	Sign             func(data []byte) []byte
	PingNonce        func(pid PID) uint64
	PongNonce        func(pid PID) uint64
	MakeEmptyMessage func(cmd string) (p2p.Message, error)
	MessageFunc      MessageFunc
}

// minUint32 is a helper function to return the minimum of two uint32s.
// This avoids a math import and the need to cast to floats.
func minUint32(a, b uint32) uint32 {
	if a < b {
		return a
	}
	return b
}

// newNetAddress attempts to extract the IP address and port from the passed
// net.Addr interface and create a NetAddress structure using that information.
func newNetAddress(addr net.Addr, services uint64) (*p2p.NetAddress, error) {
	// addr will be a net.TCPAddr when not using a proxy.
	if tcpAddr, ok := addr.(*net.TCPAddr); ok {
		ip := tcpAddr.IP
		port := uint16(tcpAddr.Port)
		na := p2p.NewNetAddressIPPort(ip, port, services)
		return na, nil
	}

	// For the most part, addr should be one of the two above cases, but
	// to be safe, fall back to trying to parse the information from the
	// address string as a last resort.
	host, portStr, err := net.SplitHostPort(addr.String())
	if err != nil {
		return nil, err
	}
	ip := net.ParseIP(host)
	port, err := strconv.ParseUint(portStr, 10, 16)
	if err != nil {
		return nil, err
	}
	na := p2p.NewNetAddressIPPort(ip, uint16(port), services)
	return na, nil
}

// HostToNetAddrFunc is a func which takes a host, port, services and returns the netaddress.
type HostToNetAddrFunc func(host string, port uint16, services uint64) (*p2p.NetAddress, error)

type Peer struct {
	// The following variables must only be used atomically.
	lastRecv   int64
	lastSend   int64
	connected  int32
	started    int32
	disconnect int32

	conn net.Conn

	// These fields are set at creation time and never modified, so they are
	// safe to read from concurrently without a mutex.
	addr         string
	cfg          Config
	inbound      bool
	messageFuncs []MessageFunc

	flagsMtx           sync.Mutex // protects the peer flags below
	id                 uint64
	na                 *p2p.NetAddress
	pk                 *crypto.PublicKey
	pid                PID
	services           uint64
	advertisedProtoVer uint32 // protocol version advertised by remote
	protocolVersion    uint32 // negotiated protocol version

	// These fields keep track of statistics for the peer and are protected
	// by the statsMtx mutex.
	statsMtx       sync.RWMutex
	timeConnected  time.Time
	lastPingNonce  uint64    // Set to nonce if we have a pending ping.
	lastPingTime   time.Time // Time we sent last ping.
	lastPingMicros int64     // Time for last ping to return.

	sendQueue chan outMsg
	inQuit    chan struct{}
	outQuit   chan struct{}
	quit      chan struct{}
}

// AddMessageFunc add a new message handler for the peer.
func (p *Peer) AddMessageFunc(messageFunc MessageFunc) {
	if messageFunc != nil {
		p.messageFuncs = append(p.messageFuncs, messageFunc)
	}
}

// handleMessage will be invoked when a message received.
func (p *Peer) handleMessage(peer *Peer, msg p2p.Message) {
	for _, messageFunc := range p.messageFuncs {
		messageFunc(peer, msg)
	}
}

// String returns the peer's address and directionality as a human-readable
// string.
//
// This function is safe for concurrent access.
func (p *Peer) String() string {
	return fmt.Sprintf("%s (%s)", p.addr, directionString(p.inbound))
}

// StatsSnapshot returns a snapshot of the current peer flags and statistics.
//
// This function is safe for concurrent access.
func (p *Peer) StatsSnapshot() *StatsSnap {
	p.statsMtx.RLock()

	p.flagsMtx.Lock()
	id := p.id
	pid := p.pid
	addr := p.addr
	services := p.services
	protocolVersion := p.advertisedProtoVer
	p.flagsMtx.Unlock()

	// Get a copy of all relevant flags and stats.
	statsSnap := &StatsSnap{
		ID:             id,
		PID:            hex.EncodeToString(pid[:]),
		Addr:           addr,
		Services:       services,
		LastSend:       p.LastSend(),
		LastRecv:       p.LastRecv(),
		ConnTime:       p.timeConnected,
		Version:        protocolVersion,
		Inbound:        p.inbound,
		LastPingMicros: p.lastPingMicros,
		LastPingTime:   p.lastPingTime,
	}

	p.statsMtx.RUnlock()
	return statsSnap
}

// ID returns the peer id.
//
// This function is safe for concurrent access.
func (p *Peer) ID() uint64 {
	p.flagsMtx.Lock()
	id := p.id
	p.flagsMtx.Unlock()

	return id
}

// PID returns the peer public key id.
//
// This function is safe for concurrent access.
func (p *Peer) PID() PID {
	p.flagsMtx.Lock()
	pid := p.pid
	p.flagsMtx.Unlock()

	return pid
}

// PK returns the peer public key.
//
// This function is safe for concurrent access.
func (p *Peer) PK() *crypto.PublicKey {
	p.flagsMtx.Lock()
	pk := p.pk
	p.flagsMtx.Unlock()

	return pk
}

// NA returns the peer network address.
//
// This function is safe for concurrent access.
func (p *Peer) NA() *p2p.NetAddress {
	p.flagsMtx.Lock()
	na := p.na
	p.flagsMtx.Unlock()

	return na
}

// Addr returns the peer address.
//
// This function is safe for concurrent access.
func (p *Peer) Addr() string {
	// The address doesn't change after initialization, therefore it is not
	// protected by a mutex.
	return p.addr
}

// Inbound returns whether the peer is inbound.
//
// This function is safe for concurrent access.
func (p *Peer) Inbound() bool {
	return p.inbound
}

// Services returns the services flag of the remote peer.
//
// This function is safe for concurrent access.
func (p *Peer) Services() uint64 {
	p.flagsMtx.Lock()
	services := p.services
	p.flagsMtx.Unlock()

	return services
}

// LastPingTime returns the last ping time of the remote peer.
//
// This function is safe for concurrent access.
func (p *Peer) LastPingTime() time.Time {
	p.statsMtx.RLock()
	lastPingTime := p.lastPingTime
	p.statsMtx.RUnlock()

	return lastPingTime
}

// LastPingMicros returns the last ping micros of the remote peer.
//
// This function is safe for concurrent access.
func (p *Peer) LastPingMicros() int64 {
	p.statsMtx.RLock()
	lastPingMicros := p.lastPingMicros
	p.statsMtx.RUnlock()

	return lastPingMicros
}

// ProtocolVersion returns the negotiated peer protocol version.
//
// This function is safe for concurrent access.
func (p *Peer) ProtocolVersion() uint32 {
	p.flagsMtx.Lock()
	protocolVersion := p.protocolVersion
	p.flagsMtx.Unlock()

	return protocolVersion
}

// LastSend returns the last send time of the peer.
//
// This function is safe for concurrent access.
func (p *Peer) LastSend() time.Time {
	return time.Unix(atomic.LoadInt64(&p.lastSend), 0)
}

// LastRecv returns the last recv time of the peer.
//
// This function is safe for concurrent access.
func (p *Peer) LastRecv() time.Time {
	return time.Unix(atomic.LoadInt64(&p.lastRecv), 0)
}

// LocalAddr returns the local address of the connection.
//
// This function is safe fo concurrent access.
func (p *Peer) LocalAddr() net.Addr {
	var localAddr net.Addr
	if atomic.LoadInt32(&p.connected) != 0 {
		localAddr = p.conn.LocalAddr()
	}
	return localAddr
}

// TimeConnected returns the time at which the peer connected.
//
// This function is safe for concurrent access.
func (p *Peer) TimeConnected() time.Time {
	p.statsMtx.RLock()
	timeConnected := p.timeConnected
	p.statsMtx.RUnlock()

	return timeConnected
}

// handlePingMsg is invoked when a peer receives a ping message.
func (p *Peer) handlePingMsg(ping *msg.Ping) {
	p.SendMessage(msg.NewPong(p.cfg.PongNonce(p.pid)), nil)
}

// handlePongMsg is invoked when a peer receives a pong message.
func (p *Peer) handlePongMsg(pong *msg.Pong) {
	p.statsMtx.Lock()
	if p.lastPingNonce != 0 && pong.Nonce == p.lastPingNonce {
		p.lastPingMicros = time.Since(p.lastPingTime).Nanoseconds()
		p.lastPingMicros /= 1000 // convert to usec.
		p.lastPingNonce = 0
	}
	p.statsMtx.Unlock()
}

func (p *Peer) makeEmptyMessage(cmd string) (p2p.Message, error) {
	var message p2p.Message
	switch cmd {
	case msg.CmdVersion:
		message = &msg.Version{}

	case msg.CmdVerAck:
		message = &msg.VerAck{}

	case msg.CmdPing:
		message = &msg.Ping{}

	case msg.CmdPong:
		message = &msg.Pong{}

	default:
		return p.cfg.MakeEmptyMessage(cmd)
	}
	return message, nil
}

func (p *Peer) readMessage() (p2p.Message, error) {
	msg, err := p2p.ReadMessage(p.conn, p.cfg.Magic, p.makeEmptyMessage)
	// Use closures to log expensive operations so they are only run when
	// the logging level requires it.
	log.Debugf("%v", newLogClosure(func() string {
		if err != nil {
			return fmt.Sprintf("Read message failed, %s", err)
		}

		// Debug summary of message.
		summary := messageSummary(msg)
		if len(summary) > 0 {
			summary = " (" + summary + ")"
		}
		return fmt.Sprintf("Received %v%s from %s", msg.CMD(), summary, p)
	}))

	return msg, err
}

func (p *Peer) writeMessage(msg p2p.Message) error {
	// Don't do anything if we're disconnecting.
	if atomic.LoadInt32(&p.disconnect) != 0 {
		return nil
	}

	// Use closures to log expensive operations so they are only run when
	// the logging level requires it.
	log.Debugf("%v", newLogClosure(func() string {
		// Debug summary of message.
		summary := messageSummary(msg)
		if len(summary) > 0 {
			summary = " (" + summary + ")"
		}
		return fmt.Sprintf("Sending %v%s to %s", msg.CMD(), summary, p)
	}))

	// Handle write message timeout.
	doneChan := make(chan error)
	go func() {
		doneChan <- p2p.WriteMessage(p.conn, p.cfg.Magic, msg)
	}()

	select {
	case err := <-doneChan:
		return err
	case <-time.After(writeTimeout):
	}
	return fmt.Errorf("sending %s to %s timeout", msg.CMD(), p)
}

// shouldHandleIOError returns whether or not the passed error, which is
// expected to have come from reading or writing should be handled.
func (p *Peer) shouldHandleIOError(err error) bool {
	// No logging when the peer is being forcibly disconnected.
	if atomic.LoadInt32(&p.disconnect) != 0 {
		return false
	}

	// No logging when the remote peer has been disconnected.
	if err == io.EOF {
		return false
	}
	if opErr, ok := err.(*net.OpError); ok && !opErr.Temporary() {
		return false
	}

	return true
}

func (p *Peer) inHandler() {
	// The timer is stopped when a new message is received and reset after it
	// is processed.
	idleTimer := time.AfterFunc(idleTimeout, func() {
		log.Warnf("Peer %s no answer for %s -- disconnecting", p, idleTimeout)
		p.Disconnect()
	})

out:
	for atomic.LoadInt32(&p.disconnect) == 0 {
		// Read a message and stop the idle timer as soon as the read
		// is done.  The timer is reset below for the next iteration if
		// needed.
		rmsg, err := p.readMessage()
		idleTimer.Stop()
		if err != nil {
			// Only log the error and send reject message if the
			// local peer is not forcibly disconnecting and the
			// remote peer has not disconnected.
			if p.shouldHandleIOError(err) {
				errMsg := fmt.Sprintf("Can't read message from %s: %v", p, err)
				if err != io.ErrUnexpectedEOF {
					log.Errorf(errMsg)
				}

				// Push a reject message for the malformed message and wait for
				// the message to be sent before disconnecting.
				//
				// NOTE: Ideally this would include the command in the header if
				// at least that much of the message was valid, but that is not
				// currently exposed by wire, so just used malformed for the
				// command.
				rejectMsg := msg.NewReject("malformed", msg.RejectMalformed, errMsg)
				// Send the message and block until it has been sent before returning.
				doneChan := make(chan error, 1)
				p.SendMessage(rejectMsg, doneChan)
				<-doneChan
			}
			break out
		}
		atomic.StoreInt64(&p.lastRecv, time.Now().Unix())

		// Handle each supported message type.
		switch m := rmsg.(type) {
		case *msg.Version:

			rejectMsg := msg.NewReject(m.CMD(), msg.RejectDuplicate, "duplicate version message")
			// Send the message and block until it has been sent before returning.
			doneChan := make(chan error, 1)
			p.SendMessage(rejectMsg, doneChan)
			<-doneChan
			break out

		case *msg.VerAck:

			// No read lock is necessary because verAckReceived is not written
			// to in any other goroutine.
			log.Infof("Already received 'verack' from peer %v -- disconnecting", p)
			break out

		case *msg.Ping:
			p.handlePingMsg(m)

		case *msg.Pong:
			p.handlePongMsg(m)

		}

		// Call handle message which is configured on peer creation.
		p.handleMessage(p, rmsg)

		// A message was received so reset the idle timer.
		idleTimer.Reset(idleTimeout)
	}

	// Ensure the idle timer is stopped to avoid leaking the resource.
	idleTimer.Stop()

	// Ensure connection is closed.
	p.Disconnect()

	close(p.inQuit)
}

// outHandler handles all outgoing messages for the peer.  It must be run as a
// goroutine.  It uses a buffered channel to serialize output messages while
// allowing the sender to continue running asynchronously.
func (p *Peer) outHandler() {
out:
	for {
		select {
		case smsg := <-p.sendQueue:
			switch m := smsg.msg.(type) {
			case *msg.Ping:
				// Update last ping time
				p.statsMtx.Lock()
				p.lastPingNonce = m.Nonce
				p.lastPingTime = time.Now()
				p.statsMtx.Unlock()
			}

			err := p.writeMessage(smsg.msg)
			if err != nil {
				p.Disconnect()
				if p.shouldHandleIOError(err) {
					log.Errorf("Failed to send message to %s: %v", p, err)
				}
				if smsg.doneChan != nil {
					smsg.doneChan <- err
				}
				continue
			}

			// At this point, the message was successfully sent, so
			// update the last send time, signal the sender of the
			// message that it has been sent (if requested), and
			// signal the send queue to deliver the next queued message.
			atomic.StoreInt64(&p.lastSend, time.Now().Unix())
			if smsg.doneChan != nil {
				smsg.doneChan <- nil
			}

		case <-p.quit:
			break out
		}
	}

	// Drain any wait channels before going away so there is nothing left
	// waiting on this goroutine.
cleanup:
	for {
		select {
		case msg := <-p.sendQueue:
			if msg.doneChan != nil {
				msg.doneChan <- ErrPeerDisconnected
			}
			// no need to send on sendDoneQueue since queueHandler
			// has been waited on and already exited.
		default:
			break cleanup
		}
	}
	close(p.outQuit)
}

// pingHandler periodically pings the peer.  It must be run as a goroutine.
func (p *Peer) pingHandler() {
	pingTicker := time.NewTicker(p.cfg.PingInterval)
	defer pingTicker.Stop()

out:
	for {
		select {
		case <-pingTicker.C:
			p.SendMessage(msg.NewPing(p.cfg.PingNonce(p.pid)), nil)

		case <-p.quit:
			break out
		}
	}
}

func (p *Peer) SendMessage(msg p2p.Message, doneChan chan<- error) {
	// Avoid risk of deadlock if goroutine already exited.  The goroutine
	// we will be sending to hangs around until it knows for a fact that
	// it is marked as disconnected and *then* it drains the channels.
	if !p.Connected() || atomic.LoadInt32(&p.started) == 0 {
		if doneChan != nil {
			go func() {
				doneChan <- ErrPeerDisconnected
			}()
		}
		return
	}
	p.sendQueue <- outMsg{msg: msg, doneChan: doneChan}
}

// Connected returns whether or not the peer is currently connected.
//
// This function is safe for concurrent access.
func (p *Peer) Connected() bool {
	return atomic.LoadInt32(&p.connected) != 0 &&
		atomic.LoadInt32(&p.disconnect) == 0
}

// Disconnect disconnects the peer by closing the connection.  Calling this
// function when the peer is already disconnected or in the process of
// disconnecting will have no effect.
func (p *Peer) Disconnect() {
	if atomic.AddInt32(&p.disconnect, 1) != 1 {
		return
	}

	if atomic.LoadInt32(&p.connected) != 0 {
		p.conn.Close()
	}
	close(p.quit)
}

// InQuit returns the signal chan of message inHandler quit.
func (p *Peer) InQuit() <-chan struct{} {
	return p.inQuit
}

// OutQuit returns the signal chan of message outHandler quit.
func (p *Peer) OutQuit() <-chan struct{} {
	return p.outQuit
}

// Quit returns the signal chan of peer quit.
func (p *Peer) Quit() <-chan struct{} {
	return p.quit
}

// readRemoteVersionMsg waits for the next message to arrive from the remote
// peer.  If the next message is not a version message or the version is not
// acceptable then return an error.
func (p *Peer) readRemoteVersionMsg() ([]byte, error) {
	// Read their version message.
	remoteMsg, err := p.readMessage()
	if err != nil {
		return nil, err
	}

	verMsg, ok := remoteMsg.(*msg.Version)
	if !ok {
		reason := "A version message must precede all others"
		rejectMsg := msg.NewReject(remoteMsg.CMD(), msg.RejectMalformed, reason)
		p.writeMessage(rejectMsg)
		return nil, errors.New(reason)
	}

	// Detect self connections.
	if p.cfg.PID.Equal(verMsg.PID) {
		return nil, errors.New("disconnecting peer connected to self")
	}

	// Verify peer public key.
	pk, err := crypto.DecodePoint(verMsg.PID[:])
	if err != nil {
		return nil, errors.New("disconnecting peer invalided public key")
	}

	// Negotiate the protocol version and set the services to what the remote
	// peer advertised.
	p.flagsMtx.Lock()
	p.advertisedProtoVer = uint32(verMsg.Version)
	p.protocolVersion = minUint32(p.protocolVersion, p.advertisedProtoVer)
	log.Debugf("Negotiated protocol version %d for peer %s",
		p.protocolVersion, p)

	p.pk = pk
	p.pid = verMsg.PID
	p.services = verMsg.Services
	p.flagsMtx.Unlock()

	p.handleMessage(p, verMsg)
	return verMsg.Nonce[:], nil
}

// readRemoteVerAckMsg waits for the next message to arrive from the remote
// peer.  If the next message is not a verack message or the signature is not
// match return an error.
func (p *Peer) readRemoteVerAckMsg(nonce []byte) error {
	// Read their verack message.
	remoteMsg, err := p.readMessage()
	if err != nil {
		return err
	}

	verAck, ok := remoteMsg.(*msg.VerAck)
	if !ok {
		reason := "A verack message must after a version message"
		rejectMsg := msg.NewReject(remoteMsg.CMD(), msg.RejectMalformed, reason)
		p.writeMessage(rejectMsg)
		return errors.New(reason)
	}

	// Verify signature of the message nonce.
	p.handleMessage(p, verAck)
	return crypto.Verify(*p.pk, nonce, verAck.Signature[:])
}

// writeLocalVersionMsg writes our version message to the remote peer.
func (p *Peer) writeLocalVersionMsg() ([]byte, error) {
	// Create a 32 bytes nonce value
	nonce := [32]byte{}
	rand.Read(nonce[:])

	// Version message.
	localVerMsg := msg.NewVersion(p.cfg.ProtocolVersion, p.cfg.Services,
		p.cfg.Port, p.cfg.PID, nonce)

	return nonce[:], p.writeMessage(localVerMsg)
}

// writeLocalVerAckMsg writes our verack message to the remote peer.
func (p *Peer) writeLocalVerAckMsg(nonce []byte) error {
	localVarAck := msg.NewVerAck(p.cfg.Sign(nonce))
	return p.writeMessage(localVarAck)
}

// negotiateInboundProtocol waits to receive a version message from the peer
// then sends our version message. If the events do not occur in that order then
// it returns an error.
func (p *Peer) negotiateInboundProtocol() error {
	theirNonce, err := p.readRemoteVersionMsg()
	if err != nil {
		return err
	}

	ourNonce, err := p.writeLocalVersionMsg()
	if err != nil {
		return err
	}

	if err := p.writeLocalVerAckMsg(theirNonce); err != nil {
		return err
	}

	return p.readRemoteVerAckMsg(ourNonce)
}

// negotiateOutboundProtocol sends our version message then waits to receive a
// version message from the peer.  If the events do not occur in that order then
// it returns an error.
func (p *Peer) negotiateOutboundProtocol() error {
	ourNonce, err := p.writeLocalVersionMsg()
	if err != nil {
		return err
	}

	theirNonce, err := p.readRemoteVersionMsg()
	if err != nil {
		return err
	}

	if err := p.readRemoteVerAckMsg(ourNonce); err != nil {
		return err
	}

	return p.writeLocalVerAckMsg(theirNonce)
}

// start begins processing input and output messages.
func (p *Peer) start() error {
	negotiateErr := make(chan error, 1)
	go func() {
		if p.inbound {
			negotiateErr <- p.negotiateInboundProtocol()
		} else {
			negotiateErr <- p.negotiateOutboundProtocol()
		}
	}()

	// Negotiate the protocol within the specified negotiateTimeout.
	select {
	case err := <-negotiateErr:
		if err != nil {
			return err
		}
	case <-time.After(negotiateTimeout):
		return errors.New("protocol negotiation timeout")
	}
	log.Debugf("Connected to %s", p.Addr())

	// The protocol has been negotiated successfully so start processing input
	// and output messages.
	atomic.AddInt32(&p.started, 1)
	go p.inHandler()
	go p.outHandler()
	go p.pingHandler()

	return nil
}

// AssociateConnection associates the given conn to the peer.   Calling this
// function when the peer is already connected will have no effect.
func (p *Peer) AssociateConnection(conn net.Conn) {
	// Already connected?
	if !atomic.CompareAndSwapInt32(&p.connected, 0, 1) {
		return
	}

	p.conn = conn
	p.timeConnected = time.Now()

	if p.inbound {
		p.addr = p.conn.RemoteAddr().String()

		// Set up a NetAddress for the peer to be used with AddrManager.  We
		// only do this inbound because outbound set this up at connection time
		// and no point recomputing.
		na, err := newNetAddress(p.conn.RemoteAddr(), p.services)
		if err != nil {
			log.Errorf("Cannot create remote net address: %v", err)
			p.Disconnect()
			return
		}
		p.na = na
	}

	go func() {
		if err := p.start(); err != nil {
			log.Debugf("Cannot start peer %v: %v", p, err)
			p.Disconnect()
		}
	}()
}

// WaitForDisconnect waits until the peer has completely disconnected and all
// resources are cleaned up.  This will happen if either the local or remote
// side has been disconnected or the peer is forcibly disconnected via Disconnect.
func (p *Peer) WaitForDisconnect() {
	<-p.quit
}

// RandomUint64 creates a random uint64 number.
func RandomUint64() uint64 {
	var randNum uint64
	binary.Read(rand.Reader, binary.BigEndian, &randNum)
	return randNum
}

// newPeerBase returns a new base peer based on the inbound flag.  This
// is used by the NewInboundPeer and NewOutboundPeer functions to perform base
// setup needed by both types of peers.
func newPeerBase(origCfg *Config, inbound bool) *Peer {
	cfg := *origCfg // Copy to avoid mutating caller.

	p := Peer{
		id:              RandomUint64(),
		inbound:         inbound,
		sendQueue:       make(chan outMsg, 1), // nonblocking sync
		inQuit:          make(chan struct{}),
		outQuit:         make(chan struct{}),
		quit:            make(chan struct{}),
		cfg:             cfg, // Copy so caller can't mutate.
		services:        cfg.Services,
		protocolVersion: cfg.ProtocolVersion,
	}
	p.AddMessageFunc(cfg.MessageFunc)
	return &p
}

// NewInboundPeer returns a new inbound peer. Use Start to begin
// processing incoming and outgoing messages.
func NewInboundPeer(cfg *Config) *Peer {
	return newPeerBase(cfg, true)
}

// NewOutboundPeer returns a new outbound peer.
func NewOutboundPeer(cfg *Config, addr string) (*Peer, error) {
	p := newPeerBase(cfg, false)
	p.addr = addr

	host, portStr, err := net.SplitHostPort(addr)
	if err != nil {
		return nil, err
	}

	port, err := strconv.ParseUint(portStr, 10, 16)
	if err != nil {
		return nil, err
	}

	p.na = p2p.NewNetAddressIPPort(net.ParseIP(host), uint16(port),
		cfg.Services)

	return p, nil
}
