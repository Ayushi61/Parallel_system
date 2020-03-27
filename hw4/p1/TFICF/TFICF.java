/*
 Single Author info:
 arajend4 Ayushi Rajendra Kumar 
*/
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import java.io.IOException;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.lang.Math;
/*
 * Main class of the TFICF MapReduce implementation.
 * Author: Tyler Stocksdale
 * Date:   10/18/2017
 */
public class TFICF {

    public static void main(String[] args) throws Exception {
        // Check for correct usage
        if (args.length != 2) {
            System.err.println("Usage: TFICF <input corpus0 dir> <input corpus1 dir>");
            System.exit(1);
        }
		
		// return value of run func
		int ret = 0;
		
		// Create configuration
		Configuration conf0 = new Configuration();
		Configuration conf1 = new Configuration();
		
		// Input and output paths for each job
		Path inputPath0 = new Path(args[0]);
		Path inputPath1 = new Path(args[1]);
        try{
            ret = run(conf0, inputPath0, 0);
        }catch(Exception e){
            e.printStackTrace();
        }
        if(ret == 0){
        	try{
            	run(conf1, inputPath1, 1);
        	}catch(Exception e){
            	e.printStackTrace();
        	}        	
        }
     
     	System.exit(ret);
    }
		
	public static int run(Configuration conf, Path path, int index) throws Exception{
		// Input and output paths for each job

		Path wcInputPath = path;
		Path wcOutputPath = new Path("output" +index + "/WordCount");
		Path dsInputPath = wcOutputPath;
		Path dsOutputPath = new Path("output" + index + "/DocSize");
		Path tficfInputPath = dsOutputPath;
		Path tficfOutputPath = new Path("output" + index + "/TFICF");
		
		// Get/set the number of documents (to be used in the TFICF MapReduce job)
        	FileSystem fs = path.getFileSystem(conf);
        	FileStatus[] stat = fs.listStatus(path);
		String numDocs = String.valueOf(stat.length);
		conf.set("numDocs", numDocs);
		
		// Delete output paths if they exist
		FileSystem hdfs = FileSystem.get(conf);
		if (hdfs.exists(wcOutputPath))
			hdfs.delete(wcOutputPath, true);
		if (hdfs.exists(dsOutputPath))
			hdfs.delete(dsOutputPath, true);
		if (hdfs.exists(tficfOutputPath))
			hdfs.delete(tficfOutputPath, true);
		
		// Create and execute Word Count job
		
		/************ YOUR CODE HERE ************/
		Job wcJob = Job.getInstance(conf, "WordCount");

		// Set the TFICF class path
		wcJob.setJarByClass(TFICF.class);

		// Set mapper and reducer classes
		wcJob.setMapperClass(WCMapper.class);
		wcJob.setCombinerClass(WCReducer.class);
		wcJob.setReducerClass(WCReducer.class);
		//output key value data type
		wcJob.setOutputKeyClass(Text.class);
		wcJob.setOutputValueClass(IntWritable.class);
		// Set Input and output paths
		FileInputFormat.addInputPath(wcJob, wcInputPath);
		FileOutputFormat.setOutputPath(wcJob, wcOutputPath);
		wcJob.waitForCompletion(true);
			
		// Create and execute Document Size job
		
			/************ YOUR CODE HERE ************/
		Job dsJob = Job.getInstance(conf, "DocSize");

		// Set the TFICF class path
		dsJob.setJarByClass(TFICF.class);


		/*//set file input format
		dsJob.setInputFormatClass(TextInputFormat.class);
		dsJob.setOutputFormatClass(TextOutputFormat.class);
*/


		// Set mapper and reducer classes
		dsJob.setMapperClass(DSMapper.class);
               // dsJob.setCombinerClass(DSReducer.class);
		dsJob.setReducerClass(DSReducer.class);
		//output key value data type

		dsJob.setOutputKeyClass(Text.class);
		dsJob.setOutputValueClass(Text.class);
		// Set Input and output paths
		FileInputFormat.addInputPath(dsJob, dsInputPath);
		FileOutputFormat.setOutputPath(dsJob, dsOutputPath);
		dsJob.waitForCompletion(true);


		//Create and execute TFICF job
		
			/************ YOUR CODE HERE ************/
		Job TFICFJob = Job.getInstance(conf, "TFICF");

		// Set the TFICF class path
		TFICFJob.setJarByClass(TFICF.class);


		//set file input format
	/*	TFICFJob.setInputFormatClass(TextInputFormat.class);
		TFICFJob.setOutputFormatClass(TextOutputFormat.class);
*/
   		TFICFJob.setMapperClass(TFICFMapper.class);
//		TFICFJob.setCombinerClass(TFICFReducer.class);
                TFICFJob.setReducerClass(TFICFReducer.class);
		//output key value data type

		TFICFJob.setOutputKeyClass(Text.class);
		TFICFJob.setOutputValueClass(Text.class);


			
		// Set Input and output paths
		FileInputFormat.addInputPath(TFICFJob, tficfInputPath);
		FileOutputFormat.setOutputPath(TFICFJob, tficfOutputPath);
    		//TFICFJob.waitForCompletion(true);

			/************ YOUR CODE HERE ************/
		// Return status to main
	return TFICFJob.waitForCompletion(true) ? 0 : 1;

    }
	
	/*
	 * Creates a (key,value) pair for every word in the document 
	 *
	 * Input:  ( byte offset , contents of one line )
	 * Output: ( (word@document) , 1 )
	 *
	 * word = an individual word in the document
	 * document = the filename of the document
	 */
	public static class WCMapper extends Mapper<Object, Text, Text, IntWritable> {
		
		/************ YOUR CODE HERE ************/

		String str="";
		public void map(Object key, Text value, Context context) throws IOException, InterruptedException {

			//iterate over each word in file
			StringTokenizer line = new StringTokenizer(value.toString());
			while (line.hasMoreTokens()) {
				//getting input file name
				String fileName = ((FileSplit) context.getInputSplit()).getPath().getName();
				String parse_word=line.nextToken();
				if(!Pattern.matches("(^[a-zA-Z'*()\\]\\.\"?{}!].*)",parse_word))
					continue;
				//System.out.print("****************************" +parse_word);
				String word=parse_word.replaceAll(".,\"*'=(:;?{!})&[]","");
				if(word.isEmpty() || word.equals("-") || !Pattern.matches("(^[a-zA-Z'()\\]\"].*)",word)  || word.equals("]"))
					continue;
				//System.out.println("  " +word);
				str=word+"@"+fileName;
				//writing to the output file
				context.write(new Text(str.toLowerCase()), new IntWritable(1));
			}
		}
		
    }

    /*
	 * For each identical key (word@document), reduces the values (1) into a sum (wordCount)
	 *
	 * Input:  ( (word@document) , 1 )
	 * Output: ( (word@document) , wordCount )
	 *
	 * wordCount = number of times word appears in document
	 */
	public static class WCReducer extends Reducer<Text, IntWritable, Text, IntWritable> {
		
		/************ YOUR CODE HERE ************/
		public void reduce(Text key, Iterable<IntWritable> values, Context context) throws IOException, InterruptedException
		{
			int sum=0;
			for(IntWritable value:values)
			{
				sum+=value.get();
			}
			context.write(key,new IntWritable(sum));
		}
		
    }
    /*
     * Rearranges the (key,value) pairs to have only the document as the k* Input:  ( (word@document) , wordCount )
     * Input:  ( (word@document) , wordCount )
     * Output: ( document , (word=wordCount) )
     */	
    public static class DSMapper extends Mapper<Object, Text , Text, Text> {
		
		/************ YOUR CODE HERE ************/
	public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
		//iterate over each word in file
		StringTokenizer line = new StringTokenizer(value.toString());
			while (line.hasMoreTokens()) {
				String lineNext=line.nextToken();
				//System.out.println(lineNext);
				String doc=lineNext.split("@")[1];
				String word=lineNext.split("@")[0]+"="+line.nextToken();
				//System.out.println(doc+"doc:"+word+"word:");
				context.write(new Text(doc),new Text(word));
			}
		}

    }
    /*
     * For each identical key (document), reduces the values (word=wordCount) into a sum (docSize) 
     *
     * Input:  ( document , (word=wordCount) )
     * Output: ( (word@document) , (wordCount/docSize) )
     *
     * docSize = total number of words in the document
     */	
     public static class DSReducer extends Reducer<Text, Text, Text, Text> {
		
		/************ YOUR CODE HERE ************/
		public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
			int docSize = 0;
			Iterator values1=values.iterator();
			//ArrayList<String> doc=new ArrayList<>();
			//ArrayList<String> key1=new ArrayList<>();
			Map<String,String> doc_key1=new HashMap<String,String>();
			while ( values1.hasNext()) {
				String value_str=values1.next().toString();
				docSize += Integer.valueOf((value_str).split("=")[1]);
		//		System.out.println("+++++++++++++++++"+value_str);
				//doc.add(value_str.split("=")[1]+"/");
				doc_key1.put(value_str.split("=")[0],value_str.split("=")[1]+"/");
			}
			System.out.println("+++++++++++++++++"+docSize+"key = "+ key.toString());
			//values1=values.iterator();
			//System.out.println(values1.hasNext()+"++++++++++");
			for(String key2:doc_key1.keySet()){
				//System.out.println("---- "+values1.next().toString());
				//String key1 = (values1.next().toString()).split("=")[0]+"@"+key.toString();
				//System.out.println("key= "+key1);
				//String val= (values1.next().toString()).split("=")[1]+"/"+String.valueOf(docSize);
				//System.out.println("val= "+val);
				context.write(new Text(key2+"@"+key.toString()),new Text(doc_key1.get(key2)+String.valueOf(docSize)));


			}

		}
    }
    /*
	 * Rearranges the (key,value) pairs to have only the word as the key
	 * 
	 * Input:  ( (word@document) , (wordCount/docSize) )
	 * Output: ( word , (document=wordCount/docSize) )
	 */
	public static class TFICFMapper extends Mapper<Object, Text, Text, Text> {

		/************ YOUR CODE HERE ************/
		public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
			StringTokenizer line = new StringTokenizer(value.toString());
                        while (line.hasMoreTokens()) {
                                String lineNext=line.nextToken();
                                //System.out.println(lineNext);
                                String doc=lineNext.split("@")[0];
                                String word=lineNext.split("@")[1]+"="+line.nextToken();
                                //System.out.println(doc+"doc:"+word+"word:");
                                context.write(new Text(doc),new Text(word));
			}	

		}

	}

    /*
	 * For each identical key (word), reduces the values (document=wordCount/docSize) into a 
	 * the final TFICF value (TFICF). Along the way, calculates the total number of documents and 
	 * the number of documents that contain the word.
	 * 
	 * Input:  ( word , (document=wordCount/docSize) )
	 * Output: ( (document@word) , TFICF )
	 *
	 * numDocs = total number of documents
	 * numDocsWithWord = number of documents containing word
	 * TFICF = ln(wordCount/docSize + 1) * ln(numDocs/numDocsWithWord +1)
	 *
	 * Note: The output (key,value) pairs are sorted using TreeMap ONLY for grading purposes. For
	 *       extremely large datasets, having a for loop iterate through all the (key,value) pairs 
	 *       is highly inefficient!
	 */
	public static class TFICFReducer extends Reducer<Text, Text, Text, Text> {
		
		private static int numDocs;
		private Map<Text, Text> tficfMap = new HashMap<>();
		
		// gets the numDocs value and stores it
		protected void setup(Context context) throws IOException, InterruptedException {
			Configuration conf = context.getConfiguration();
			numDocs = Integer.parseInt(conf.get("numDocs"));
		}
				
		public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
			
			/************ YOUR CODE HERE ************/
			//int docSize = 0;
                        Iterator values1=values.iterator();
			int numDocsWithWord=0;
			ArrayList<Integer> wordCount=new ArrayList<>();
			ArrayList<Integer> docSize=new ArrayList<>(); 
			//ArrayList<Double> TFICF=new ArrayList();
                        ArrayList<String> doc=new ArrayList<>();
                        ArrayList<String> key1=new ArrayList<>();
                        while ( values1.hasNext()) {
                                String value_str=values1.next().toString();
//`				System.out.println("doc size is : -----"+key.toString()+"=word"+ Integer.valueOf((value_str).split("=")[1].split("/")[1]));
                                docSize.add(Integer.valueOf((value_str).split("=")[1].split("/")[1]));
				wordCount.add(Integer.valueOf((value_str).split("=")[1].split("/")[0]));
                //              System.out.println("+++++++++++++++++"+value_str);
                                doc.add(value_str.split("=")[0]+"@"+key.toString());
                                //key1.add(key.toString());
				numDocsWithWord++;
                        }
                      //  System.out.println("+++++++++++++++++"+docSize+"key = "+ key.toString());
                        //values1=values.iterator();
                        //System.out.println(values1.hasNext()+"++++++++++");
                        for(int i=0;i<numDocsWithWord;i++) {
				double TFICF=Math.log((double)wordCount.get(i)/(double)docSize.get(i)+1)*Math.log((double)(numDocs+1)/(double)(numDocsWithWord+1));
                                //System.out.println("---- "+values1.next().toString());
                                //String key1 = (values1.next().toString()).split("=")[0]+"@"+key.toString();
                                //System.out.println("key= "+key1);
                                //String val= (values1.next().toString()).split("=")[1]+"/"+String.valueOf(docSize);
                                //System.out.println("val= "+val);
                                //context.write(new Text(key1.get(i)),new Text(doc.get(i)+String.valueOf(docSize)));
				tficfMap.put(new Text(doc.get(i)), new Text(Double.toString(TFICF)));

                        }
		 
			//Put the output (key,value) pair into the tficfMap instead of doing a context.write
			//tficfMap.put(/*document@word*/, /*TFICF*/);
		}
		
		// sorts the output (key,value) pairs that are contained in the tficfMap
		protected void cleanup(Context context) throws IOException, InterruptedException {
            Map<Text, Text> sortedMap = new TreeMap<Text, Text>(tficfMap);
			for (Text key : sortedMap.keySet()) {
                context.write(key, sortedMap.get(key));
            }
        }
		
    } 
}
