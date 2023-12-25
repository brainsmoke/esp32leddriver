
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from conf.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import AlienPlanet
from ani.multiwave import MultiWave
from ani.movie import Earth
from ani.shadowplay import ShadowPlay
from ani.shadowwalk import ShadowWalk

def get_animations():
	return (MultiWave, Lorenz, ShadowPlay, Gradient, Earth, ShadowWalk, Fire, Spiral, Orbit, Wobble, AlienPlanet, Spots, Chroma)

