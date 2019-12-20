<?php
header("Content-Type: application/json");
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: POST");
header("Access-Control-Max-Age: 3600");
header("Access-Control-Allow-Headers: Content-Type, Access-Control-Allow-Headers");

$return = array();

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
  if ( !empty($_POST["API_key"]) && !empty($_POST["action"]) ) {
    if ($_POST["API_key"] == "a067db7c-bffd-4f26-8151-20a487679dc3") {
      try {
        #$output = shell_exec("python3 /var/www/PC_controller/PC_controller_transmitter.py " + $_POST["action"]);
        $output = shell_exec('python3 test.py');
        var_dump($output);
        $return["success"] = true;
      } catch (\Exception $e) {
        $return["error"] = "transmission issue";
        $return["exception"] = $e;
      }
    }
    else {
      $return["success"] = false;
      $return["error"] = "bad API key";
    }
  }
  else {
    $return["success"] = false;
    $return["error"] = "missing argument";
  }

}
else if ($_SERVER['REQUEST_METHOD'] === 'GET') {
  
}
else{
  $return["success"] = false;
  $return["error"] = "not supported method";
}

echo json_encode($return);
?>
