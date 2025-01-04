
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.materials import AlienPlanet
from ani.snake import Snake

def get_animations():
	return ((Lorenz, {}), (Fire, {}), (Snake, {'snake_len': 20, 'snake_count': 12, 'max_speed': 1600}), (Gradient, {}), (Orbit, {}), (Wobble, {}), (AlienPlanet, {}), (Spots, {}), (Chroma, {}))

