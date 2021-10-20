#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include "libMTSClient.h"

////////////////////////// object structs
typedef struct _MTS_ESP_mtsclient
{
    t_object ob;            // the object itself (must be first)
    MTSClient *mts_client;
    long ref_count;
} t_MTS_ESP_mtsclient;

typedef struct _MTS_ESP_mtof
{
    t_object ob;            // the object itself (must be first)
    void *m_outlet1_play;
    void *m_outlet2_semitones;
    void *m_outlet3_ratio;
    void *m_outlet4_freq;
    long m_in;
    void *m_proxy;
    t_MTS_ESP_mtsclient *mts_client_obj;
    long midichannel;       // -1 if unknown, else 0-15
    long *midichannel_list;
    long midichannel_list_size;
    long midinote;
    long *midinote_list;
    long midinote_list_size;
} t_MTS_ESP_mtof;

///////////////////////// function prototypes
void *MTS_ESP_mtsclient_new(t_symbol *s);
void MTS_ESP_mtsclient_free(t_MTS_ESP_mtsclient *x);

void MTS_ESP_mtof_assist(t_MTS_ESP_mtof *x, void *b, long m, long a, char *s);

void *MTS_ESP_mtof_new(t_symbol *s);
void MTS_ESP_mtof_free(t_MTS_ESP_mtof *x);

void MTS_ESP_mtof_int(t_MTS_ESP_mtof *x, long n);
void MTS_ESP_mtof_float(t_MTS_ESP_mtof *x, double n);
void MTS_ESP_mtof_bang(t_MTS_ESP_mtof *x);
void MTS_ESP_mtof_list(t_MTS_ESP_mtof *x, t_symbol *s, long argc, t_atom *argv);

void MTS_ESP_mtof_set_midinote(t_MTS_ESP_mtof *x, long midinote);
void MTS_ESP_mtof_update_outlets(t_MTS_ESP_mtof *x);
void MTS_ESP_mtof_set_midinote_list(t_MTS_ESP_mtof *x, t_symbol *s, long argc, t_atom *argv);
void MTS_ESP_mtof_set_midichannel(t_MTS_ESP_mtof *x, long midichannel);
void MTS_ESP_mtof_set_midichannel_list(t_MTS_ESP_mtof *x, t_symbol *s, long argc, t_atom *argv);


//////////////////////// global class pointer variable
void *MTS_ESP_mtsclient_class;
void *MTS_ESP_mtof_class;

//////////////////////// helper function to check if MTS client created for top-level patcher
static void MTS_ESP_mtof_check_client(t_MTS_ESP_mtof *x)
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
}

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
const static double ratioToSemitones = 17.31234049066756088832;

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
    
	c = class_new("MTS-ESP.mtof", (method)MTS_ESP_mtof_new, (method)MTS_ESP_mtof_free, (long)sizeof(t_MTS_ESP_mtof), 0L, A_GIMME, 0);

    class_addmethod(c, (method)MTS_ESP_mtof_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)MTS_ESP_mtof_int, "int", A_LONG, 0);
    class_addmethod(c, (method)MTS_ESP_mtof_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)MTS_ESP_mtof_bang, "bang", 0);
    class_addmethod(c, (method)MTS_ESP_mtof_list, "list", A_GIMME, 0);
    class_addmethod(c, (method)stdinletinfo, "inletinfo", A_CANT, 0);
    
	class_register(CLASS_BOX, c);
	MTS_ESP_mtof_class = c;
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

void MTS_ESP_mtof_assist(t_MTS_ESP_mtof *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0:
                strncpy_zero(s, "MIDI note number in, bang updates outlets", 512);
                break;
            case 1:
                strncpy_zero(s, "MIDI channel in, set to 0 if unknown", 512);
                break;
        }
    }
    else {
        switch (a) {
            case 0:
                strncpy_zero(s, "Frequency out", 512);
                break;
            case 1:
                strncpy_zero(s, "Ratio detune out", 512);
                break;
            case 2:
                strncpy_zero(s, "Semitone detune out", 512);
                break;
            case 3:
                strncpy_zero(s, "Play note if 1, ignore if 0", 512);
                break;
        }
    }
}

void *MTS_ESP_mtof_new(t_symbol *s)
{
    t_MTS_ESP_mtof *x = NULL;
    if ((x = (t_MTS_ESP_mtof *)object_alloc((t_class *)MTS_ESP_mtof_class))) {
        x->mts_client_obj = NULL;
        x->midichannel = -1;
        x->midichannel_list = NULL;
        x->midichannel_list_size = 0;
        x->midinote = 69;
        x->midinote_list = NULL;
        x->midinote_list_size = 0;
        x->m_proxy = proxy_new((t_object *)x, 1, &x->m_in);
        x->m_outlet1_play = outlet_new((t_object *)x, NULL);
        x->m_outlet2_semitones = outlet_new((t_object *)x, NULL);
        x->m_outlet3_ratio = outlet_new((t_object *)x, NULL);
        x->m_outlet4_freq = outlet_new((t_object *)x, NULL);
    }
    return (x);
}

void MTS_ESP_mtof_free(t_MTS_ESP_mtof *x)
{
    if (x->mts_client_obj) {
        x->mts_client_obj->ref_count--;
        if (x->mts_client_obj->ref_count <= 0) {
            object_unregister((t_object *)x->mts_client_obj);
            object_free((void *)x->mts_client_obj);
        }
    }
    if (x->midichannel_list) {
        free(x->midichannel_list);
    }
    if (x->midinote_list) {
        free(x->midinote_list);
    }
    freeobject((t_object *)x->m_proxy);
}

void MTS_ESP_mtof_int(t_MTS_ESP_mtof *x, long n)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            MTS_ESP_mtof_set_midinote(x, n);
            break;
        case 1:
            MTS_ESP_mtof_set_midichannel(x, n);
            break;
    }
}

void MTS_ESP_mtof_float(t_MTS_ESP_mtof *x, double n)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            MTS_ESP_mtof_set_midinote(x, (long)n);
            break;
        case 1:
            MTS_ESP_mtof_set_midichannel(x, (long)n);
            break;
    }
}

void MTS_ESP_mtof_bang(t_MTS_ESP_mtof *x)
{
    if (proxy_getinlet((t_object *)x) == 0) {
        MTS_ESP_mtof_update_outlets(x);
    }
}

void MTS_ESP_mtof_list(t_MTS_ESP_mtof *x, t_symbol *s, long argc, t_atom *argv)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            MTS_ESP_mtof_set_midinote_list(x, s, argc, argv);
            break;
        case 1:
            MTS_ESP_mtof_set_midichannel_list(x, s, argc, argv);
            break;
    }
}

void MTS_ESP_mtof_set_midinote(t_MTS_ESP_mtof *x, long midinote)
{
    x->midinote = clamp_note_value(midinote);
    x->midinote_list_size = 0;
    if (x->midinote_list) {
        free(x->midinote_list);
        x->midinote_list = NULL;
    }
    MTS_ESP_mtof_update_outlets(x);
}

void MTS_ESP_mtof_update_outlets(t_MTS_ESP_mtof *x)
{
    MTS_ESP_mtof_check_client(x);
    
    if (x->midinote_list_size && x->midinote_list) {
        t_atom *out_list_play = (t_atom*)malloc(sizeof(t_atom) * x->midinote_list_size);
        t_atom *out_list_semitones = (t_atom*)malloc(sizeof(t_atom) * x->midinote_list_size);
        t_atom *out_list_ratio = (t_atom*)malloc(sizeof(t_atom) * x->midinote_list_size);
        t_atom *out_list_freq = (t_atom*)malloc(sizeof(t_atom) * x->midinote_list_size);
        if (!out_list_play || !out_list_semitones || !out_list_ratio || !out_list_freq) {
            return;
        }
        
        long i;
        for (i = 0; i < x->midinote_list_size; i++) {
            if (x->mts_client_obj) {
                long midichannel = x->midichannel;
                if (i < x->midichannel_list_size && x->midichannel_list) {
                    midichannel = *(x->midichannel_list + i);
                }
                double freq = MTS_NoteToFrequency(x->mts_client_obj->mts_client, *(x->midinote_list + i), midichannel);
				double ratio = 1.;
				double semitones = 0.;
				if (MTS_HasMaster(x->mts_client_obj->mts_client)) {
					ratio = freq * iet[*(x->midinote_list + i)];
					semitones = ratioToSemitones * log(ratio);
				}
                atom_setlong(out_list_play+i, !MTS_ShouldFilterNote(x->mts_client_obj->mts_client, *(x->midinote_list + i), midichannel));
                atom_setfloat(out_list_semitones + i, semitones);
                atom_setfloat(out_list_ratio + i, ratio);
                atom_setfloat(out_list_freq + i, freq);
            }
            else {
                atom_setlong(out_list_play + i, 1);
                atom_setfloat(out_list_semitones + i, 0.);
                atom_setfloat(out_list_ratio + i, 1.);
                atom_setfloat(out_list_freq + i, et[*(x->midinote_list + i)]);
            }
        }
        
        outlet_list(x->m_outlet1_play, 0L, x->midinote_list_size, out_list_play);
        outlet_list(x->m_outlet2_semitones, 0L, x->midinote_list_size, out_list_semitones);
        outlet_list(x->m_outlet3_ratio, 0L, x->midinote_list_size, out_list_ratio);
        outlet_list(x->m_outlet4_freq, 0L, x->midinote_list_size, out_list_freq);
        
        free(out_list_play);
        free(out_list_semitones);
        free(out_list_ratio);
        free(out_list_freq);
    }
    else {
        if (x->mts_client_obj) {
            double freq = MTS_NoteToFrequency(x->mts_client_obj->mts_client, x->midinote, x->midichannel);
			double ratio = 1.;
            double semitones = 0.;
            if (MTS_HasMaster(x->mts_client_obj->mts_client)) {
				ratio = freq * iet[x->midinote];
				semitones = ratioToSemitones * log(ratio);
            }
            outlet_int(x->m_outlet1_play, !MTS_ShouldFilterNote(x->mts_client_obj->mts_client, x->midinote, x->midichannel));
            outlet_float(x->m_outlet2_semitones, semitones);
            outlet_float(x->m_outlet3_ratio, ratio);
            outlet_float(x->m_outlet4_freq, freq);
        }
        else {
            outlet_int(x->m_outlet1_play, 1);
            outlet_float(x->m_outlet2_semitones, 0);
            outlet_float(x->m_outlet3_ratio, 1.);
            outlet_float(x->m_outlet4_freq, et[x->midinote]);
        }
    }
}

void MTS_ESP_mtof_set_midinote_list(t_MTS_ESP_mtof *x, t_symbol *s, long argc, t_atom *argv)
{
    MTS_ESP_mtof_check_client(x);
    
    if (!argc) return;
    
    t_atom *out_list_play = (t_atom*)malloc(sizeof(t_atom) * argc);
    t_atom *out_list_semitones = (t_atom*)malloc(sizeof(t_atom) * argc);
    t_atom *out_list_ratio = (t_atom*)malloc(sizeof(t_atom) * argc);
    t_atom *out_list_freq = (t_atom*)malloc(sizeof(t_atom) * argc);
    long *midinotes = (long*)malloc(sizeof(long) * argc);
    if (!out_list_play || !out_list_semitones || !out_list_ratio || !out_list_freq || !midinotes) {
        if (out_list_play) {
            free(out_list_play);
        }
        if (out_list_semitones) {
            free(out_list_semitones);
        }
        if (out_list_ratio) {
            free(out_list_ratio);
        }
        if (out_list_freq) {
            free(out_list_freq);
        }
        if (midinotes) {
            free(midinotes);
        }
        return;
    }
    
    long i, n;
    t_atom *ap;
    long *m;
    for (i = 0, n = 0, ap = argv, m = midinotes; i < argc; i++, ap++) {
        switch (atom_gettype(ap)) {
            case A_LONG:
                *m++ = clamp_note_value(atom_getlong(ap));
                n++;
                break;
            case A_FLOAT:
                *m++ = clamp_note_value((long)atom_getfloat(ap));
                n++;
                break;
            default:
                break;
        }
    }
    
    if (n) {
        for (i = 0; i < n; i++) {
            if (x->mts_client_obj) {
                long midichannel = x->midichannel;
                if (i < x->midichannel_list_size && x->midichannel_list) {
                    midichannel = *(x->midichannel_list + i);
                }
                double freq = MTS_NoteToFrequency(x->mts_client_obj->mts_client, *(midinotes + i), midichannel);
                double ratio = 1.;
                double semitones = 0.;
                if (MTS_HasMaster(x->mts_client_obj->mts_client)) {
                	ratio = freq * iet[*(midinotes + i)];
                	semitones = ratioToSemitones * log(ratio);
                }
                atom_setlong(out_list_play + i, !MTS_ShouldFilterNote(x->mts_client_obj->mts_client, *(midinotes + i), midichannel));
                atom_setfloat(out_list_semitones + i, semitones);
                atom_setfloat(out_list_ratio + i, ratio);
                atom_setfloat(out_list_freq + i, freq);
            }
            else {
                atom_setlong(out_list_play + i, 1);
                atom_setfloat(out_list_semitones + i, 0.);
                atom_setfloat(out_list_ratio + i, 1.);
                atom_setfloat(out_list_freq + i, et[*(midinotes+i)]);
            }
        }
        
        outlet_list(x->m_outlet1_play, 0L, n, out_list_play);
        outlet_list(x->m_outlet2_semitones, 0L, n, out_list_semitones);
        outlet_list(x->m_outlet3_ratio, 0L, n, out_list_ratio);
        outlet_list(x->m_outlet4_freq, 0L, n, out_list_freq);
        
        if (x->midinote_list) {
            free(x->midinote_list);
        }
        x->midinote_list = midinotes;
        x->midinote_list_size = n;
    }
    else {
        free(midinotes);
    }
    
    free(out_list_play);
    free(out_list_semitones);
    free(out_list_ratio);
    free(out_list_freq);
}

void MTS_ESP_mtof_set_midichannel(t_MTS_ESP_mtof *x, long midichannel)
{
    x->midichannel = clamp_channel_value(midichannel);
    x->midichannel_list_size = 0;
    if (x->midichannel_list) {
        free(x->midichannel_list);
        x->midichannel_list = NULL;
    }
}

void MTS_ESP_mtof_set_midichannel_list(t_MTS_ESP_mtof *x, t_symbol *s, long argc, t_atom *argv)
{
    if (!argc) return;
    
    long *midichannels = (long*)malloc(sizeof(long) * argc);
    if (!midichannels) {
        return;
    }
    
    long i, n;
    t_atom *ap;
    long *m;
    for (i = 0, n = 0, ap = argv, m = midichannels; i < argc; i++, ap++) {
        switch (atom_gettype(ap)) {
            case A_LONG:
                *m++ = clamp_channel_value(atom_getlong(ap));
                n++;
                break;
            case A_FLOAT:
                *m++ = clamp_channel_value((long)atom_getfloat(ap));
                n++;
                break;
            default:
                break;
        }
    }

    if (n) {
        if (x->midichannel_list) {
            free(x->midichannel_list);
        }
        x->midichannel_list = midichannels;
        x->midichannel_list_size = n;
        x->midichannel = x->midichannel_list[x->midichannel_list_size - 1];
    }
    else {
        free(midichannels);
    }
}

