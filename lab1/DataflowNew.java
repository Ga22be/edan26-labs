import java.util.ListIterator;
import java.util.LinkedList;
import java.util.BitSet;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.Executors;

class Random {
	int	w;
	int	z;
	
	public Random(int seed)
	{
		w = seed + 1;
		z = seed * seed + seed + 2;
	}

	int nextInt()
	{
		z = 36969 * (z & 65535) + (z >> 16);
		w = 18000 * (w & 65535) + (w >> 16);

		return (z << 16) + w;
	}
}

class Vertex {
	private int		index;
	private boolean		listed;
	List<Vertex>		pred;
	List<Vertex>		succ;
	BitSet			in;
	BitSet			out;
	BitSet			use;
	BitSet			def;

	Vertex(int i)
	{
		index	= i;
		pred	= new LinkedList<Vertex>();
		succ	= new LinkedList<Vertex>();
		in	= new BitSet();
		out	= new BitSet();
		use	= new BitSet();
		def	= new BitSet();
	}

	public synchronized void setListed(boolean listed) {
		this.listed = listed;
	}

	private synchronized boolean listIfNotListed(List<Vertex> worklist) {
		if (!listed) {
			listed = true;
			worklist.add(this);
			return true;
		}
		return false;
	}

	void computeIn(List<Vertex> worklist)
	{
		for (Vertex v : succ) {
			synchronized(v) {
				out.or(v.in);
			}
		}

		BitSet old = in;

		// in = use U (out - def)
		synchronized(this) {
			in = new BitSet();
			in.or(out);	
			in.andNot(def);	
			in.or(use);
		}


		if (!in.equals(old)) {

			for (Vertex v : pred) {
				v.listIfNotListed(worklist);
			}
		}
	}

	public void print()
	{
		int	i;

		System.out.print("use[" + index + "] = { ");
		for (i = 0; i < use.size(); ++i)
			if (use.get(i))
				System.out.print("" + i + " ");
		System.out.println("}");
		System.out.print("def[" + index + "] = { ");
		for (i = 0; i < def.size(); ++i)
			if (def.get(i))
				System.out.print("" + i + " ");
		System.out.println("}\n");

		System.out.print("in[" + index + "] = { ");
		for (i = 0; i < in.size(); ++i)
			if (in.get(i))
				System.out.print("" + i + " ");
		System.out.println("}");

		System.out.print("out[" + index + "] = { ");
		for (i = 0; i < out.size(); ++i)
			if (out.get(i))
				System.out.print("" + i + " ");
		System.out.println("}\n");
	}

}

class Task implements Runnable {
	private Vertex v;
	private List<Vertex> worklist;

	public Task(List<Vertex> worklist, Vertex v) {
		this.worklist = worklist;
		this.v = v;
	}
	
	@Override
	public void run() {
		v.computeIn(worklist);
	}

}

class DataflowNew {

	public static void connect(Vertex pred, Vertex succ)
	{
		pred.succ.add(succ);
		succ.pred.add(pred);
	}

	public static void generateCFG(Vertex vertex[], int maxsucc, Random r)
	{
		int	i;
		int	j;
		int	k;
		int	s;	// number of successors of a vertex.

		System.out.println("generating CFG...");

		connect(vertex[0], vertex[1]);
		connect(vertex[0], vertex[2]);
		
		for (i = 2; i < vertex.length; ++i) {
			s = (r.nextInt() % maxsucc) + 1;
			for (j = 0; j < s; ++j) {
				k = Math.abs(r.nextInt()) % vertex.length;
				connect(vertex[i], vertex[k]);
			}
		}
	}

	public static void generateUseDef(	
		Vertex	vertex[],
		int	nsym,
		int	nactive,
		Random	r)
	{
		int	i;
		int	j;
		int	sym;

		System.out.println("generating usedefs...");

		for (i = 0; i < vertex.length; ++i) {
			for (j = 0; j < nactive; ++j) {
				sym = Math.abs(r.nextInt()) % nsym;

				if (j % 4 != 0) {
					if (!vertex[i].def.get(sym))
						vertex[i].use.set(sym);
				} else {
					if (!vertex[i].use.get(sym))
						vertex[i].def.set(sym);
				}
			}
		}
	}

	public static void liveness(Vertex vertex[], int nthread)
	{
		Vertex			v;
		int			i;
		List<Vertex>	worklist;
		long			begin;
		long			end;

		System.out.println("computing liveness...");

		begin = System.nanoTime();
		// Makes a thread-safe list
		worklist = Collections.synchronizedList(new LinkedList<Vertex>());

		for (i = 0; i < vertex.length; ++i) {
			worklist.add(vertex[i]);
			vertex[i].setListed(true);
		}
		
		ThreadPoolExecutor executor = (ThreadPoolExecutor) Executors.newFixedThreadPool(nthread);
		while (executor.getActiveCount() > 0 || !worklist.isEmpty()) {
			try {
				v = worklist.remove(0);
				v.setListed(false);
				Runnable task = new Task(worklist, v);
				executor.submit(task);	
			} catch (IndexOutOfBoundsException e) {
				// somebody else got the last one	
			}
		}
		
		executor.shutdown();

//		while (!worklist.isEmpty()) {
//			v = worklist.remove(0);
//			v.setListed(false);
//			v.computeIn(worklist);
//		}
		end = System.nanoTime();

//		System.out.println("List size" + worklist.size());

		System.out.println("T = " + (end-begin)/1e9 + " s");
	}

	public static void main(String[] args)
	{
		int	i;
		int	nsym;
		int	nvertex;
		int	maxsucc;
		int	nactive;
		int	nthread;
		boolean	print;
		Vertex	vertex[];
		Random	r;

		r = new Random(1);

		nsym = Integer.parseInt(args[0]);
		nvertex = Integer.parseInt(args[1]);
		maxsucc = Integer.parseInt(args[2]);
		nactive = Integer.parseInt(args[3]);
		nthread = Integer.parseInt(args[4]);
		print = Integer.parseInt(args[5]) != 0;
	
		System.out.println("nsym = " + nsym);
		System.out.println("nvertex = " + nvertex);
		System.out.println("maxsucc = " + maxsucc);
		System.out.println("nactive = " + nactive);

		vertex = new Vertex[nvertex];

		for (i = 0; i < vertex.length; ++i)
			vertex[i] = new Vertex(i);

		generateCFG(vertex, maxsucc, r);
		generateUseDef(vertex, nsym, nactive, r);
		liveness(vertex, nthread);

		if (print)
			for (i = 0; i < vertex.length; ++i)
				vertex[i].print();
	}	
}
