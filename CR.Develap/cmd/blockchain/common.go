/*
Copyright © 2019 Cyber Republic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
package blockchain

import (
	"os"

	"github.com/docker/go-connections/nat"
)

type DockerPort struct {
	ContainerRESTPort string
	ContainerRPCPort  string
	HostRESTPort      string
	HostRPCPort       string
}

type DockerContainerDataDir struct {
	HostCreate    bool
	ContainerPath string
}

type DockerContainer struct {
	ContainerName         string
	ImageName             string
	Volumes               map[string]DockerContainerDataDir
	ContainerExposedPorts nat.PortSet
	HostPortMappings      nat.PortMap
	Networks              []string
}

var (
	Env                string
	Nodes              string
	CurrentDir         = getCurrentDir()
	NodeDockerImageMap = map[string]string{
		"mainchain":         "cyberrepublic/elastos-mainchain-node:v0.3.7",
		"did":               "cyberrepublic/elastos-sidechain-did-node:v0.1.3",
		"token":             "cyberrepublic/elastos-sidechain-token-node:v0.1.2",
		"eth":               "cyberrepublic/elastos-sidechain-eth-node:dev",
		"eth.oracle":        "cyberrepublic/elastos-sidechain-eth-oracle:v0.0.1",
		"arbitrator":        "cyberrepublic/elastos-arbitrator-node:v0.1.2",
		"service.wallet":    "cyberrepublic/elastos-wallet-service:latest",
		"service.sidechain": "cyberrepublic/elastos-sidechain-service:latest",
	}
	NodeDockerDataPathMap = map[string]string{
		"mainchain":  "/ela/elastos",
		"did":        "/did/elastos_did",
		"token":      "/token/elastos_token",
		"eth":        "/eth/elastos_eth",
		"arbitrator": "/arbiter/elastos_arbiter",
	}
	NodeDockerKeystorePathMap = map[string]string{
		"mainchain":  "/ela/keystore.dat",
		"arbitrator": "/arbiter/elastos_arbiter",
	}
	NodeDockerConfigPathMap = map[string]string{
		"mainchain":  "/ela/config.json",
		"did":        "/did/config.json",
		"token":      "/token/config.json",
		"arbitrator": "/arbiter/config.json",
	}
	NodeDockerPortMainChain = map[string]DockerPort{
		"mainnet":  DockerPort{"20334", "20336", "20334", "20336"},
		"testnet":  DockerPort{"21334", "21336", "21334", "21336"},
		"localnet": DockerPort{"20334", "20336", "22334", "22336"},
	}
	NodeDockerPortSidechainDID = map[string]DockerPort{
		"mainnet":  DockerPort{"20604", "20606", "20604", "20606"},
		"testnet":  DockerPort{"21604", "21606", "21604", "21606"},
		"localnet": DockerPort{"20604", "20606", "22604", "22606"},
	}
	NodeDockerPortSidechainToken = map[string]DockerPort{
		"mainnet":  DockerPort{"20614", "20616", "20614", "20616"},
		"testnet":  DockerPort{"21614", "21616", "21614", "21616"},
		"localnet": DockerPort{"20614", "20616", "22614", "22616"},
	}
	NodeDockerPortSidechainEth = map[string]DockerPort{
		"mainnet":  DockerPort{ContainerRPCPort: "20636", HostRPCPort: "20636"},
		"testnet":  DockerPort{ContainerRPCPort: "20636", HostRPCPort: "21636"},
		"localnet": DockerPort{ContainerRPCPort: "20636", HostRPCPort: "22636"},
	}
)

func getCurrentDir() string {
	var currentDir string
	if pwd, err := os.Getwd(); err == nil {
		currentDir = pwd
	}
	return currentDir
}
