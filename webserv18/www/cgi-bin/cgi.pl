#!/usr/bin/perl

use strict;
use warnings;
use List::Util 'shuffle';

print "Content-Type: text/html\n\n";

# Définir la longueur du mot de passe
my $longueur_mot_de_passe = 20;

# Définir les caractères utilisés pour générer le mot de passe
my @lettres_minuscules = ('a'..'z');
my @lettres_majuscules = ('A'..'Z');
my @chiffres = ('0'..'9');
my @caracteres_speciaux = ('!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '=', '-', '[', ']', '{', '}', ';', ':', ',', '.', '/');
my @tous_les_caracteres = (@lettres_minuscules, @lettres_majuscules, @chiffres, @caracteres_speciaux);

# Générer le mot de passe en mélangeant les caractères
my @mot_de_passe = (shuffle(@tous_les_caracteres))[0 .. $longueur_mot_de_passe - 1];

# Afficher le mot de passe généré
print "<html><head><title>Générateur de Mot de Passe</title><meta charset='utf-8'></head><body>";
print "<h1>Mot de passe généré</h1>";
print "<p>Mot de passe généré : ", join('', @mot_de_passe), "</p>";
print "</body></html>";




