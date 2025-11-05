class Frame
{	Meta *M;
	Frame *Next;
	public :
	Frame ();
	Frame (int w, int h);
	~Frame ();
	void next (Frame *next) { Next=next; }
	Frame *next () { return Next; }
	Meta *getMeta () { return M; }
	void copyfrom (Meta *meta)
	{	M=new WindowsMeta(meta);
	}
};

class Frames
{	Frame *First;
	public :
	Frames () : First(0) {}
	~Frames ();
	void append ();
	void append (int w, int h);
	void remove ();
	void show (int n);
	int nframes ();
	void copyto (int n);
	void copyfrom (int n);
	int makeag (BitmapPS *bitmap);
};

extern Frames frames;