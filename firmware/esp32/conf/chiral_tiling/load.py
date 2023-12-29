
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.materials import AlienPlanet

def get_animations():
	return (Lorenz, Fire, Gradient, Orbit, Wobble, AlienPlanet, Spots, Chroma)

