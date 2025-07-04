
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import WaveSelectGradient, Spiral, Wobble, ConfigMode
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
        ("Earth", Movie, {'filename' : 'earth.bin'}),
        ("ShadowWalk", ShadowWalk, {}),
        ("Fire", Fire, {}),
        ("Topo", Topo, {}),
        ("Spiral", Spiral, {}),
        ("Orbit", Orbit, {}),
        ("Wobble", Wobble, {}),
        ("AlienPlanet", AlienPlanet, {})
    )

def get_config_animation():
    return ("Config", ConfigMode, {})

