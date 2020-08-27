
# How to build trinity

## Get the code
### Download Trinity repo using Git:

`git clone https://github.com/elastos/Elastos.Trinity.git`

### Update code
`cd Elastos.Trinity`

`./synccode.sh`


## Build Trinity

`./ToolChains/bin/build all`

### Platform
the default platform is android, you can build for ios with:
`./ToolChains/bin/build all -pf ios`

or for all:
`./ToolChains/bin/build all -pf all`

# The other build option
## For Runtime developer 
the `build all` command will build all the dapps, so this will spend more time.

If you don't change the dapp, you should use:
`./ToolChains/bin/build runtime`

## For plugin developer
`./ToolChains/bin/build plugin -p Plugin/xxx`

## Only build dapp
`./ToolChains/bin/build dapp`

or

`./ToolChains/bin/build dapp -p Dapps/Ionic/DID`

## Build partner apps
`./ToolChains/bin/build partners`

or

`./ToolChains/bin/build partner -p dappname`
