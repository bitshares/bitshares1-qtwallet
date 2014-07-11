<//-- This is a proof of concept web application for BitShares logins. -->
<?PHP
    //CONFIG
    $RPC_SERVER_IP = "127.0.0.1";
    $RPC_SERVER_PORT = 50837;
    $RPC_SERVER_USER = "server";
    $RPC_SERVER_PASS = "server";
    $RPC_SERVER_WALLET = "server";
    $RPC_SERVER_WALLET_PASS = "iamtheserver";

    $BITSHARES_USER_NAME = "server.com";

    $LOGIN_PAGE_DOMAIN_AND_PATH = "localhost:8888/login.php";
    //END CONFIG

    $sock = socket_create(AF_INET, SOCK_STREAM, getprotobyname("tcp"));
    socket_connect($sock, $RPC_SERVER_IP, $RPC_SERVER_PORT);
    
    function rpc($method, $params = array())
    {
        global $sock;
        $json = json_encode(array('method'=>$method,'params'=>$params,'id'=>1));
        socket_write($sock, $json);
        $json = json_decode(socket_read($sock, 99999, PHP_NORMAL_READ), true);
        return $json["result"];
    }
    
    rpc("login", [$RPC_SERVER_USER, $RPC_SERVER_PASS]);
    rpc("open", [$RPC_SERVER_WALLET]);
    rpc("unlock", ["2", $RPC_SERVER_WALLET_PASS]);
    
    if(isset($_REQUEST["client_key"]))
    {
        $loginPackage = rpc("wallet_login_finish", [$_REQUEST["server_key"], $_REQUEST["client_key"], $_REQUEST["signed_secret"]]);
        if(!isset($loginPackage))
        {
            echo"Sorry, your login session has expired. Redirecting you to login again.<script type='text/javascript'>setTimeout(function(){location.href='$_SERVER[PHP_SELF]'}, 3000)</script>";
            socket_close($sock);
            return;
        }
        $userAccount = rpc("blockchain_get_account", [$loginPackage["user_account_key"]]);
        echo "Welcome, $_REQUEST[client_name]! You are now logged in.<br/>";
        if(!isset($userAccount))
            echo "You have not registered your name with BitShares X.<br/>";
        else {
            if(isset($userAccount["public_data"]["gravatarID"]))
                echo "<img src='http://www.gravatar.com/avatar/{$userAccount["public_data"]["gravatarID"]}' />";
            if(isset($userAccount["delegate_info"]))
                echo "<p>This is the VIP section, for delegates only.</p>";
        }
    } else {
        $url = rpc("wallet_login_start", [$BITSHARES_USER_NAME]);
        $url .= $LOGIN_PAGE_DOMAIN_AND_PATH;
        ?>
BitShares will now prompt you to complete the login in a new window. You may now close this window.
<script type="text/javascript">location.href="<?=$url?>";</script>
        <?PHP
    }

    socket_close($sock);
?>
