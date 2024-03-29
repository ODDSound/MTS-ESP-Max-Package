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

typedef struct _MTS_ESP_ftom
{
    t_object ob;			// the object itself (must be first)
    void *m_outlet1_midichannel;
    void *m_outlet2_midinote;
    long m_in;
    void *m_proxy;
    t_MTS_ESP_mtsclient *mts_client_obj;
    char midichannel;       // -1 if unknown, else 0-15
    long *midichannel_list;
    long midichannel_list_size;
    double freq;
    double *freq_list;
    long freq_list_size;
} t_MTS_ESP_ftom;

///////////////////////// function prototypes
void *MTS_ESP_mtsclient_new(t_symbol *s);
void MTS_ESP_mtsclient_free(t_MTS_ESP_mtsclient *x);

void MTS_ESP_ftom_assist(t_MTS_ESP_ftom *x, void *b, long m, long a, char *s);

void *MTS_ESP_ftom_new(t_symbol *s);
void MTS_ESP_ftom_free(t_MTS_ESP_ftom *x);

void MTS_ESP_ftom_int(t_MTS_ESP_ftom *x, long n);
void MTS_ESP_ftom_float(t_MTS_ESP_ftom *x, double n);
void MTS_ESP_ftom_bang(t_MTS_ESP_ftom *x);
void MTS_ESP_ftom_list(t_MTS_ESP_ftom *x, t_symbol *s, long argc, t_atom *argv);

void MTS_ESP_ftom_set_freq(t_MTS_ESP_ftom *x, double freq);
void MTS_ESP_ftom_update_outlets(t_MTS_ESP_ftom *x);
void MTS_ESP_ftom_set_freq_list(t_MTS_ESP_ftom *x, t_symbol *s, long argc, t_atom *argv);
void MTS_ESP_ftom_set_midichannel(t_MTS_ESP_ftom *x, long midichannel);
void MTS_ESP_ftom_set_midichannel_list(t_MTS_ESP_ftom *x, t_symbol *s, long argc, t_atom *argv);

//////////////////////// global class pointer variables
void *MTS_ESP_mtsclient_class;
void *MTS_ESP_ftom_class;

//////////////////////// helper function to check if MTS client created for top-level patcher
static void MTS_ESP_ftom_check_client(t_MTS_ESP_ftom *x)
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

//////////////////////// helper functions to clamp freq and MIDI channel values
static inline double clamp_freq_value(double freq)
{
    if (freq > 0) {
        return freq;
    }
    return 0;
}

static inline long clamp_channel_value(long midichannel)
{
    if (midichannel > 0 && midichannel <= 16) {
        return midichannel - 1;
    }
    return -1;
}

//////////////////////// helper function to convert frequency to midi note
const static double ln2=0.693147180559945309417;
static char freqToNoteET(double freq)
{
    static double freqs[128];static bool init=false;
    if (!init) {for (int i=0;i<128;i++) freqs[i]=440.*pow(2.,(i-69.)/12.);init=true;}
    if (freq<=freqs[0]) return 0;
    if (freq>=freqs[127]) return 127;
    int mid=0;int n=-1;int n2=-1;
    for (int first=0,last=127;freq!=freqs[(mid=first+(last-first)/2)];(freq<freqs[mid])?(last=mid-1):(first=mid+1)) if (first>last)
    {
        if (!mid) {n=mid;break;}
        if (mid>127) mid=127;
        n=mid-((freq-freqs[mid-1])<(freqs[mid]-freq));
        break;
    }
    if (n==-1) {if (freq==freqs[mid]) n=mid;else return 60;}
    if (!n) n2=1;
    else if (n==127) n2=126;
    else n2=n+1*(fabs(freqs[n-1]-freq)<fabs(freqs[n+1]-freq)?-1:1);
    if (n2<n) {int t=n;n=n2;n2=t;}
    double fmid=freqs[n]*pow(2.,0.5*(log(freqs[n2]/freqs[n])/ln2));
    return (char)(freq<fmid?n:n2);
}

void ext_main(void *r)
{
    t_class *mtsclient_class = class_findbyname(CLASS_BOX, gensym("MTS-ESP.mtsclient"));
    if (!mtsclient_class) {
        mtsclient_class = class_new("MTS-ESP.mtsclient", (method)MTS_ESP_mtsclient_new, (method)MTS_ESP_mtsclient_free, (long)sizeof(t_MTS_ESP_mtsclient), 0L, A_GIMME, 0);
        class_register(CLASS_BOX, mtsclient_class);
    }
    MTS_ESP_mtsclient_class = mtsclient_class;
    
	t_class *c;
    
	c = class_new("MTS-ESP.ftom", (method)MTS_ESP_ftom_new, (method)MTS_ESP_ftom_free, (long)sizeof(t_MTS_ESP_ftom), 0L, A_GIMME, 0);

    class_addmethod(c, (method)MTS_ESP_ftom_assist, "assist", A_CANT, 0);
    class_addmethod(c, (method)MTS_ESP_ftom_int, "int", A_LONG, 0);
    class_addmethod(c, (method)MTS_ESP_ftom_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)MTS_ESP_ftom_bang, "bang", 0);
    class_addmethod(c, (method)MTS_ESP_ftom_list, "list", A_GIMME, 0);
    class_addmethod(c, (method)stdinletinfo, "inletinfo", A_CANT, 0);

	class_register(CLASS_BOX, c);
	MTS_ESP_ftom_class = c;
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

void MTS_ESP_ftom_assist(t_MTS_ESP_ftom *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {
        switch (a) {
            case 0:
                strncpy_zero(s, "Frequency in, bang updates outlet", 512);
                break;
            case 1:
                strncpy_zero(s, "Destination MIDI channel, set to 0 if unknown", 512);
                break;
        }
    }
    else {
        switch (a) {
            case 0:
                strncpy_zero(s, "MIDI note number out", 512);
                break;
            case 1:
                strncpy_zero(s, "MIDI channel out", 512);
                break;
        }
    }
}

void *MTS_ESP_ftom_new(t_symbol *s)
{
	t_MTS_ESP_ftom *x = NULL;
	if ((x = (t_MTS_ESP_ftom *)object_alloc((t_class *)MTS_ESP_ftom_class))) {
        x->mts_client_obj = NULL;
        x->midichannel = -1;
        x->midichannel_list = NULL;
        x->midichannel_list_size = 0;
        x->freq = 440.;
        x->freq_list = NULL;
        x->freq_list_size = 0;
        x->m_proxy = proxy_new((t_object *)x, 1, &x->m_in);
        x->m_outlet1_midichannel = outlet_new((t_object *)x, NULL);
        x->m_outlet2_midinote = outlet_new((t_object *)x, NULL);
	}
	return (x);
}

void MTS_ESP_ftom_free(t_MTS_ESP_ftom *x)
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
    if (x->freq_list) {
        free(x->freq_list);
    }
    freeobject((t_object *)x->m_proxy);
}

void MTS_ESP_ftom_int(t_MTS_ESP_ftom *x, long n)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            MTS_ESP_ftom_set_freq(x, (double)n);
            break;
        case 1:
            MTS_ESP_ftom_set_midichannel(x, n);
            break;
    }
}

void MTS_ESP_ftom_float(t_MTS_ESP_ftom *x, double n)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            MTS_ESP_ftom_set_freq(x, n);
            break;
        case 1:
            MTS_ESP_ftom_set_midichannel(x, (long)n);
            break;
    }
}

void MTS_ESP_ftom_bang(t_MTS_ESP_ftom *x)
{
    if (proxy_getinlet((t_object *)x) == 0) {
        MTS_ESP_ftom_update_outlets(x);
    }
}

void MTS_ESP_ftom_list(t_MTS_ESP_ftom *x, t_symbol *s, long argc, t_atom *argv)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            MTS_ESP_ftom_set_freq_list(x, s, argc, argv);
            break;
        case 1:
            MTS_ESP_ftom_set_midichannel_list(x, s, argc, argv);
            break;
    }
}

void MTS_ESP_ftom_set_freq(t_MTS_ESP_ftom *x, double freq)
{
    x->freq = clamp_freq_value(freq);
    x->freq_list_size = 0;
    if (x->freq_list) {
        free(x->freq_list);
        x->freq_list = NULL;
    }
    MTS_ESP_ftom_update_outlets(x);
}

void MTS_ESP_ftom_update_outlets(t_MTS_ESP_ftom *x)
{
    MTS_ESP_ftom_check_client(x);
    
    if (x->freq_list_size && x->freq_list) {
        t_atom *out_list_midichannel = (t_atom*)malloc(sizeof(t_atom) * x->freq_list_size);
        t_atom *out_list_midinote = (t_atom*)malloc(sizeof(t_atom) * x->freq_list_size);
        if (!out_list_midichannel || !out_list_midinote) {
            if (out_list_midichannel) {
                free(out_list_midichannel);
            }
            if (out_list_midinote) {
                free(out_list_midinote);
            }
            return;
        }
        
        long i;
        for (i = 0; i < x->freq_list_size; i++) {
            long midichannel = x->midichannel;
            if (i < x->midichannel_list_size && x->midichannel_list) {
                midichannel = *(x->midichannel_list + i);
            }
            if (x->mts_client_obj) {
                if (midichannel&~15) {
                    char chan = 0;
                    atom_setlong(out_list_midinote + i, MTS_FrequencyToNoteAndChannel(x->mts_client_obj->mts_client, *(x->freq_list + i), &chan));
                    midichannel = chan;
                }
                else {
                    atom_setlong(out_list_midinote + i, MTS_FrequencyToNote(x->mts_client_obj->mts_client, *(x->freq_list + i), midichannel));
                }
                atom_setlong(out_list_midichannel + i, midichannel + 1);
            }
            else {
                if (midichannel&~15) midichannel = 0;
                atom_setlong(out_list_midinote + i, freqToNoteET(*(x->freq_list + i)));
                atom_setlong(out_list_midichannel + i, midichannel + 1);
            }
        }
        outlet_list(x->m_outlet1_midichannel, 0L, x->freq_list_size, out_list_midichannel);
        outlet_list(x->m_outlet2_midinote, 0L, x->freq_list_size, out_list_midinote);
        free(out_list_midichannel);
        free(out_list_midinote);
    }
    else {
        if (x->mts_client_obj) {
            if (x->midichannel&~15) {
                char chan = 0;
                long note = MTS_FrequencyToNoteAndChannel(x->mts_client_obj->mts_client, x->freq, &chan);
                outlet_int(x->m_outlet1_midichannel, chan + 1);
                outlet_int(x->m_outlet2_midinote, note);
            }
            else {
                outlet_int(x->m_outlet1_midichannel, x->midichannel + 1);
                outlet_int(x->m_outlet2_midinote, MTS_FrequencyToNote(x->mts_client_obj->mts_client, x->freq, x->midichannel));
            }
        }
        else {
            if (x->midichannel&~15) {
                outlet_int(x->m_outlet1_midichannel, 1);
            }
            else {
                outlet_int(x->m_outlet1_midichannel, x->midichannel + 1);
            }
            outlet_int(x->m_outlet2_midinote, freqToNoteET(x->freq));
        }
    }
}

void MTS_ESP_ftom_set_freq_list(t_MTS_ESP_ftom *x, t_symbol *s, long argc, t_atom *argv)
{
    if (!argc) return;
    
    MTS_ESP_ftom_check_client(x);

    t_atom *out_list_midichannel = (t_atom*)malloc(sizeof(t_atom) * argc);
    t_atom *out_list_midinote = (t_atom*)malloc(sizeof(t_atom) * argc);
    double *freqs = (double*)malloc(sizeof(double) * argc);
    if (!out_list_midichannel || !out_list_midinote || !freqs) {
        if (out_list_midichannel) {
            free(out_list_midichannel);
        }
        if (out_list_midinote) {
            free(out_list_midinote);
        }
        if (freqs) {
            free(freqs);
        }
        return;
    }
    
    long i, n;
    t_atom *ap;
    double *f;
    for (i = 0, n = 0, ap = argv, f = freqs; i < argc; i++, ap++) {
        switch (atom_gettype(ap)) {
            case A_LONG:
                *f++ = clamp_freq_value((double)atom_getlong(ap));
                n++;
                break;
            case A_FLOAT:
                *f++ = clamp_freq_value(atom_getfloat(ap));
                n++;
                break;
            default:
                break;
        }
    }
    
    if (n) {
        for (i = 0; i < n; i++) {
            long midichannel = x->midichannel;
            if (i < x->midichannel_list_size && x->midichannel_list) {
                midichannel = *(x->midichannel_list + i);
            }
            if (x->mts_client_obj) {
                if (midichannel&~15) {
                    char chan = 0;
                    atom_setlong(out_list_midinote + i, MTS_FrequencyToNoteAndChannel(x->mts_client_obj->mts_client, *(freqs + i), &chan));
                    midichannel = chan;
                }
                else {
                    atom_setlong(out_list_midinote + i, MTS_FrequencyToNote(x->mts_client_obj->mts_client, *(freqs + i), midichannel));
                }
                atom_setlong(out_list_midichannel + i, midichannel + 1);
            }
            else {
                if (midichannel&~15) midichannel = 0;
                atom_setlong(out_list_midinote + i, freqToNoteET(*(freqs + i)));
                atom_setlong(out_list_midichannel + i, midichannel + 1);
            }
        }
        outlet_list(x->m_outlet1_midichannel, 0L, n, out_list_midichannel);
        outlet_list(x->m_outlet2_midinote, 0L, n, out_list_midinote);
        if (x->freq_list) {
            free(x->freq_list);
        }
        x->freq_list = freqs;
        x->freq_list_size = n;
    }
    else {
        free(freqs);
    }
    
    free(out_list_midichannel);
    free(out_list_midinote);
}

void MTS_ESP_ftom_set_midichannel(t_MTS_ESP_ftom *x, long midichannel)
{
    x->midichannel = clamp_channel_value(midichannel);
    x->midichannel_list_size = 0;
    if (x->midichannel_list) {
        free(x->midichannel_list);
        x->midichannel_list = NULL;
    }
}

void MTS_ESP_ftom_set_midichannel_list(t_MTS_ESP_ftom *x, t_symbol *s, long argc, t_atom *argv)
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

