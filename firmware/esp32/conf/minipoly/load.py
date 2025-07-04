
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import WaveSelectGradient, Spiral, Wobble, ConfigMode
from ani.spot import Spots, Chroma
from ani.materials import AlienPlanet
from ani.multiwave import MultiWave
from ani.topo import Topo
from ani.snake import Snake

def get_animations():
	return (
        ("Fire", Fire, {}),
        ("Wobble", Wobble, {}),
        ("MultiWave", MultiWave, {}),
        ("Snake", Snake, {}),
        ("Lorenz", Lorenz, {}),
        ("WaveSelectGradient", WaveSelectGradient, {}),
        ("Topo", Topo, {}),
        ("Spiral", Spiral, {}),
        ("Orbit", Orbit, {}),
        ("AlienPlanet", AlienPlanet, {})
    )

def get_config_animation():
    return ("Config", ConfigMode, {})

