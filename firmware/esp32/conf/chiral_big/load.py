
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import WaveSelectGradient, Gradient, Spiral, Wobble, ConfigMode
from ani.spot import Spots, Chroma
from ani.materials import AlienPlanet
from ani.snake import Snake

def get_animations():
	return (
        ("Lorenz", (Lorenz, {}),
        ("Fire", Fire, {}),
        ("Snake", Snake, {'snake_len': 20, 'snake_count': 12, 'max_speed': 1600}),
        ("WaveSelectGradient", WaveSelectGradient, {}),
        ("Spiral", Spiral, {}),
        ("Orbit", Orbit, {}),
        ("Wobble", Wobble, {}),
        ("AlienPlanet", AlienPlanet, {}),
        ("Spots", Spots, {}),
        ("Chroma", Chroma, {})
    )

def get_config_animation():
    return ("Config", ConfigMode, {})

