#include "main.h"

static void callback(Sound *self, MIX_Track *track, const SDL_AudioSpec *spec, float *pcm, int samples) {
    self -> pcm = pcm;
    self -> samples = samples;
    self -> channels = spec -> channels;
}

static int load(Sound *self, const char *name) {
    for (Audio *this = audio; this; this = this -> next)
        if (!strcmp(this -> name, name))
            return self -> src = this, 0;

    Audio *sound = malloc(sizeof(Audio));

    if (sound) {
        if ((sound -> name = strdup(name))) {
            sound -> next = audio;
            audio = self -> src = sound;
            sound -> src = MIX_LoadAudio(mixer, name, true);

            return sound -> src && MIX_SetTrackAudio(self -> track, audio -> src) &&
                MIX_SetTrackCookedCallback(self -> track, (MIX_TrackMixCallback) callback, self) ? 0 :
                (PyErr_SetString(error, SDL_GetError()), -1);
        }

        free(sound);
    }

    return PyErr_NoMemory(), -1;
}

// static PyObject *sound_get_samples(Sound *self, void *closure) {
//     // if (self -> pcm) {
//     const int len = self -> samples / self -> channels;
//     PyObject *channels = PyTuple_New(self -> channels);

//     if (!channels)
//         return NULL;

//     for (int i = 0; i < self -> channels; i ++) {
//         PyObject *tuple = PyTuple_New(len);

//         if (!tuple) {
//             Py_DECREF(channels);
//             return NULL;
//         }

//         for (int j = 0; j < len; j ++) {
//             PyObject *value = PyFloat_FromDouble(self -> pcm[j * self -> channels + i]);

//             if (!value) {
//                 Py_DECREF(tuple);
//                 Py_DECREF(channels);

//                 return NULL;
//             }

//             PyTuple_SET_ITEM(tuple, j, value);
//         }

//         PyTuple_SET_ITEM(channels, i, tuple);
//     }

//     return channels;
//     // }

//     // return PyTuple_New(0);
// }

static PyObject *sound_get_samples(Sound *self, void *closure) {
    const int len = self -> channels ? self -> samples / self -> channels : 0;
    PyObject *tuple = PyTuple_New(len);

    if (!tuple)
        return NULL;

    for (int i = 0; i < len; i ++) {
        PyObject *value = PyFloat_FromDouble(self -> pcm[i * self -> channels]);

        if (!value) {
            Py_DECREF(tuple);
            return NULL;
        }

        PyTuple_SET_ITEM(tuple, i, value);
    }

    return tuple;
}

static PyObject *sound_get_amp(Sound *self, void *closure) {
    double sum = 0;

    for (int i = 0; i < self -> samples; i ++)
        sum += self -> pcm[i] * self -> pcm[i];

    return PyFloat_FromDouble(sqrt(sum / self -> samples));
}

static PyObject *sound_get_playing(Sound *self, void *closure) {
    return PyBool_FromLong(MIX_TrackPlaying(self -> track));
}

static PyObject *sound_get_paused(Sound *self, void *closure) {
    return PyBool_FromLong(MIX_TrackPaused(self -> track));
}

static Sound *sound_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Sound *self = (Sound *) type -> tp_alloc(type, 0);
    return self ? (self -> track = MIX_CreateTrack(mixer)) ? self : (PyErr_SetString(error, SDL_GetError()), NULL) : NULL;
}

static int sound_init(Sound *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"name", NULL};
    const char *name = NULL;

    INIT(!PyArg_ParseTupleAndKeywords(args, kwds, "|s:Sound", kwlist, &name))

    if (!name) {
        sprintf(path.src + path.size, PICKUP);
        name = path.src;
    }

    return load(self, name);
}

static PyObject *sound_play(Sound *self, PyObject *args) {
    return MIX_PlayTrack(self -> track, 0) ? Py_None : (PyErr_SetString(error, SDL_GetError()), NULL);
}

static PyObject *sound_pause(Sound *self, PyObject *args) {
    return MIX_PauseTrack(self -> track) ? Py_None : (PyErr_SetString(error, SDL_GetError()), NULL);
}

static PyObject *sound_resume(Sound *self, PyObject *args) {
    return MIX_ResumeTrack(self -> track) ? Py_None : (PyErr_SetString(error, SDL_GetError()), NULL);
}

static PyGetSetDef sound_getset[] = {
    {"amp", (getter) sound_get_amp, NULL, "Get the current amplitude (loudness) of the audio playback", NULL},
    {"samples", (getter) sound_get_samples, NULL, "Extract the raw PCM data from the audio as a tuple of floats", NULL},
    {"playing", (getter) sound_get_playing, NULL, "Determine whether the audio is playing", NULL},
    {"paused", (getter) sound_get_paused, NULL, "Determine whether the audio has been paused", NULL},
    {NULL}
};

static PyMethodDef sound_methods[] = {
    {"play", (PyCFunction) sound_play, METH_NOARGS, "Play or restart the audio"},
    {"pause", (PyCFunction) sound_pause, METH_NOARGS, "Pause the audio"},
    {"resume", (PyCFunction) sound_resume, METH_NOARGS, "Resume the audio from paused"},
    {NULL}
};

static PyType_Slot sound_slots[] = {
    {Py_tp_doc, "Play audio and sound effects"},
    {Py_tp_new, sound_new},
    {Py_tp_init, sound_init},
    {Py_tp_getset, sound_getset},
    {Py_tp_methods, sound_methods},
    {0}
};

Spec sound_data = {{"Sound", sizeof(Sound), 0, Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, sound_slots}};