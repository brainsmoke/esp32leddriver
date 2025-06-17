
from ani.monofire import Fire2D
from ani.monosnake import Snake
from ani.monogradient import Gradient, WaveSelectGradient
from ani.movie import Movie

def get_animations():
    return (
        ("Tunnel", WaveSelectGradient, {'map_file':'tunnel.json'}),
        ("Fire2D", Fire2D, {'dim': (16, 16) }),
        ("Roll", Movie, {'filename':'rr.bin', 'fps':25}),
        ("Spiral", WaveSelectGradient, {'map_file':'gradient.json'}),
        ("Snake", Snake, {'snake_len': 7, 'snake_count': 2, 'max_speed': 600}),
        ("hallway", WaveSelectGradient, {'map_file':'hallway.json'}),
        ("Moire", Gradient, {'map_file':'moire.json'}),
    )

