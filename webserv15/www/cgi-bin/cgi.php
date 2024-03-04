#!/usr/bin/php
<?php
echo "Content-Type: text/html; charset=utf-8\n\n";

// Fonction pour obtenir l'heure actuelle dans un fuseau horaire donné
function get_heure($timezone) {
    date_default_timezone_set($timezone);
    return date("H:i:s");
}

// Tableau associatif des campus de l'école 42 et de leurs fuseaux horaires
$campus_42 = array(
    "Australie" => array(
        "42 Adelaide" => "Australia/Adelaide"
    ),
    "Angola" => array(
        "42 Luanda" => "Africa/Luanda"
    ),
    "Arménie" => array(
        "42 Yerevan" => "Asia/Yerevan"
    ),
    "Autriche" => array(
        "42 Vienna" => "Europe/Vienna"
    ),
    "Belgique" => array(
        "19 – Anvers" => "Europe/Brussels",
        "19 – Bruxelles" => "Europe/Brussels"
    ),
    "Brésil" => array(
        "42 Belo Horizonte" => "America/Sao_Paulo",
        "42 Rio" => "America/Sao_Paulo",
        "42 São Paulo" => "America/Sao_Paulo"
    ),
    "Canada" => array(
        "42 Québec" => "America/Toronto"
    ),
    "Corée du Sud" => array(
        "42 Gyeongsan" => "Asia/Seoul",
        "42 Séoul" => "Asia/Seoul"
    ),
    "Espagne" => array(
        "42 Barcelona" => "Europe/Madrid",
        "42 Madrid" => "Europe/Madrid",
        "42 Málaga" => "Europe/Madrid",
        "42 Urduliz" => "Europe/Madrid"
    ),
    "Émirats Arabes Unis" => array(
        "42 Abu Dhabi" => "Asia/Dubai"
    ),
    "Finlande" => array(
        "Hive – Helsinki" => "Europe/Helsinki"
    ),
    "France" => array(
        "42 Angoulême" => "Europe/Paris",
        "42 Le Havre" => "Europe/Paris",
        "42 Lyon" => "Europe/Paris",
        "42 Mulhouse" => "Europe/Paris",
        "42 Nice" => "Europe/Paris",
        "42 Paris" => "Europe/Paris",
        "42 Perpignan" => "Europe/Paris"
    ),
    "Italie" => array(
        "42 Firenze" => "Europe/Rome",
        "42 Milano" => "Europe/Rome",
        "42 Roma" => "Europe/Rome"
    ),
    "Japon" => array(
        "42 Tokyo" => "Asia/Tokyo"
    ),
    "Jordanie" => array(
        "42 Amman" => "Asia/Amman"
    ),
    "Liban" => array(
        "42 Beirut" => "Asia/Beirut"
    ),
    "Luxembourg" => array(
        "42 Luxembourg" => "Europe/Luxembourg"
    ),
    "Madagascar" => array(
        "42 Antananarivo" => "Indian/Antananarivo"
    ),
    "Malaisie" => array(
        "42 Iskandar Puteri" => "Asia/Kuala_Lumpur",
        "42 Kuala Lumpur" => "Asia/Kuala_Lumpur",
        "42 Penang" => "Asia/Kuala_Lumpur"
    ),
    "Maroc" => array(
        "1337 – Ben Guérir" => "Africa/Casablanca",
        "1337 – Khouribga" => "Africa/Casablanca",
        "1337 – Med" => "Africa/Casablanca"
    ),
    "Palestine" => array(
        "42 Nablus" => "Asia/Hebron"
    ),
    "Pays-Bas" => array(
        "Codam – Amsterdam" => "Europe/Amsterdam"
    ),
    "Pologne" => array(
        "42 Warsaw" => "Europe/Warsaw"
    ),
    "Portugal" => array(
        "42 Lisboa" => "Europe/Lisbon",
        "42 Porto" => "Europe/Lisbon"
    ),
    "Royaume-Uni" => array(
        "42 London" => "Europe/London"
    ),
    "Singapour" => array(
        "42 Singapore" => "Asia/Singapore"
    ),
    "Suisse" => array(
        "42 Lausanne" => "Europe/Zurich"
    ),
    "Tchéquie" => array(
        "42 Prague" => "Europe/Prague"
    ),
    "Thaïlande" => array(
        "42 Bangkok" => "Asia/Bangkok"
    ),
    "Turquie" => array(
        "42 Istanbul" => "Europe/Istanbul"
    )
);

// Affichage de l'heure pour chaque campus
echo "<h1>Horloge des campus 42 dans le monde</h1>";
foreach ($campus_42 as $pays => $campus) {
    echo "<h2>$pays</h2>";
    ksort($campus); // Trie les campus par ordre alphabétique
    foreach ($campus as $nom => $timezone) {
        $heure = get_heure($timezone);
        echo "<p>$nom : $heure</p>";
    }
}
?>
