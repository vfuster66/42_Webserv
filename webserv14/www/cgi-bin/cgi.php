#!/usr/bin/php

<?php
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Assurez-vous que le champ 'message' est bien envoyé
    if (!empty($_POST['message'])) {
        $message = htmlspecialchars($_POST['message']);
        echo "Message reçu : " . $message;
    } else {
        echo "Aucun message reçu.";
    }
} else {
    // Si le fichier est accédé directement ou non via un POST, affichez un message d'erreur simple.
    echo "Ce script doit être appelé via un formulaire.";
}
?>




