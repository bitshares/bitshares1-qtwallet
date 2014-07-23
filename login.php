<//-- This is a proof of concept web application for BitShares logins. -->
<?PHP
    //CONFIG
    $RPC_SERVER_USER = "a";
    $RPC_SERVER_PASS = "a";
    $RPC_SERVER_ADDRESS = "localhost";
    $RPC_SERVER_PORT = 13175;
    $RPC_SERVER_PATH = "rpc";
    $RPC_SERVER_WALLET = "server";
    $RPC_SERVER_WALLET_PASS = "iamtheserver";

    $BITSHARES_USER_NAME = "test-server";

    $LOGIN_PAGE_DOMAIN_AND_PATH = "localhost:8888/login.php";
    //END CONFIG

    // Using EasyBitcoin RPC client from https://github.com/aceat64/EasyBitcoin-PHP
    require_once "includes/easybitcoin.php";
    $bitshares = new Bitcoin($RPC_SERVER_USER, $RPC_SERVER_PASS, $RPC_SERVER_ADDRESS, $RPC_SERVER_PORT, $RPC_SERVER_PATH);

    $bitshares->open($RPC_SERVER_WALLET);
    $bitshares->unlock(2, $RPC_SERVER_WALLET_PASS);   
    
    if(isset($_REQUEST["client_key"]))
    {
        $loginPackage = $bitshares->wallet_login_finish($_REQUEST["server_key"], $_REQUEST["client_key"], $_REQUEST["signed_secret"]);
        if($loginPackage == false)
        {
            echo"Sorry, your login session has expired. Redirecting you to login again.<script type='text/javascript'>setTimeout(function(){location.href='$_SERVER[PHP_SELF]'}, 3000)</script>";
            return;
        }
        $userAccount = $bitshares->blockchain_get_account($loginPackage["user_account_key"]);
        echo "Welcome, $_REQUEST[client_name]! You are now logged in.<br/>";
        if(isset($userAccount["public_data"]["gravatarID"]))
            echo "<img src='http://www.gravatar.com/avatar/{$userAccount["public_data"]["gravatarID"]}' />";
        if(isset($userAccount["delegate_info"]))
            echo "<p>This is the VIP section, for delegates only.</p>";
    } else {
        $url = $bitshares->wallet_login_start($BITSHARES_USER_NAME);
        $url .= $LOGIN_PAGE_DOMAIN_AND_PATH;
        ?>
BitShares will now prompt you to complete the login in a new window. You may now close this window.
<script type="text/javascript">location.href="<?=$url?>";</script>
        <?PHP
    }

?>
