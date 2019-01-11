"use strict";

const common = require("./common");

module.exports = async function(json_data, res) {
    try {
        console.log("Getting Sidechain Logs At Block Height: ");
        let blkheight = json_data["params"]["height"];
        console.log(blkheight);
        console.log("============================================================");
        let logs = await common.contract.getPastEvents(common.payloadReceived.name, {fromBlock: blkheight, toBlock: blkheight});
        let result = new Array();
        let txhash = null;
        let txlog = null;
        let outputindex = 0;
        let costamount = 0;
        for (const log of logs) {
            if (txhash === null || txhash != log["transactionHash"]) {
                txhash = log["transactionHash"];
                let txinfo = await common.web3.eth.getTransaction(txhash);
                let txreceipt = await common.web3.eth.getTransactionReceipt(txhash)
                costamount = txreceipt.gasUsed * txinfo.gasPrice
                txlog = {"txid": txhash.slice(2)};
                result.push(txlog);
                txlog["crosschainassets"] = new Array();
            }

            let crosschainamount = String((log["returnValues"]["_amount"]- costamount) / 1e18);
            let outputamount = String(log["returnValues"]["_amount"] / 1e18);


            crosschainamount = crosschainamount.substring(0,crosschainamount.lastIndexOf('.')+9);
            outputamount = outputamount.substring(0,outputamount.lastIndexOf('.')+9);


            txlog["crosschainassets"].push({
                "crosschainaddress": log["returnValues"]["_addr"],
                "crosschainamount": crosschainamount,
                "outputamount":outputamount
            });
            outputindex++;
        }
        res.json({"result": result, "id": null, "error": null, "jsonrpc": "2.0"});
        return;
    } catch (err) {
        common.reterr(err, res);
        return;
    }
}
