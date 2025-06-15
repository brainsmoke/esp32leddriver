
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import Checkers, AlienPlanet
from ani.shadowwalk import ShadowWalk
from ani.snake import Snake

def get_animations():
	return (
        ("Lorenz", Lorenz, {}),
        ("Rutherford", Rutherford, {}),
        ("Fire", Fire, {}),
        ("Gradient", Gradient, {}),
        ("Orbit", Orbit, {}),
        ("Snake", Snake, {'snake_len': 3, 'snake_count': 3, 'mirror_leds':True}),
        ("ShadowWalk", ShadowWalk, {}),
        ("Wobble", Wobble, {}),
        ("Checkers", Checkers, {}),
        ("AlienPlanet", AlienPlanet, {}),
        ("Spots", Spots, {}),
        ("Chroma", Chroma, {})
    )

