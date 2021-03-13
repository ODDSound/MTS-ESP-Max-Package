#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include "z_dsp.h"
#include "libMTSClient.h"

////////////////////////// object structs
typedef struct _MTS_ESP_mtsclient
{
    t_object ob;            // the object itself (must be first)
    MTSClient *mts_client;
    long ref_count;
} t_MTS_ESP_mtsclient;

typedef struct _MTS_ESP_mtof_dsp
{
    t_pxobject m_obj;       // the object itself (must be first)
    t_MTS_ESP_mtsclient *mts_client_obj;
    long midichannel;       // -1 if unknown, else 0-15
    long midinote;
    long count1;              // signal connection counts
    long count2;
} t_MTS_ESP_mtof_dsp;

///////////////////////// function prototypes
void *MTS_ESP_mtsclient_new(t_symbol *s);
void MTS_ESP_mtsclient_free(t_MTS_ESP_mtsclient *x);

void MTS_ESP_mtof_dsp_assist(t_MTS_ESP_mtof_dsp *x, void *b, long m, long a, char *s);

void *MTS_ESP_mtof_dsp_new(t_symbol *s);
void MTS_ESP_mtof_dsp_free(t_MTS_ESP_mtof_dsp *x);

void MTS_ESP_mtof_dsp_dsp64(t_MTS_ESP_mtof_dsp *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void MTS_ESP_mtof_dsp_perform64(t_MTS_ESP_mtof_dsp *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);

void MTS_ESP_mtof_dsp_int(t_MTS_ESP_mtof_dsp *x, long n);
void MTS_ESP_mtof_dsp_float(t_MTS_ESP_mtof_dsp *x, double n);

void MTS_ESP_mtof_dsp_set_midinote(t_MTS_ESP_mtof_dsp *x, long n);
void MTS_ESP_mtof_dsp_set_midichannel(t_MTS_ESP_mtof_dsp *x, long n);

//////////////////////// global class pointer variable
void *MTS_ESP_mtsclient_class;
void *MTS_ESP_mtof_dsp_class;

//////////////////////// helper functions to clamp MIDI note and channel values
static inline long clamp_note_value(long midinote)
{
    if (midinote < 0) {
        midinote = 0;
    }
    else if (midinote > 127) {
        midinote = 127;
    }
    return midinote;
}

static inline long clamp_channel_value(long midichannel)
{
    if (midichannel > 0 && midichannel <= 16) {
        return midichannel - 1;
    }
    return -1;
}

//////////////////////// tuning constants
static double et[128];
static double iet[128];
const static double ratioToSemitones=17.31234049066756088832;

void ext_main(void *r)
{
    for (int i = 0; i < 128; i++) {
        et[i] = 440. * pow(2., (i-69.) / 12.);
        iet[i] = 1. / et[i];
    }
    
    t_class *mtsclient_class = class_findbyname(CLASS_BOX, gensym("MTS-ESP.mtsclient"));
    if (!mtsclient_class) {
        mtsclient_class = class_new("MTS-ESP.mtsclient", (method)MTS_ESP_mtsclient_new, (method)MTS_ESP_mtsclient_free, (long)sizeof(t_MTS_ESP_mtsclient), 0L, A_GIMME, 0);
        class_register(CLASS_BOX, mtsclient_class);
    }
    MTS_ESP_mtsclient_class = mtsclient_class;
    
	t_class *c;
    
	c = class_new("MTS-ESP.mtof~", (method)MTS_ESP_mtof_dsp_new, (method)MTS_ESP_mtof_dsp_free, (long)sizeof(t_MTS_ESP_mtof_dsp), 0L, A_GIMME, 0);

    class_addmethod(c, (method)MTS_ESP_mtof_dsp_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)MTS_ESP_mtof_dsp_dsp64, "dsp64", A_CANT, 0);
    class_addmethod(c, (method)MTS_ESP_mtof_dsp_int, "int", A_LONG, 0);
    class_addmethod(c, (method)MTS_ESP_mtof_dsp_float, "float", A_FLOAT, 0);

    class_dspinit(c);
	class_register(CLASS_BOX, c);
	MTS_ESP_mtof_dsp_class = c;
}

void *MTS_ESP_mtsclient_new(t_symbol *s)
{
    t_MTS_ESP_mtsclient *x = NULL;
    if ((x = (t_MTS_ESP_mtsclient *)object_alloc((t_class *)MTS_ESP_mtsclient_class))) {
        x->mts_client = MTS_RegisterClient();
        x->ref_count = 0;
    }
    return (x);
}

void MTS_ESP_mtsclient_free(t_MTS_ESP_mtsclient *x)
{
    MTS_DeregisterClient(x->mts_client);
}

void MTS_ESP_mtof_dsp_assist(t_MTS_ESP_mtof_dsp *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0:
                strncpy_zero(s, "(signal/int) MIDI note number in", 512);
                break;
            case 1:
                strncpy_zero(s, "(signal/int) MIDI channel in, set to 0 if unknown", 512);
                break;
        }
    }
    else {
        switch (a) {
            case 0:
                strncpy_zero(s, "(signal) Frequency out", 512);
                break;
            case 1:
                strncpy_zero(s, "(signal) Ratio detune out", 512);
                break;
            case 2:
                strncpy_zero(s, "(signal) Semitone detune out", 512);
                break;
        }
    }
}

void *MTS_ESP_mtof_dsp_new(t_symbol *s)
{
    t_MTS_ESP_mtof_dsp *x = NULL;
    if ((x = (t_MTS_ESP_mtof_dsp *)object_alloc((t_class *)MTS_ESP_mtof_dsp_class))) {
        x->mts_client_obj = NULL;
        x->midichannel = -1;
        x->midinote = 69;
        x->count1 = 0;
        x->count2 = 0;
        dsp_setup((t_pxobject *)x, 2);
        outlet_new((t_object *)x, "signal");
        outlet_new((t_object *)x, "signal");
        outlet_new((t_object *)x, "signal");
    }
    return (x);
}

void MTS_ESP_mtof_dsp_free(t_MTS_ESP_mtof_dsp *x)
{
    if (x->mts_client_obj) {
        x->mts_client_obj->ref_count--;
        if (x->mts_client_obj->ref_count <= 0) {
            object_unregister((t_object *)x->mts_client_obj);
            object_free((void *)x->mts_client_obj);
        }
    }
     dsp_free((t_pxobject *)x);
}

void MTS_ESP_mtof_dsp_dsp64(t_MTS_ESP_mtof_dsp *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    if (!x->mts_client_obj) {
        t_object *obj = NULL;
        if (!(obj = (t_object *)object_findregistered(gensym("MTS-ESP"), gensym("mts_client_object")))) {
            obj = (t_object *)newinstance(gensym("MTS-ESP.mtsclient"), 0, NULL);
            if (obj) {
                object_register(gensym("MTS-ESP"), gensym("mts_client_object"), obj);
            }
        }
        if (obj) {
            x->mts_client_obj = (t_MTS_ESP_mtsclient *)obj;
            x->mts_client_obj->ref_count++;
        }
    }
    
    x->count1 = count[0];
    x->count2 = count[1];
    object_method(dsp64, gensym("dsp_add64"), x, MTS_ESP_mtof_dsp_perform64, 0, NULL);
}

void MTS_ESP_mtof_dsp_perform64(t_MTS_ESP_mtof_dsp *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    double *in1 = ins[0];
    double *in2 = ins[1];
    double *out1 = outs[0];
    double *out2 = outs[1];
    double *out3 = outs[2];
    
    double last_freq = 0.;
    long last_midinote = -1;
    long last_midichannel = -1;
    double ratio = 1.;
    double semitones = 0.;
    
    while (sampleframes--) {
        long midinote = x->midinote;
        if (x->count1 && in1) {
            midinote = clamp_note_value((long)*in1);
        }
        if (x->mts_client_obj) {
            long midichannel = x->midichannel;
            if (x->count2 && in2) {
                midichannel = clamp_channel_value((long)*in2);
            }
            double freq = MTS_NoteToFrequency(x->mts_client_obj->mts_client, midinote, midichannel);
            if (midinote != last_midinote || midichannel != last_midichannel || freq != last_freq) {
                if (MTS_HasMaster(x->mts_client_obj->mts_client)) {
                    ratio = freq * iet[midinote];
                    semitones = ratioToSemitones * log(ratio);
                }
                else {
                    ratio = 1.;
                    semitones = 0.;
                }
            }
            last_freq = freq;
            last_midinote = midinote;
            last_midichannel = midichannel;
            
            *out1++ = freq;
            *out2++ = ratio;
            *out3++ = semitones;
        }
        else {
            *out1++ = et[midinote];
            *out2++ = 1.;
            *out3++ = 0.;
        }
        in1++;
        in2++;
    }
}

void MTS_ESP_mtof_dsp_int(t_MTS_ESP_mtof_dsp *x, long n)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            MTS_ESP_mtof_dsp_set_midinote(x, n);
            break;
        case 1:
            MTS_ESP_mtof_dsp_set_midichannel(x, n);
            break;
    }
}

void MTS_ESP_mtof_dsp_float(t_MTS_ESP_mtof_dsp *x, double n)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            MTS_ESP_mtof_dsp_set_midinote(x, (long)n);
            break;
        case 1:
            MTS_ESP_mtof_dsp_set_midichannel(x, (long)n);
            break;
    }
}

void MTS_ESP_mtof_dsp_set_midinote(t_MTS_ESP_mtof_dsp *x, long n)
{
    x->midinote = clamp_note_value(n);
}

void MTS_ESP_mtof_dsp_set_midichannel(t_MTS_ESP_mtof_dsp *x, long n)
{
    x->midichannel = clamp_channel_value(n);
}

