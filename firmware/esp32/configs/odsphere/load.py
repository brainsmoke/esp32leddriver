
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from conf.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import Checkers, AlienPlanet
from ani.multiwave import MultiWave
from ani.movie import Earth

def get_animations():
	return (Lorenz, Gradient, Earth, Fire, Spiral, MultiWave, Orbit, Wobble, Checkers, AlienPlanet, Spots, Chroma)

