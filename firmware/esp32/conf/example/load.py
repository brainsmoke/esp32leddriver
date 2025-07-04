
from ani.orbit import Orbit
from ani.lorenz import Lorenz
from ani.fire import Fire
from ani.gradient import Gradient, Spiral, Wobble, ConfigMode
from ani.spot import Spots, Chroma
from ani.rutherford import Rutherford
from ani.materials import Checkers, AlienPlanet
from ani.shadowwalk import ShadowWalk

def get_animations():
    return (
        ("Lorenz", Lorenz, {}),
        ("Rutherford", Rutherford, {}),
        ("Fire", Fire, {}),
        ("Gradient", Gradient, {}),
        ("Orbit", Orbit, {}),
        ("ShadowWalk", ShadowWalk, {}),
        ("Wobble", Wobble, {}),
        ("Checkers", Checkers, {}),
        ("AlienPlanet", AlienPlanet, {}),
        ("Spots", Spots, {}),
        ("Chroma", Chroma, {})
    )

def get_config_animation():
    return ("Config", ConfigMode, {})

