local m = require("api")

-- client: path, password, if create
local wallet = client.new("keystore.dat", "elastos", false)

-- account
local addr = wallet:get_address()
local pubkey = wallet:get_publickey()
print(addr)
print(pubkey)

-- asset_id
local asset_id = m.get_asset_id()

-- amount, fee
local amount = 5000
local fee = 0.001

-- deposit params
local deposit_address = "DbAbPQCiY9RYycme9LCduVDECdCCiwj2Pz"
local own_publickey = "03aa307d123cf3f181e5b9cc2839c4860a27caf5fb329ccde2877c556881451007"
local node_publickey = "021cfade3eddd057d8ca178057a88c4654b15c1ada7ee9ab65517f00beb6977556"
local nick_name = "Noderators"
local url = "www.noderators.org"
local location = "112211"
local host_address = "127.0.0.1"

-- register producer payload: publickey, nickname, url, local, host, wallet
local rp_payload = registerproducer.new(own_publickey, node_publickey, nick_name, url, location, host_address, wallet)
print(rp_payload:get())

-- transaction: version, txType, payloadVersion, payload, locktime
local tx = transaction.new(9, 0x09, 0, rp_payload, 0)
print(tx:get())

-- input: from, amount + fee
local charge = tx:appendenough(addr, (amount + fee) * 100000000)
print(charge)

-- outputpayload
local default_output = defaultoutput.new()

-- output: asset_id, value, recipient, output_paload_type, outputpaload
local charge_output = output.new(asset_id, charge, addr, 0, default_output)
local amount_output = output.new(asset_id, amount * 100000000, deposit_address, 0, default_output)
tx:appendtxout(charge_output)
tx:appendtxout(amount_output)
-- print(charge_output:get())
-- print(amount_output:get())

-- sign
tx:sign(wallet)
print(tx:get())

-- send
local hash = tx:hash()
local res = m.send_tx(tx)

print("sending " .. hash)

if (res ~= hash)
then
    print(res)
else
    print("tx send success")
end
