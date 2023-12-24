
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import Checkers, AlienPlanet

def get_animations():
	return (Lorenz, Rutherford, Fire, Gradient, Orbit, Wobble, Checkers, AlienPlanet, Spots, Chroma)

