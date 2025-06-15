
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from conf.fire import Fire
from ani.gradient import WaveSelectGradient, Spiral, Wobble
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import AlienPlanet
from ani.multiwave import MultiWave
from ani.movie import Earth
from ani.shadowplay import ShadowPlay
from ani.shadowwalk import ShadowWalk
from ani.topo import Topo
from ani.snake import Snake

def get_animations():
	return (
        ("MultiWave", MultiWave, {}),
        ("Snake", Snake, {}),
        ("Lorenz", Lorenz, {}),
        ("ShadowPlay", ShadowPlay, {}),
        ("WaveSelectGradient", WaveSelectGradient, {}),
        ("Earth", Earth, {}),
        ("ShadowWalk", ShadowWalk, {}),
        ("Fire", Fire, {}),
        ("Topo", Topo, {}),
        ("Spiral", Spiral, {}),
        ("Orbit", Orbit, {}),
        ("Wobble", Wobble, {}),
        ("AlienPlanet", AlienPlanet, {})
    )

