
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.materials import AlienPlanet
from ani.multiwave import MultiWave
from ani.topo import Topo
from ani.snake import Snake

def get_animations():
	return (MultiWave, Snake, Lorenz, Gradient, Fire, Topo, Spiral, Orbit, Wobble, AlienPlanet)

