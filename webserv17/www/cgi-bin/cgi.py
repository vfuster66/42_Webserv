#!/usr/bin/python3
print("Content-Type: text/html; charset=utf-8\n\n")

import random

# Liste de citations
citations = [
    "La vie est soit une aventure audacieuse, soit rien du tout. - Helen Keller",
    "Le succès, c'est tomber sept fois et se relever huit. - Proverbe japonais",
    "La seule façon de faire du bon travail est d'aimer ce que vous faites. - Steve Jobs",
    "La vie n'est pas d'attendre que l'orage passe, c'est d'apprendre à danser sous la pluie. - Sénèque",
    "Ne rêvez pas votre vie, vivez vos rêves. - Antoine de Saint-Exupéry",
    "La folie, c'est se comporter de la même manière et s'attendre à un résultat différent. - Albert Einstein",
    "Le bonheur n'est pas quelque chose que l'on possède, c'est quelque chose que l'on est. - John Lennon"
]

# Fonction pour choisir une citation aléatoire
def choisir_citation():
    return random.choice(citations)

# Afficher une citation aléatoire
print("Voici une citation pour vous :\n")
print(choisir_citation())





