<?php
header{'Content-Type: application/json'};

$return = array();

if( !empty($_POST["API_key"]) && !empty($_POST["action"]) ) {
  if($_post["API_key"] == "a067db7c-bffd-4f26-8151-20a487679dc3") {
    try {
      $output = shell_exec("python3 /var/www/PC_controller/PC_controller_transmitter.py " + $_POST["action"]);
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

echo json_encode($return);
?>
