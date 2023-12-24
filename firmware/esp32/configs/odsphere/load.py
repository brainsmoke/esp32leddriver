
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble, InsideWobble
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import Checkers, AlienPlanet
from ani.multiwave import MultiWave
from ani.movie import Earth

def get_animations():
	return (Lorenz, InsideWobble, Earth, Fire, Gradient, MultiWave, Orbit, Wobble, Checkers, AlienPlanet, Spots, Chroma)

