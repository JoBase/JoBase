import pathlib

path = pathlib.Path(__file__).parent.absolute()

images = ['man', 'bad', 'rocket', 'enemy', 'astronaut']
sounds = ['blast']

for image in images:
    globals()[image.upper()] = pathlib.Path(path / ('images/' + image + '.png'))
    
for sound in sounds:
    globals()[sound.upper()] = pathlib.Path(path / ('sounds/' + sound + '.wav'))