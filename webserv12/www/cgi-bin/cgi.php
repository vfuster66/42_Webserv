<?php
// Vérifier si le formulaire a été soumis
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // Récupérer les données du formulaire
    $prenom = htmlspecialchars($_POST["prenom"]);
    $nom = htmlspecialchars($_POST["nom"]);
    $telephone = htmlspecialchars($_POST["telephone"]);
    $email = htmlspecialchars($_POST["email"]);

    // Formatage des données à enregistrer dans le fichier texte
    $donnees = "Prénom : $prenom\nNom : $nom\nTéléphone : $telephone\nEmail : $email\n\n";

    // Chemin vers le fichier texte où enregistrer les données
    $fichier = "cgi_php.txt";

    // Ouvrir le fichier en mode écriture (ajout en fin de fichier)
    $fichier_handle = fopen($fichier, 'a');

    // Vérifier si l'ouverture du fichier a réussi
    if ($fichier_handle === false) {
        die("Impossible d'ouvrir le fichier pour écriture.");
    }

    // Écrire les données dans le fichier
    if (fwrite($fichier_handle, $donnees) === false) {
        die("Impossible d'écrire dans le fichier.");
    }

    // Fermer le fichier
    fclose($fichier_handle);

    // Redirection vers une page de confirmation
    header("Location: confirmation.html");
    exit();
} else {
    // Redirection vers une page d'erreur si le formulaire n'a pas été soumis correctement
    header("Location: erreur.html");
    exit();
}
?>




