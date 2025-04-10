<?php
$now = new DateTime();
if (isset($_GET['link'])) {
	$zone = $_GET["zone"];
	$hours = (int)$zone + 6;
} else {
	$hours = 2;
}
echo $now->modify($hours . " hour")->format('Y-m-d H:i:s');
$utc = (int)$hours - 6;
echo sprintf(" GMT %+03d:00", $utc);
?>