import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Scanner;

public final class Main {
	
	final static class ModelMaterial {
		
		public String name;
		
		public float[] color;
		public float[] specularColor;
		
		public String texture;
		
		public ModelMaterial(String n) {
			this.name = n;
		}
		
	}
	
	final static class ModelComponent {
		
		public final ModelMaterial material;
		
		public final List<float[]> pointArray = new ArrayList<>();
		public final List<float[][]> lineArray = new ArrayList<>();
		public final List<float[][][]> triangleArray = new ArrayList<>();
		
		public ModelComponent(ModelMaterial mtl) {
			if (mtl == null)
				throw new NullPointerException();
			this.material = mtl;
		}
		
	}
	
	final static class VertexIndex {
		
		public final List<float[]> vertexArray = new ArrayList<>();
		
		public final List<float[]> normalArray = new ArrayList<>();
		
		public final List<float[]> textureBindArray = new ArrayList<>();
		
	}
	
	static Map<String, ModelMaterial> loadMaterialLibrary(File f) throws IOException {
		InputStream in = new FileInputStream(f);
		BufferedReader rdr = new BufferedReader(new InputStreamReader(in, "UTF-8"));
		Map<String, ModelMaterial> map = new HashMap<>();
		ModelMaterial mtl = null;
		String ln = null;
		while ((ln = rdr.readLine()) != null) {
			ln = ln.trim();
			if (ln.length() == 0 || ln.charAt(0) == '#')
				continue;
			Scanner sc = new Scanner(ln);
			try {
				switch (sc.next()) {
				case "newmtl": {
					String n = sc.nextLine().trim();
					System.out.println("Material: " + n);
					mtl = new ModelMaterial(n);
					map.put(mtl.name, mtl);
					break;
				}
				case "illum":
					System.out.println("Skipped illumination model.");
					break;
				case "Ka":
				case "Kd":
					mtl.color = new float[] {sc.nextFloat(), sc.nextFloat(), sc.nextFloat()};
					if (sc.hasNext()) {
						System.err.println("Bad color statement.");
						System.err.println(": " + ln);
						break;
					}
					break;
				case "Ks":
					mtl.specularColor = new float[] {sc.nextFloat(), sc.nextFloat(), sc.nextFloat()};
					if (sc.hasNext()) {
						System.err.println("Bad color statement.");
						System.err.println(": " + ln);
						break;
					}
					break;
				case "Ns":
					break;
				case "Ke":
					System.out.println("Skipped emissive color.");
					break;
				case "d":
				case "Tr":
					System.out.println("Skipped transparency.");
					break;
				case "map_Ka":
				case "map_Kd":
					mtl.texture = sc.nextLine().trim();
					break;
				case "map_Ks":
					System.out.println("Skipped specular texture map.");
					break;
				default:
					System.err.println("Warning: Unrecognizable statement!");
					System.err.println(": " + ln);
				}
			} catch (RuntimeException ex) {
				System.err.println("Uncaught exception!");
				ex.printStackTrace();
			}
			sc.close();
		}
		rdr.close();
		return map;
	}
	
	static Collection<ModelComponent> loadModel(File f) throws IOException {
		InputStream in = new FileInputStream(f);
		BufferedReader rdr = new BufferedReader(new InputStreamReader(in, "UTF-8"));
		Map<String, ModelMaterial> mtlmap = null;
		VertexIndex vi = new VertexIndex();
		List<ModelComponent> comparr = new ArrayList<>();
		ModelComponent comp = null;
		String ln = null;
		while ((ln = rdr.readLine()) != null) {
			ln = ln.trim();
			if (ln.length() == 0 || ln.charAt(0) == '#')
				continue;
			Scanner sc = new Scanner(ln);
			try {
				switch (sc.next()) {
				case "v": {
					float[] v = new float[4];
					v[3] = 1.0f;
					for (int i = 0; sc.hasNext() && i < v.length; i++) {
						v[i] = sc.nextFloat();
					}
					if (sc.hasNext()) {
						System.err.println("Bad vertex statement.");
						System.err.println(": " + ln);
						break;
					}
					vi.vertexArray.add(v);
					break;
				}
				case "vt": {
					float[] v = new float[3];
					for (int i = 0; sc.hasNext() && i < v.length; i++) {
						v[i] = sc.nextFloat();
					}
					if (sc.hasNext()) {
						System.err.println("Bad vertex texture bind.");
						System.err.println(": " + ln);
						break;
					}
					vi.textureBindArray.add(v);
					break;
				}
				case "vn": {
					float[] v = new float[3];
					for (int i = 0; i < v.length; i++) {
						v[i] = sc.nextFloat();
					}
					if (sc.hasNext()) {
						System.err.println("Bad vertex normal statement.");
						System.err.println(": " + ln);
						break;
					}
					vi.normalArray.add(v);
					break;
				}
				case "vp":
					System.out.println("Skipped parameter space vertex.");
					break;
				case "mtllib": {
					String n = sc.nextLine().trim();
					System.out.println("Material library: " + n);
					mtlmap = loadMaterialLibrary(new File(n));
					break;
				}
				case "usemtl":
					comp = new ModelComponent(mtlmap.get(sc.nextLine().trim()));
					comparr.add(comp);
					break;
				case "o":
				case "g":
					System.out.println("Object: " + sc.nextLine().trim());
					break;
				case "p": {
					float[] v = vi.vertexArray.get(sc.nextInt() - 1);
					if (sc.hasNext()) {
						System.err.println("Bad point statement.");
						System.err.println(": " + ln);
						break;
					}
					comp.pointArray.add(v);
					break;
				}
				case "l": {
					float[] v = vi.vertexArray.get(sc.nextInt() - 1);
					do {
						float[][] vs = new float[][] {v, vi.vertexArray.get(sc.nextInt() - 1)}; 
						comp.lineArray.add(vs);
						v = vs[1];
					} while (sc.hasNext());
					break;
				}
				case "f": {
					float[][][] tri = new float[3][3][];
					for (int i = 0; i < tri.length; i++) {
						String[] strs = sc.next().split("/", 3);
						tri[i][0] = vi.vertexArray.get(Integer.parseInt(strs[0]) - 1);
						if (strs.length >= 2)
							tri[i][2] = vi.textureBindArray.get(Integer.parseInt(strs[1]) - 1);
						if (strs.length == 3)
							tri[i][1] = vi.normalArray.get(Integer.parseInt(strs[2]) - 1);
					}
					if (sc.hasNext()) {
						System.err.println("Unsupported plane statement.");
						System.err.println(": " + ln);
						break;
					}
					comp.triangleArray.add(tri);
					break;
				}
				default:
					System.err.println("Warning: Unrecognizable statement!");
					System.err.println(": " + ln);
				}
			} catch (Exception ex) {
				System.err.println("Uncaught exception!");
				ex.printStackTrace();
			}
			sc.close();
		}
		rdr.close();
		return comparr;
	}
	
	static void writeVector(DataOutput dout, float[] v, int d) throws IOException {
		for (int i = 0; i < d; i++) {
			dout.writeInt(Integer.reverseBytes(Float.floatToIntBits(v[i])));
		}
	}
	static void writeVertex(DataOutput dout, float[] v) throws IOException {
		writeVector(dout, v, 3);
	}
	
	static void writeSimpleModel(File f, Collection<ModelComponent> compcol) throws IOException {
		//TODO
		OutputStream out = new FileOutputStream(f);
		DataOutputStream dout = new DataOutputStream(new BufferedOutputStream(out));
		dout.writeInt(Integer.reverseBytes(compcol.size()));
		for (ModelComponent comp : compcol) {
			ModelMaterial mtl = comp.material;
			writeVector(dout, mtl.color, 3);
			writeVector(dout, mtl.specularColor, 3);
			byte[] bs = mtl.texture.getBytes();
			dout.writeShort(Short.reverseBytes((short)bs.length));
			dout.write(bs);
			dout.writeInt(Integer.reverseBytes(comp.pointArray.size()));
			dout.writeInt(Integer.reverseBytes(comp.lineArray.size()));
			dout.writeInt(Integer.reverseBytes(comp.triangleArray.size()));
			for (float[] v : comp.pointArray) {
				writeVertex(dout, v);
			}
			for (float[][] ln : comp.lineArray) {
				for (float[] v : ln) {
					writeVertex(dout, v);
				}
			}
			for (float[][][] tri : comp.triangleArray) {
				for (float[][] v : tri) {
					writeVertex(dout, v[0]);
				}
				for (float[][] v : tri) {
					writeVector(dout, v[1], 3);
				}
				for (float[][] v : tri) {
					writeVector(dout, v[2], 2);
				}
			}
		}
		dout.close();
	}
	
	static void writeModel(DataOutput dout, Collection<ModelComponent> compcol) throws IOException {
		Map<String, Integer> texmap = new LinkedHashMap<>();
		for (ModelComponent comp : compcol) {
			String tex = comp.material.texture;
			if (!texmap.containsKey(tex))
				texmap.put(tex, texmap.size());
		}
		dout.writeInt(0xFF4F424A);
		dout.writeShort(Short.reverseBytes((short)texmap.size()));
		dout.writeShort(Short.reverseBytes((short)compcol.size()));
		for (String tex : texmap.keySet()) {
			byte[] bs = tex.getBytes();
			dout.writeShort(Short.reverseBytes((short)bs.length));
			dout.write(bs);
		}
		for (ModelComponent comp : compcol) {
			Map<float[], Integer> vmap = new LinkedHashMap<>();
			Map<float[], Integer> vnmap = new LinkedHashMap<>();
			Map<float[], Integer> vtmap = new LinkedHashMap<>();
			for (float[] v : comp.pointArray) {
				if (!vmap.containsKey(v))
					vmap.put(v, vmap.size());
			}
			for (float[][] ln : comp.lineArray) {
				for (float[] v : ln) {
					if (!vmap.containsKey(v))
						vmap.put(v, vmap.size());
				}
			}
			for (float[][][] tri : comp.triangleArray) {
				for (float[][] v : tri) {
					if (!vmap.containsKey(v[0]))
						vmap.put(v[0], vmap.size());
					if (!vnmap.containsKey(v[1]))
						vnmap.put(v[1], vnmap.size());
					if (!vtmap.containsKey(v[2]))
						vtmap.put(v[2], vtmap.size());
				}
			}
			ModelMaterial mtl = comp.material;
			writeVector(dout, mtl.color, 3);
			writeVector(dout, mtl.specularColor, 3);
			dout.writeShort(Short.reverseBytes(texmap.get(mtl.texture).shortValue()));
			dout.writeShort(Short.reverseBytes((short)vmap.size()));
			dout.writeShort(Short.reverseBytes((short)vnmap.size()));
			dout.writeShort(Short.reverseBytes((short)vtmap.size()));
			for (float[] v : vmap.keySet()) {
				writeVertex(dout, v);
			}
			for (float[] vn : vnmap.keySet()) {
				writeVector(dout, vn, 3);
			}
			for (float[] vt : vtmap.keySet()) {
				writeVector(dout, vt, 2);
			}
			dout.writeShort(Short.reverseBytes((short)comp.pointArray.size()));
			dout.writeShort(Short.reverseBytes((short)comp.lineArray.size()));
			dout.writeShort(Short.reverseBytes((short)comp.triangleArray.size()));
			for (float[] v : comp.pointArray) {
				dout.writeShort(Short.reverseBytes(vmap.get(v).shortValue()));
			}
			for (float[][] ln : comp.lineArray) {
				for (float[] v : ln) {
					dout.writeShort(Short.reverseBytes(vmap.get(v).shortValue()));
				}
			}
			for (float[][][] tri : comp.triangleArray) {
				for (float[][] v : tri) {
					dout.writeShort(Short.reverseBytes(vmap.get(v[0]).shortValue()));
				}
				for (float[][] v : tri) {
					dout.writeShort(Short.reverseBytes(vnmap.get(v[1]).shortValue()));
				}
				for (float[][] v : tri) {
					dout.writeShort(Short.reverseBytes(vtmap.get(v[2]).shortValue()));
				}
			}
		}
	}
	
	public static void main(String[] args) throws IOException {
		if (args.length > 1)
			throw new IllegalArgumentException("Must provide one file as argument.");
		File f = new File(args.length == 1 ? args[0] : "model.obj");
		Collection<ModelComponent> compcol = loadModel(f);
		OutputStream out = new FileOutputStream(new File("model.dat"));
		DataOutputStream dout = new DataOutputStream(new BufferedOutputStream(out));
		writeModel(dout, compcol);
		dout.close();
	}
	
}
