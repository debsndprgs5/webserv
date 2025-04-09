<?php
// Récupération des paramètres (GET ou POST)
$num = isset($_REQUEST['num']) ? $_REQUEST['num'] : null;
$animal = isset($_REQUEST['animal']) ? $_REQUEST['animal'] : null;

// Validation du paramètre 'num'
if (!is_numeric($num)) {
    echo "Erreur : Le paramètre 'num' doit être un nombre entre 1 et 5.";
    exit;
}
$num = intval($num);
if ($num < 1 || $num > 5) {
    echo "Erreur : Le paramètre 'num' doit être compris entre 1 et 5.";
    exit;
}

// Validation du paramètre 'animal'
$allowedAnimals = array("bober", "slimak", "ratatouille", "homik");
if (!in_array($animal, $allowedAnimals)) {
    echo "Erreur : Le paramètre 'animal' doit être 'bober', 'homik', 'slimak' ou 'ratatouille'.";
    exit;
}

// Utilisation d'un nowdoc pour intégrer l'ASCII art sans soucis d'échappement des quotes
$asciiArts = array(
    "bober" => <<<'EOD'
⠀⠀⢠⣿⣿⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣿⣿⡄⠀⠀
⠀⢠⣾⣿⣿⡿⠉⢿⣿⣿⣿⣿⣿⣿⡿⠉⢿⣿⣿⣷⡄⠀
⠀⠘⢿⣿⣿⣇⣠⣿⡿⠛⠉⠉⠛⢿⣿⣄⣸⣿⣿⡿⠃⠀
⠀⠀⣼⣿⣿⣿⣿⡋⠀⠀⠀⠀⠀⠀⢙⣿⣿⣿⣿⣧⠀⠀
⠀⣸⣿⣿⣿⣿⣿⣿⣾⣇⣀⣀⣸⣷⣿⣿⣿⣿⣿⣿⣇⠀
⣰⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣆
⣿⣿⣿⣿⣿⣿⠿⠟⠛⠉⢸⡇⠉⠛⠻⠿⣿⣿⣿⣿⣿⣿
⢻⣿⣿⣿⣿⣿⡇⠀⠀⠀⢸⡇⠀⠀⠀⢸⣿⣿⣿⣿⣿⡟
⠀⠻⣿⣿⣿⣿⣿⡀⠀⠀⢸⡇⠀⠀⢀⣿⣿⣿⣿⣿⠟⠀
⠀⠀⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⠀⠀
⠀⠀⠀⠀⠈⠙⠻⢿⣿⣿⣿⣿⣿⣿⡿⠟⠋⠁⠀⠀⠀⠀
EOD
,
    "slimak" => <<<'EOD'
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⡀⠀⠀⠀⠀⠀⠀⠀⣶⡆
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢧⡀⠀⠀⠀⠀⠀⢀⠏⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢻⢄⠀⠀⠀⢠⠎⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠑⡠⠀⡴⡡⠁⠀⠀
⠀⠀⠀⠀⠀⠀⠀⢀⣴⡎⠉⡩⠃⠈⠕⢦⡀⠀⠀⠀⠀⡉⠀⢉⠃⠀⠀⠀
⠀⠀⠀⠀⠀⠀⣴⣿⡉⠘⠻⣤⠔⠊⠀⢀⠋⢆⠀⢀⣾⡏⠈⢺⡆⠀⠀⠀
⠀⠀⠀⠀⠀⢸⢿⠇⠞⠉⠀⢹⣄⡠⠖⠁⣠⣾⠟⡱⣻⠂⠀⣸⠀⠀⠀⠀
⠀⠀⠀⠀⠀⢸⣳⢴⠋⠢⡄⠘⠁⣠⣴⠞⠛⠁⢈⡽⠁⠀⣠⠃⠀⠀⠀⠀
⠀⣀⢠⣀⡂⠉⠿⣀⡆⠀⠸⣬⢿⠋⣁⣀⣈⡴⠋⠀⠠⠊⠁⠀⠀⠀⠀⠀
⠙⠉⠉⠙⠛⠛⠛⠋⠙⠛⠛⠛⠉⠉⠛⠛⠁⠈⠛⠃⠈⠀⠀⠀⠀⠀⠀⠀
EOD
,
    "ratatouille" => <<<'EOD'
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⣶⣶⣦⡴⢶⣶⣶⣆⠸⠿⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⣻⣿⣷⣾⣿⣿⡿⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⣿⣿⣿⣿⣤⠄⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣶⣾⣿⣿⣿⣿⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣾⣿⣿⣿⣿⣿⠿⣿⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⢠⣾⣿⣿⣿⣿⣿⣿⣿⠀⣿⣿⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⣾⣿⣟⠛⠻⣿⣿⣿⣿⣦⡈⠉⠛⠻⡆⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⣿⣦⡈⢻⣿⣿⣿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⢿⣿⣿⣿⣿⣧⠈⣿⣿⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⢀⣀⡀⠸⣿⣿⣿⣿⣟⣀⣉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⣾⡿⠟⠛⠀⠉⠛⠛⠛⠛⠛⠛⠛⠛⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⢿⣧⣀⠀⠀⠀⠀⢀⣀⣠⣤⠶⠶⠶⠶⢶⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠈⠉⠛⠛⠛⠛⠉⠉⠀⠀⠀⠀⠀⣀⡼⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
EOD
,
    "homik" => <<<'EOD'
        _                       
      (`-`;-"```"-;`-`)         
       \.'         './          
       /             \          
       ;   0     0   ;          
      /| =         = |\         
     ; \   '._Y_.'   / ;        
    ;   `-._ \|/ _.-'   ;       
   ;        `"""`        ;      
   ;    `""-.   .-""`    ;      
   /;  '--._ \ / _.--   ;\      
  :  `.   `/|| ||\`   .'  :     
   '.  '-._       _.-'   .'       
   (((-'`  `"""""`   `'-)))     

EOD
);

for ($i = 0; $i < $num; $i++) {
    echo $asciiArts[$animal] . "\n";
}
?>
